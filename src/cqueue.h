/**
 * cqueue.h - this file defines a conflation queue that uses the key_value()
 *            of the elements placed into this queue as the index for
 *            conflation. Specifically, if two values with the same key_value()
 *            are placed in the cqueue, the second will override the first,
 *            and the location in the queue will be defined by the location
 *            of the first. This "conflating" is based on the key_value() so
 *            that updated values of the same "kind" of data can be placed
 *            into this queue, and only the most recent value will be popped
 *            off when the time comes. This is all done locklessly with the
 *            other components of DKit, and is a very useful tool to have.
 */
#ifndef __DKIT_CQUEUE_H
#define __DKIT_CQUEUE_H

// System Headers
#include <string.h>

// Third-Party Headers

// Other Headers
#include "FIFO.h"
#include "spsc/CircularFIFO.h"
#include "mpsc/CircularFIFO.h"
#include "spmc/CircularFIFO.h"
#include "trie.h"
#include "pool.h"

// Forward Declarations

// Public Constants
/**
 * We need to have a simple enum for the different "types" of queues that
 * we can use for the pool - all based on the complexity of the access. This
 * is meant to allow the user to have complete flexibility in how to ask for,
 * and recycle items from the pool.
 */
#ifndef __DKIT_QUEUE_TYPE
#define __DKIT_QUEUE_TYPE
namespace dkit {
enum queue_type {
	sp_sc = 0,
	mp_sc,
	sp_mc,
};
}		// end of namespace dkit
#endif	// __DKIT_QUEUE_TYPE

// Public Datatypes

// Public Data Constants


