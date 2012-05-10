/**
 * CircularFIFO.h - this file defines the template class for a single-producer,
 *                  single-consumer, circular FIFO queue with a size initially
 *                  specified in the definition of the instance. This queue
 *                  is completely thread-safe so long as there is ONE and ONLY
 *                  ONE thread placing elements into this container, and ONE
 *                  and ONLY ONE thread removing them. The syntax is very
 *                  simple - push() will push an element, returning 'true' if
 *                  there is room, and the value has been placed in the queue.
 *                  The peek() method allows the caller to see the element on
 *                  the top of the queue, and it HAS to be the same thread as
 *                  the SINGLE consumer, but the value stays on the queue.
 *                  The pop() will return true if there's something to pop
 *                  off the queue.
 */
#ifndef __DKIT_SPSC_CIRCULARFIFO_H
#define __DKIT_SPSC_CIRCULARFIFO_H

//	System Headers
#include <stdint.h>

//	Third-Party Headers

//	Other Headers
#include "FIFO.h"

//	Forward Declarations

//	Public Constants

//	Public Datatypes

//	Public Data Constants


namespace dkit {
namespace spsc {
/**
 * This is the main template definition for the 2^N sized FIFO queue
 */
template <class T, uint8_t N> class CircularFIFO :
	public FIFO<T>
{
	public :
		/********************************************************
		 *
		 *                Constructors/Destructor
		 *
		 ********************************************************/
		/**
		 * This form of the constructor initializes the queue to a series
		 * of 2^N empty T instances. If there is no default constructor
		 * for T, this will not compile. Please make sure there's AT
		 * LEAST a default constructor and copy construstor as well as
		 * an assignment operator for T - it's all going to be needed.
		 */
		CircularFIFO() :
			FIFO<T>(),
			_elements(),
			_head(0),
			_tail(0)
		{
		}


		/**
		 * This is the standard copy constructor and needs to be in every
		 * class to make sure that we don't have too many things running
		 * around.
		 */
		CircularFIFO( const CircularFIFO<T,N> & anOther ) :
			FIFO<T>(),
			_elements(),
			_head(0),
			_tail(0)
		{
			// let the '=' operator do it
			*this = anOther;
		}


		/**
		 * This is the destructor for the queue and makes sure that
		 * everything is cleaned up before leaving.
		 */
		virtual ~CircularFIFO()
		{
			// there's not a lot to do at this point - yet
		}


		/**
		 * When we want to process the result of an equality we need to
		 * make sure that we do this right by always having an equals
		 * operator on all classes.
		 *
		 * Because of the single-producer, single-consumer, nature of
		 * this class, it is IMPOSSIBLE to have the assignment operator
		 * be thread-safe without a mutex. This defeats the entire
		 * purpose, so what we have is a non-thread-safe assignment
		 * operator that is still useful if care is exercised to make
		 * sure that no use-case exists where there will be readers or
		 * writers to this queue while it is being copied in this assignment.
		 */
		CircularFIFO<T,N> & operator=( const CircularFIFO<T,N> & anOther )
		{
			// make sure that we don't do this to ourselves
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


		/********************************************************
		 *
		 *                Accessor Methods
		 *
		 ********************************************************/
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
		 * This method pushes the provided element onto the queue, and
		 * because it's FIFO, it's going to be the placed at the 'tail'
		 * of the queue, and the pop() will take elements off the 'head'.
		 * This method will make sure that there is a place to put the
		 * element BEFORE placing it in the queue, and will return 'false'
		 * if there is no available space. Otherwise, it will place the
		 * element, and then return 'true'.
		 */
		virtual bool push( const T & anElem )
		{
			uint32_t	newTail = (_tail + 1);
			// if we have room, then let's save it in the spot
			if (newTail != _head) {
				_elements[_tail & eMask] = anElem;
				_tail = newTail;
				return true;
			}

			// the queue had no more space - push back!
			return false;
		}


		/**
		 * This form of the pop() method takes an element reference as
		 * an argument where the top of the queue will be placed - assuming
		 * there's something there to get. If so, then the value will be
		 * replaced, and a 'true' will be returned. Otherwise, a 'false'
		 * will be returned, as there's nothing to return.
		 */
		virtual bool pop( T & anElem )
		{
			// see if we have anything in the queue to pull out
			if (_head == _tail) {
				return false;
			}

			// OK, grab the head of the queue, and move up one
			anElem = _elements[_head & eMask];
			__sync_fetch_and_add(&_head, 1);
			return true;
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
		 * This method looks at the first element on the top of the queue
		 * and returns it in the provided reference argument. If there's
		 * nothing there, the return value will be 'false', and the reference
		 * will be untouched. If there's something there, the return value
		 * will be 'true'. This is a very similar usage pattern to the
		 * pop() method with the argument - it just doesn't advance the
		 * 'head' pointer.
		 */
		virtual bool peek( T & anElem )
		{
			// see if we have anything in the queue to pull out
			if (_head == _tail) {
				return false;
			}

			// OK, grab the head of the queue, but DO NOT move up one
			anElem = _elements[_head & eMask];
			return true;
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
		 * This method will remove all the elements from the queue by
		 * simply popping them off one by one until they are all removed.
		 * In order for this to be thread-safe, this action can only be
		 * called by the CONSUMER thread, as that's the same activity as
		 * is happening in this method.
		 */
		virtual void clear()
		{
			T		v;
			while (pop(v));
		}


		/**
		 * This method returns 'true' if there are no elements in the queue.
		 */
		virtual bool empty()
		{
			return (_head == _tail);
		}


		/********************************************************
		 *
		 *                Utility Methods
		 *
		 ********************************************************/
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
		 * We need to have a linear array of the elements as well as the
		 * head and tail of the queue. Since all these are going to be
		 * messed with by one producer and one consumer thread, making the
		 * ivars volatile is sufficient to ensure the thread-safety of
		 * the container.
		 */
		volatile T			_elements[eSize];
		volatile size_t		_head;
		volatile size_t		_tail;
};
}		// end of namespace spsc
}		// end of namespace dkit

#endif	// __DKIT_SPSC_CIRCULARFIFO_H
