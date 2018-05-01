/**
 * LinkedFIFO.h - this file defines a single-producer, multi-consumer, linked
 *                FIFO queue and is using the compare-and-swap to achieve
 *                the lock-free goal. There can be any number of threads that
 *                call the pop() methods, but there can be ONE and ONLY ONE
 *                thread that calls the push() method.
 */
#ifndef __DKIT_SPMC_LINKEDFIFO_H
#define __DKIT_SPMC_LINKEDFIFO_H

// System Headers

// Third-Party Headers

// Other Headers
#include "FIFO.h"

// Forward Declarations

// Public Constants

// Public Datatypes

// Public Data Constants


namespace dkit {
namespace spmc {
/**
 * This is the main class definition
 */
template <class T> class LinkedFIFO :
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
		 * makes a simple queue ready to hold instances of T.
		 */
		LinkedFIFO() :
			FIFO<T>(),
			_head(NULL),
			_tail(NULL)
		{
			// start with NOTHING in the list - not one single thing
		}


		/**
		 * This is the standard copy constructor that needs to be in every
		 * class to make sure that we control how many copies we have
		 * floating around in the system.
		 */
		LinkedFIFO( const LinkedFIFO<T> & anOther ) :
			FIFO<T>(),
			_head(NULL),
			_tail(NULL)
		{
			// let the '=' operator do the heavy lifting...
			*this = anOther;
		}


		/**
		 * This is the standard destructor and needs to be virtual to make
		 * sure that if we subclass off this, the right destructor will be
		 * called.
		 */
		virtual ~LinkedFIFO()
		{
			// simply clear out the queue and we're all cleaned up
			clear();
		}


