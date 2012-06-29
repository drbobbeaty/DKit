/**
 * FIFO.h - this file defines a general first-in, first-out queue that all
 *          the specific implementations will inherit from. This class is
 *          primarily abstract methods because we need them to be implemented
 *          by the subclasses, but it's important to have a super class that
 *          can stand in for any one of the FIFO queues in this library.
 */
#ifndef __DKIT_FIFO_H
#define __DKIT_FIFO_H

// System Headers

// Third-Party Headers

// Other Headers

// Forward Declarations

// Public Constants

// Public Datatypes

// Public Data Constants


namespace dkit {
/**
 * This is the main class definition
 */
template <class T> class FIFO
{
	public:
		/*******************************************************************
		 *
		 *                     Constructors/Destructor
		 *
		 *******************************************************************/
		/**
		 * This is the default constructor that assumes NOTHING - it just
		 * makes a simple queue ready to hold instances of T.
		 */
		FIFO()
		{
		}


		/**
		 * This is the standard copy constructor that needs to be in every
		 * class to make sure that we control how many copies we have
		 * floating around in the system.
		 */
		FIFO( const FIFO<T> & anOther )
		{
			// let the '=' operator do the heavy lifting...
			*this = anOther;
		}


		/**
		 * This is the standard destructor and needs to be virtual to make
		 * sure that if we subclass off this, the right destructor will be
		 * called.
		 */
		virtual ~FIFO()
		{
		}


		/**
		 * When we process the result of an equality we need to make sure
		 * that we do this right by always having an equals operator on
		 * all classes.
		 */
		FIFO & operator=( const FIFO<T> & anOther )
		{
			if (this != & anOther) {
				// this is where we might clean things up
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
		virtual bool push( const T & anElem ) = 0;
		/**
		 * This method updates the passed-in reference with the value on the
		 * top of the queue - if it can. If so, it'll return the value and
		 * 'true', but if it can't, as in the queue is empty, then the method
		 * will return 'false' and the value will be untouched.
		 */
		virtual bool pop( T & anElem ) = 0;
		/**
		 * This form of the pop() method will throw a std::exception
		 * if there is nothing to pop, but otherwise, will return the
		 * the first element on the queue. This is a slightly different
		 * form that fits a different use-case, and so it's a handy
		 * thing to have around at times.
		 */
		virtual T pop() = 0;
		/**
		 * If there is an item on the queue, this method will return a look
		 * at that item without updating the queue. The return value will be
		 * 'true' if there is something, but 'false' if the queue is empty.
		 */
		virtual bool peek( T & anElem ) = 0;
		/**
		 * This form of the peek() method is very much like the non-argument
		 * version of the pop() method. If there is something on the top of
		 * the queue, this method will return a COPY of it. If not, it will
		 * throw a std::exception, that needs to be caught.
		 */
		virtual T peek() = 0;
		/**
		 * This method will clear out the contents of the queue so if
		 * you're storing pointers, then you need to be careful as this
		 * could leak.
		 */
		virtual void clear() = 0;
		/**
		 * This method will return 'true' if there are no items in the
		 * queue. Simple.
		 */
		virtual bool empty() = 0;
		/**
		 * This method will return the total number of items in the
		 * queue. Since it's possible that multiple threads are adding
		 * to this queue at any one point in time, it's really at BEST
		 * a snapshot of the size, and is only completely accurate
		 * when the queue is stable.
		 */
		virtual size_t size() const = 0;

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
		bool operator==( const FIFO<T> & anOther ) const
		{
			// right now, identity is the only equality we know
			return (this == & anOther);
		}


		/**
		 * This method checks to see if two queues are NOT equal in their
		 * contents and not their pointer values. This is how you'd likely
		 * expect equality to work.
		 */
		bool operator!=( const FIFO<T> & anOther ) const
		{
			return !operator==(anOther);
		}
};
}		// end of namespace dkit

#endif	// __DKIT_FIFO_H