namespace dkit {
/**
 * This is the main class definition. The paramteres are as follows:
 *   T = the type of data to store in the conflation queue
 *   N = power of 2 for the size of the queue (2^N)
 *   Q = the type of access the queue has to have (SP/MP & SC/MC)
 *   KS = the size of the key for the value 'T'
 *   PN = the power of two of pooled keys to have in reserve (default: 2^17)
 */
template <class T, uint8_t N, queue_type Q, trie_key_size KS, uint8_t PN = 17> class cqueue :
	public FIFO<T>
{
	private:
		/**
		 * In order to put the keys into the queue, we need to be able to
		 * put them into a pool so taht we don't thrash memory too much.
		 * In order to do that, we need to new and delete the keys. This
		 * is most easily done if we throw it into a struct, and then just
		 * use it as a simple C-style struct of the right size. Simple.
		 */
		struct key_t {
			uint8_t		bytes[KS];
			/**
			 * These setters make it much easier to set the value of the
			 * key based on the different template values that we might
			 * be getting back from the key_value() function of the
			 * caller.
			 */
			void set( uint16_t aValue ) { memcpy(bytes, &aValue, 2); }
			void set( uint32_t aValue ) { memcpy(bytes, &aValue, 4); }
			void set( uint64_t aValue ) { memcpy(bytes, &aValue, 8); }
			void set( uint8_t aValue[] ) { memcpy(bytes, aValue, eKeyBytes); }
		};

	public:
		/*******************************************************************
		 *
		 *                     Constructors/Destructor
		 *
		 *******************************************************************/
		/**
		 * This is the default constructor that assumes NOTHING - it just
		 * makes an empty queue of the requested type, and then it's ready
		 * to be used by the caller.
		 */
		cqueue() :
			FIFO<T>(),
			_queue(NULL),
			_map()
		{
			/**
			 * We need to look at the 'type' and then create the FIFO
			 * that makes sense to what he's asking for. The size is the
			 * power of two (second arg), and will limit how many things
			 * can be in the pool at once.
			 */
			switch (Q) {
				case sp_sc:
					_queue = new spsc::CircularFIFO<key_t *, N>();
					break;
				case mp_sc:
					_queue = new mpsc::CircularFIFO<key_t *, N>();
					break;
				case sp_mc:
					_queue = new spmc::CircularFIFO<key_t *, N>();
					break;
			}
		}


		/**
		 * This is the standard copy constructor that needs to be in every
		 * class to make sure that we control how many copies we have
		 * floating around in the system.
		 */
		cqueue( const cqueue<T, N, Q, KS, PN> & anOther ) :
			FIFO<T>(),
			_queue(NULL),
			_map()
		{
			// let the '=' operator do the heavy lifting...
			*this = anOther;
		}


		/**
		 * This is the standard destructor and needs to be virtual to make
		 * sure that if we subclass off this, the right destructor will be
		 * called.
		 */
		virtual ~cqueue()
		{
			/**
			 * If we have a queue, then we need to know if it's holding
			 * pointers. If so, then we need to pop every one out and
			 * then delete each item. It's the only clean way to do it.
			 */
			if (_queue != NULL) {
				// we need to empty the queue of all keys - and delete each
				key_t	*key = NULL;
				while (_queue->pop(key)) {
					if (key != NULL) {
						delete key;
					}
				}
				// finally, drop the queue itself
				delete _queue;
			}
		}


		/**
		 * When we process the result of an equality we need to make sure
		 * that we do this right by always having an equals operator on
		 * all classes.
		 */
		cqueue & operator=( const cqueue<T, N, Q, KS, PN> & anOther )
		{
			if (this != & anOther) {
				if (_queue == NULL) {
					switch (Q) {
						case sp_sc:
							_queue = new spsc::CircularFIFO<key_t *, N>(*(anOther._queue));
							break;
						case mp_sc:
							_queue = new mpsc::CircularFIFO<key_t *, N>(*(anOther._queue));
							break;
						case sp_mc:
							_queue = new spmc::CircularFIFO<key_t *, N>(*(anOther._queue));
							break;
					}
				} else {
					*_queue = *(anOther._queue);
				}
				_map = anOther._map;
			}
			return *this;
		}


		/*******************************************************************
		 *
		 *                        Accessor Methods
		 *
		 *******************************************************************/
		/**
		 * This method takes an item and places it in the queue - if it can.
		 * If so, then it will return 'true', otherwise, it'll return 'false'.
		 */
		virtual bool push( const T & anElem )
		{
			// check on the existance of the queue
			if (_queue == NULL) {
				throw std::runtime_error("There is no defined queue for the keys - can't continue");
			}
			// see if we need to add the key to the queue
			if (!_map.upsert(anElem)) {
				// get an empty key from the pool
				key_t		*key = _pool.next();
				if (key == NULL) {
					throw std::runtime_error("Unable to pull a key from the pool - can't continue");
				}
				// copy in the value for this element
				key->set(key_value(anElem));
				// ...and then save it into the queue in the right place
				_queue->push(key);
			}
			return true;
		}


		/**
		 * This method updates the passed-in reference with the value on the
		 * top of the queue - if it can. If so, it'll return the value and
		 * 'true', but if it can't, as in the queue is empty, then the method
		 * will return 'false' and the value will be untouched.
		 */
		virtual bool pop( T & anElem )
		{
			bool		success = false;
			// check on the existance of the queue
			if (_queue == NULL) {
				throw std::runtime_error("There is no defined queue for the keys - can't continue");
			}
			// try to pop a key, and if we can, then extract the value
			key_t		*key = NULL;
			if (_queue->pop(key)) {
				success = _map.remove(key->bytes, anElem);
				_pool.recycle(key);
			}
			// return what we got from the trie
			return success;
		}


		/**
		 * This form of the pop() method will throw a std::exception
		 * if there is nothing to pop, but otherwise, will return the
		 * the first element on the queue. This is a slightly different
		 * form that fits a different use-case, and so it's a handy
		 * thing to have around at times.
		 */
		virtual T pop()
		{
			T		v;
			if (!pop(v)) {
				throw std::exception();
			}
			return v;
		}


		/**
		 * If there is an item on the queue, this method will return a look
		 * at that item without updating the queue. The return value will be
		 * 'true' if there is something, but 'false' if the queue is empty.
		 */
		virtual bool peek( T & anElem )
		{
			bool		success = false;
			// check on the existance of the queue
			if (_queue == NULL) {
				throw std::runtime_error("There is no defined queue for the keys - can't continue");
			}
			// try to pop a key, and if we can, then copy the value
			key_t		*key = NULL;
			if (_queue->peek(key)) {
				success = _map.get(key->bytes, anElem);
			}
			// return what we got from the trie
			return success;
		}


		/**
		 * This form of the peek() method is very much like the non-argument
		 * version of the pop() method. If there is something on the top of
		 * the queue, this method will return a COPY of it. If not, it will
		 * throw a std::exception, that needs to be caught.
		 */
		virtual T peek()
		{
			T		v;
			if (!peek(v)) {
				throw std::exception();
			}
			return v;
		}


		/**
		 * This method will clear out the contents of the queue so if
		 * you're storing pointers, then you need to be careful as this
		 * could leak.
		 */
		virtual void clear()
		{
			// we need to empty the queue of all keys - and delete each
			if (_queue != NULL) {
				key_t		*key = NULL;
				while (_queue->pop(key)) {
					_pool.recycle(key);
				}
			}
			// ...and also empty the trie
			_map.clear();
		}


		/**
		 * This method will return 'true' if there are no items in the
		 * queue. Simple.
		 */
		virtual bool empty()
		{
			if (_queue != NULL) {
				return _queue->empty();
			}
			return true;
		}


		/**
		 * This method will return the total number of items in the
		 * queue. Since it's possible that multiple threads are adding
		 * to this queue at any one point in time, it's really at BEST
		 * a snapshot of the size, and is only completely accurate
		 * when the queue is stable.
		 */
		virtual size_t size() const
		{
			if (_queue != NULL) {
				return _queue->size();
			}
			return 0;
		}


		/********************************************************
		 *
		 *                Functor Methods
		 *
		 ********************************************************/
		/**
		 * This method takes the functor subclass instance and applies
		 * it's process() method to all the valid entries in the cqueue.
		 * Since the processing will take place on the Node level, it's
		 * completely up to the implementor to decide what they want
		 * to do, and how they want to do it.
		 */
		bool apply( typename trie<T, KS>::functor & aFunctor )
		{
			return _map.apply(aFunctor);
		}


		/*******************************************************************
		 *
		 *                         Utility Methods
		 *
		 *******************************************************************/
		/**
		 * This method checks to see if two queues are equal in their
		 * contents and not their pointer values. This is how you'd likely
		 * expect equality to work.
		 */
		bool operator==( const cqueue<T, N, Q, KS, PN> & anOther ) const
		{
			// right now, identity is the only equality we know
			return (this == & anOther);
		}


		/**
		 * This method checks to see if two queues are NOT equal in their
		 * contents and not their pointer values. This is how you'd likely
		 * expect equality to work.
		 */
		bool operator!=( const cqueue<T, N, Q, KS, PN> & anOther ) const
		{
			return !operator=(anOther);
		}

	private:
		/**
		 * Just to make things clear, we're making an enum for the number
		 * of bytes in the key for this conflation queue. This will be
		 * used in the code to know what to make, and how to copy the
		 * values around.
		 */
		enum {
			eKeyBytes = KS
		};

		/**
		 * The queue of <uint8_t *> "keys" based on the style Q, is going
		 * to be a pointer we create in the constructor and use here. It's
		 * the same API no matter what Q is, it's just a cleaner way to
		 * implement the queue.
		 */
		FIFO<key_t *>			*_queue;
		/**
		 * This is the pool of keys that we are going to be using for the
		 * queue. The pool just makes it faster and easier to get them in
		 * and out, and we've got a simple structure defined to make the
		 * allocation and clean up clean and easy.
		 */
		pool<key_t *, PN, Q>	_pool;
		/**
		 * The trie/map to hold the values as they come in. Since this is
		 * a "map" of sorts, the same keyed value will be placed on top of
		 * itself as many times as necessary. The old values will be disposed
		 * of properly by the trie, so we don't need to worry about leaking.
		 */
		trie<T, KS>				_map;
};
}		// end of namespace dkit

#endif	// __DKIT_CQUEUE_H