		/**
		 * When we process the result of an equality we need to make sure
		 * that we do this right by always having an equals operator on
		 * all classes.
		 *
		 * Because of the multiple-producer, single-consumer, nature of
		 * this class, it is IMPOSSIBLE to have the assignment operator
		 * be thread-safe without a mutex. This defeats the entire
		 * purpose, so what we have is a non-thread-safe assignment
		 * operator that is still useful if care is exercised to make
		 * sure that no use-case exists where there will be readers or
		 * writers to this queue while it is being copied in this assignment.
		 */
		LinkedFIFO & operator=( const LinkedFIFO<T> & anOther )
		{
			if (this != & anOther) {
				/**
				 * Assuming the argument is stable, then we're just going
				 * to walk it, pushing on the values from it in the right
				 * order so they are appended to us.
				 */
				for (Node *n = anOther._head; n != NULL; n = n->next) {
					if (!push(n->value)) {
						break;
					}
				}
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
			bool		error = false;

			// create a new Node with the provided value
			Node	*me = new Node(anElem);
			if (me == NULL) {
				error = true;
			} else {
				/**
				 * We need to add the new value to the tail and then link it
				 * back into the list. The logic for doing this is interesting
				 * in that we need to make sure that we handle the TWO cases
				 * of linking back into the list properly. The first is if
				 * there is NO LIST, and in that case, we need to make this
				 * the new head, and update the tail. If it's not empty, then
				 * we need to make sure that the mode we're about to link
				 * really exists. We do this by looking at the 'next' pointer.
				 * If it's NULL, then we know it's not about to be deleted,
				 * and we can set it. These atomic operations are really
				 * quite handy.
				 */
				if (!__sync_bool_compare_and_swap(&_head, NULL, me)) {
					// only set the link on the last node that exists
					__sync_bool_compare_and_swap(&(_tail->next), NULL, me);
				}
				Node	*oldTail = _tail;
				while (!__sync_bool_compare_and_swap(&_tail, oldTail, me)) {
					oldTail = _tail;
				}
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

			Node	*oldHead = NULL;
			// if there's no head, then the queue is empty - bad news
			while ((oldHead = _head) != NULL) {
				if (__sync_bool_compare_and_swap(&_head, oldHead, oldHead->next)) {
					break;
				}
			}
			// if we got something, then extract the value and drop the Node
			if (oldHead != NULL) {
				// extract the value from the node we've removed
				anElem = oldHead->value;
				/**
				 * If this node is the last in the list - characterized by
				 * the 'next' being NULL, then we need to update it's
				 * 'next' before we delete it. Why? So that the push()
				 * method understands that this node is about to go out of
				 * scope, and we don't want to update it.
				 */
				__sync_bool_compare_and_swap(&(oldHead->next), NULL, (Node *)0x01L);
				delete oldHead;
			} else {
				// nothing to get, so return an error and no value change
				error = true;
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
		 *
		 * In a multi-consumer queue, this method is VERY dangerous because
		 * there is no way to quarantee that another thread won't pop() off
		 * the value returned from peek(), while the peek() is trying to do
		 * something with it. In general, there's no way to know that the
		 * queue is STABLE from peek() to potential pop(), and so this
		 * method is only useful if the queue is STABLE.
		 */
		virtual bool peek( T & anElem )
		{
			bool		error = false;

			// if the next guy is NULL, we're empty
			if (__sync_bool_compare_and_swap(&_head, NULL, NULL)) {
				error = true;
			} else {
				// look at the next valid node to get the next value
				anElem = _head->value;
			}

			return !error;
		}


		/**
		 * This form of the peek() method is very much like the non-argument
		 * version of the pop() method. If there is something on the top of
		 * the queue, this method will return a COPY of it. If not, it will
		 * throw a std::exception, that needs to be caught.
		 *
		 * In a multi-consumer queue, this method is VERY dangerous because
		 * there is no way to quarantee that another thread won't pop() off
		 * the value returned from peek(), while the peek() is trying to do
		 * something with it. In general, there's no way to know that the
		 * queue is STABLE from peek() to potential pop(), and so this
		 * method is only useful if the queue is STABLE.
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
			// pretty simple - just pop everything off the queue
			T		v;
			while (pop(v));
		}


		/**
		 * This method will return 'true' if there are no items in the
		 * queue. Simple.
		 */
		virtual bool empty()
		{
			return (__sync_bool_compare_and_swap(&_head, NULL, NULL));
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
			size_t		retval = 0;
			for (Node *n = _head; n != NULL; n = n->next) {
				++retval;
			}
			return retval;
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
		 *
		 * Once again, since it's possible that there are multiple threads
		 * adding element to BOTH queues, this is only really going to be
		 * accutate when BOTH queues are stable and not undergoing change.
		 * Only then, can we be assured of the stability of the contents.
		 */
		bool operator==( const LinkedFIFO<T> & anOther ) const
		{
			bool	equals = true;

			// next, check the elements for equality
			if (equals) {
				Node	*me = _head;
				Node	*him = anOther._head;
				while (me != NULL) {
					if ((him == NULL) || (me->value != him->value)) {
						equals = false;
						break;
					}
					me = me->next;
					him = him->next;
				}
			}

			return equals;
		}


		/**
		 * This method checks to see if two queues are NOT equal in their
		 * contents and not their pointer values. This is how you'd likely
		 * expect equality to work.
		 */
		bool operator!=( const LinkedFIFO<T> & anOther ) const
		{
			return !operator=(anOther);
		}

	private:
		/**
		 * My linked list will be made of these nodes, and I'll make them
		 * with the one-argument constructor that just sets the value and
		 * nulls out the next pointer. Pretty simple.
		 */
		struct Node {
			T		value;
			Node	*next;

			Node() : value(), next(NULL) { }
			Node( const T & aValue ) : value(aValue), next(NULL) { }
			~Node() { }
		};

		/**
		 * We have a very simple structure - a singly-linked list of values
		 * that I'm just going to be very careful about modifying.
		 */
		Node	*volatile _head;
		Node	*volatile _tail;
};
}		// end of namespace spmc
}		// end of namespace dkit

#endif	// __DKIT_SPMC_LINKEDFIFO_H
