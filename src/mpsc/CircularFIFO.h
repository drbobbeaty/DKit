/**
 * CircularFIFO.h - this file defines the template class for a multi-producer,
 *                  single-consumer, circular FIFO queue with a size initially
 *                  specified in the definition of the instance. This queue
 *                  is completely thread-safe so long as there is ONE and ONLY
 *                  ONE thread removing elements from this container, as there
 *                  can be as many as necessary placing them into the queue.
 *                  The syntax is very simple - push() will push an element,
 *                  returning 'true' if there is room, and the value has been
 *                  placed in the queue.
 *
 *                  The peek() method allows the caller to see the element on
 *                  the top of the queue, and it HAS to be the same thread as
 *                  the SINGLE consumer, but the value stays on the queue.
 *                  The pop() will return true if there's something to pop
 *                  off the queue.
 */
#ifndef __DKIT_MPSC_CIRCULARFIFO_H
#define __DKIT_MPSC_CIRCULARFIFO_H

// System Headers
#include <stdint.h>

// Third-Party Headers

// Other Headers
#include "FIFO.h"

// Forward Declarations

// Public Constants

// Public Datatypes

// Public Data Constants



namespace dkit {
namespace mpsc {
/**
 * This is the main class definition
 */
template <class T, uint8_t N> class CircularFIFO :
	public FIFO<T>
{
	public:
		/*******************************************************************
		 *
		 *                     Constructors/Destructor
		 *
		 *******************************************************************/
		/**
		 * This is the default constructor that assumes NOTHING - it just
		 * makes a simple queue ready to hold things.
		 */
		CircularFIFO() :
			FIFO<T>(),
			_elements(),
			_head(0),
			_tail(0)
		{
		}


		/**
		 * This is the standard copy constructor that needs to be in every
		 * class to make sure that we control how many copies we have
		 * floating around in the system.
		 */
		CircularFIFO( const CircularFIFO<T, N> & anOther ) :
			FIFO<T>(),
			_elements(),
			_head(0),
			_tail(0)
		{
			// let the '=' operator do the heavy lifting...
			*this = anOther;
		}


		/**
		 * This is the standard destructor and needs to be virtual to make
		 * sure that if we subclass off this, the right destructor will be
		 * called.
		 */
		virtual ~CircularFIFO()
		{
			clear();
		}


		/**
		 * When we process the result of an equality we need to make sure
		 * that we do this right by always having an equals operator on
		 * all classes.
		 *
		 * Because of the multi-producer, single-consumer, nature of
		 * this class, it is IMPOSSIBLE to have the assignment operator
		 * be thread-safe without a mutex. This defeats the entire
		 * purpose, so what we have is a non-thread-safe assignment
		 * operator that is still useful if care is exercised to make
		 * sure that no use-case exists where there will be readers or
		 * writers to this queue while it is being copied in this assignment.
		 */
		CircularFIFO & operator=( const CircularFIFO<T, N> & anOther )
		{
			if (this != & anOther) {
				// now let's copy in the elements one by one
				for (size_t i = 0; i < eSize; i++) {
					_elements[i] = anOther._elements[i];
				}
				// now copy the pointers
				_head = anOther._head;
				_tail = anOther._tail;
			}
			return *this;
		}


		/*******************************************************************
		 *
		 *                        Accessor Methods
		 *
		 *******************************************************************/
		/**
		 * This pair of methods does what you'd expect - it returns the
		 * length of the queue as it exists at the present time. It's
		 * got two names because there are so many different kinds of
		 * implementations that it's often convenient to use one or the
		 * other to remain consistent.
		 */
		virtual size_t size() const
		{
			size_t		sz = _tail - _head;
			// see if we wrapped around - and correct the unsigned math
			if (sz > eSize) {
				sz = (sz + eSize) & eMask;
			}
			return sz;
		}


		virtual size_t length() const
		{
			return size();
		}


		/**
		 * This method returns the current capacity of the vector and
		 * is NOT the size per se. The capacity is what this queue
		 * will hold.
		 */
		virtual size_t capacity() const
		{
			return eSize;
		}


		/********************************************************
		 *
		 *                Element Accessing Methods
		 *
		 ********************************************************/
		/**
		 * This method takes an item and places it in the queue - if it can.
		 * If so, then it will return 'true', otherwise, it'll return 'false'.
		 */
		virtual bool push( const T & anElem )
		{
			bool		error = false;

			// move the tail (insertion point) and get the old value for me
			size_t	insPt = __sync_fetch_and_add(&_tail, 1) & eMask;
			// see if we've crashed the queue all the way around...
			if (_elements[insPt].valid) {
				// try to back out the damage we've done to the tail
				__sync_sub_and_fetch(&_tail, 1);
				error = true;
			} else {
				// save the data in the right spot and flag it as valid
				const_cast<T &>(_elements[insPt].value) = anElem;
				_elements[insPt].valid = true;
			}

			return !error;
		}


		/**
		 * This method updates the passed-in reference with the value on the
		 * top of the queue - if it can. If so, it'll return the value and
		 * 'true', but if it can't, as in the queue is empty, then the method
		 * will return 'false' and the value will be untouched.
		 */
		virtual bool pop( T & anElem )
		{
			bool		error = false;

			// see if we have an empty queue...
			if (!_elements[_head & eMask].valid) {
				error = true;
			} else {
				/**
				 * We need to get the current _head, and post-fetch increment
				 * it, and then pull out the value from the head and then
				 * invalidate the location in the queue. This looks a little
				 * off until you remember that the fetch is done BEFORE the
				 * adding of the 1 to _head.
				 */
				size_t	grab = __sync_fetch_and_add(&_head, 1) & eMask;
				anElem = const_cast<T &>(_elements[grab].value);
				_elements[grab].valid = false;
			}

			return !error;
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
			bool		error = false;

			// see if we have an empty queue...
			if (!_elements[_head & eMask].valid) {
				error = true;
			} else {
				anElem = const_cast<T &>(_elements[_head & eMask].value);
			}

			return !error;
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
			T		v;
			while (pop(v));
		}


		/**
		 * This method will return 'true' if there are no items in the
		 * queue. Simple.
		 */
		virtual bool empty()
		{
			return (_head == _tail);
		}


		/*******************************************************************
		 *
		 *                         Utility Methods
		 *
		 *******************************************************************/
		/**
		 * Traditionally, this operator would look at the elements in this
		 * queue, compare them to the elements in the argument queue, and
		 * see if they are the same. The problem with this is that in a
		 * single-producer, single-consumer system, there's no thread that
		 * can actually perform this operation in a thread-safe manner.
		 *
		 * Moreover, the complexity in doing this is far more than we want to
		 * take on in this class. It's lightweight, and while it's certainly
		 * possible to make a good equals operator, it's not worth the cost
		 * at this time. This method will always return 'false'.
		 */
		bool operator==( const CircularFIFO<T,N> & anOther ) const
		{
			return false;
		}


		/**
		 * Traditionally, this operator would look at the elements in this
		 * queue, compare them to the elements in the argument queue, and
		 * see if they are NOT the same. The problem with this is that in a
		 * single-producer, single-consumer system, there's no thread that
		 * can actually perform this operation in a thread-safe manner.
		 *
		 * Moreover, the complexity in doing this is far more than we want to
		 * take on in this class. It's lightweight, and while it's certainly
		 * possible to make a good not-equals operator, it's not worth the cost
		 * at this time. This method will always return 'true'.
		 */
		bool operator!=( const CircularFIFO<T,N> & anOther ) const
		{
			return !operator==(anOther);
		}


	private:
		/**
		 * Since the size of the queue is in the definition of the
		 * instance, it's possible to make some very simple enums that
		 * are the size and the masking bits for the index values, so that
		 * we know before anything starts up, how big to make things and
		 * how to "wrap around" when the time comes.
		 */
		enum {
			eMask = ((1 << N) - 1),
			eSize = (1 << N)
		};

		/**
		 * In order to simplify the ring buffer access, I'm going to actually
		 * have the ring a series of 'nodes', and for each, there will be a
		 * value and a valid 'flag'. If the flag isn't true, then the value
		 * in the node isn't valid. We'll use this to decouple the push()
		 * and pop() so that each only needs to know it's own location and
		 * then interrogate the node for state.
		 */
		struct Node {
			T		value;
			bool	valid;

			Node() : value(), valid(false) { }
			Node( const T & aValue ) : value(aValue), valid(true) { }
			~Node() { }
		};

		/**
		 * We have a very simple structure - an array of values of a fixed
		 * size and a simple head and tail.
		 */
		volatile Node		_elements[eSize];
		volatile size_t		_head;
		volatile size_t		_tail;
};
}		// end of namespace mpsc
}		// end of namespace dkit

#endif	// __DKIT_MPSC_CIRCULARFIFO_H
