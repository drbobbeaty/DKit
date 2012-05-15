/**
 * pool.h - this file defines a general pool of items that can be used as a
 *          more efficient way to deal with reuse than the traditional
 *          create/use/destroy scheme that's so common. The storage for the
 *          pool will be somewhat dictated by the usage - in that the client
 *          must give the pool a TYPE of queue to use: SPSC, MPSC, or SPMC.
 *          The pool will use this to create the storage it will use and then
 *          it's a simple matter of calling next() to get the next available
 *          item, and then recycle() to recycle it.
 */
#ifndef __DKIT_POOL_H
#define __DKIT_POOL_H

// System Headers

// Third-Party Headers
#include <boost/type_traits.hpp>

// Other Headers
#include "FIFO.h"
#include "spsc/CircularFIFO.h"
#include "mpsc/CircularFIFO.h"
#include "spmc/CircularFIFO.h"

// Forward Declarations
/**
 * In order to allow for pointers AND plain old datatypes to be the contents
 * of the pool, we need to have a few template functions that will take care
 * of creation and destruction on the items. It's really pretty sweet. These
 * will be defined after the class, and are used in only one place in the
 * class, but it's an important place.
 */
namespace dkit {
namespace pool_util {
template<typename T> void create( T t );
template<typename T> void create( T * & t );
template<typename T> void destroy( T t );
template<typename T> void destroy( T * & t );
}		// end of namespace pool_util
}		// end of namespace dkit

// Public Constants
/**
 * We need to have a simple enum for the different "types" of queues that
 * we can use for the pool - all based on the complexity of the access. This
 * is meant to allow the user to have complete flexibility in how to ask for,
 * and recycle items from the pool.
 */
namespace dkit {
enum queue_type {
	sp_sc = 0,
	mp_sc,
	sp_mc,
};
}		// end of namespace dkit

// Public Datatypes

// Public Data Constants


namespace dkit {
/**
 * This is the main class definition
 */
template <class T, uint8_t N, queue_type Q> class pool
{
	public:
		/*******************************************************************
		 *
		 *                     Constructors/Destructor
		 *
		 *******************************************************************/
		/**
		 * This is the default constructor that assumes NOTHING - it just
		 * makes a simple pool of no elements, but ready to generate what's
		 * needed, and store recycled values to a given limit.
		 */
		pool() :
			mQueue(NULL)
		{
			/**
			 * We need to look at the 'type' and then create the FIFO
			 * that makes sense to what he's asking for. The size is the
			 * power of two (second arg), and will limit how many things
			 * can be in the pool at once.
			 */
			switch (Q) {
				case sp_sc:
					mQueue = new spsc::CircularFIFO<T, N>();
					break;
				case mp_sc:
					mQueue = new mpsc::CircularFIFO<T, N>();
					break;
				case sp_mc:
					mQueue = new spmc::CircularFIFO<T, N>();
					break;
			}
		}


		/**
		 * This is the standard copy constructor that needs to be in every
		 * class to make sure that we control how many copies we have
		 * floating around in the system.
		 */
		pool( const pool<T, N, Q> & anOther ) :
			mQueue(NULL)
		{
			// let the '=' operator do the heavy lifting...
			*this = anOther;
		}


		/**
		 * This is the standard destructor and needs to be virtual to make
		 * sure that if we subclass off this, the right destructor will be
		 * called.
		 */
		virtual ~pool()
		{
			/**
			 * If we have a queue, then we need to know if it's holding
			 * pointers. If so, then we need to pop every one out and
			 * then delete each item. It's the only clean way to do it.
			 */
			if (mQueue != NULL) {
				// if it's pointers, then delete each one
				if (boost::is_pointer<T>::value) {
					T		val;
					while (mQueue->pop(val)) {
						pool_util::destroy(val);
					}
				}
				// finally, drop the queue itself
				delete mQueue;
			}
		}


		/**
		 * When we process the result of an equality we need to make sure
		 * that we do this right by always having an equals operator on
		 * all classes.
		 */
		pool & operator=( const pool<T, N, Q> & anOther )
		{
			if (this != & anOther) {
				/**
				 * At this point in time, we're not going to assign
				 * one pool to another. If they are pointers, we need
				 * to handle things very differently, and there's no
				 * quarantee that we'll be able to clone these effectively.
				 */
			}
			return *this;
		}


		/*******************************************************************
		 *
		 *                        Accessor Methods
		 *
		 *******************************************************************/
		/**
		 * This method is called to pull another item from the pool, or
		 * create a new one if nothing is in the pool. This is the classic
		 * way of getting the "next" item to work with.
		 */
		T next()
		{
			T		n;
			// see if we can pop one off the queue. If not, make one
			if ((mQueue == NULL) || !mQueue->pop(n)) {
				pool_util::create(n);
			}
			// return what we have - new or used
			return n;
		}


		/**
		 * This method is called when the user wants to recycle one of
		 * the items to the pool. If the pool is full, then we'll simply
		 * delete it. Otherwise, we'll put it back in the pool for use
		 * the next time.
		 */
		void recycle( T anItem )
		{
			if ((mQueue == NULL) || !mQueue->push(anItem)) {
				pool_util::destroy(anItem);
			}
		}


		/**
		 * This method returns the number of items in the pool at this time.
		 * When starting out, this will initially be zero, but as we put
		 * things into the pool via recycle(), this will build up to the
		 * maximum size allowed.
		 */
		size_t size() const
		{
			size_t		sz = 0;
			if (mQueue != NULL) {
				sz = mQueue->size();
			}
			return sz;
		}


		/**
		 * This method will return 'true' if there are no items in the pool
		 * to supply. Not to worry, any requests from the pool will first
		 * try the pool for existing items, and if none exists, one will be
		 * created for you.
		 */
		bool empty()
		{
			bool	ans = true;
			if ((mQueue != NULL) && !mQueue->empty()) {
				ans = false;
			}
			return ans;
		}


		/*******************************************************************
		 *
		 *                         Utility Methods
		 *
		 *******************************************************************/
		/**
		 * This method checks to see if two pools are equal in their
		 * contents and not their pointer values. This is how you'd likely
		 * expect equality to work.
		 */
		bool operator==( const FIFO<T> & anOther ) const
		{
			// right now, identity is the only equality we know
			return (this == & anOther);
		}


		/**
		 * This method checks to see if two pools are NOT equal in their
		 * contents and not their pointer values. This is how you'd likely
		 * expect equality to work.
		 */
		bool operator!=( const FIFO<T> & anOther ) const
		{
			return !operator=(anOther);
		}

	private:
		/**
		 * The queue of T based on the style Q, is going to be a pointer
		 * we create in the constructor and use here. It's the same API
		 * no matter what Q is, it's just a cleaner way to implement the
		 * queue.
		 */
		FIFO<T>		*mQueue;
};


namespace pool_util {
/**
 * In order to handle both pointers and non-pointers as data
 * types 'T' in the pool, we need to take advantage of the
 * template methods and make create() and delete() methods
 * for pointers and non-pointers.
 *
 * For create(), it's pretty easy - we allow for nothing to be
 * done for the non-pointer, and a standard 'new' for the pointer.
 * For delete(), it's the same - we delete it and then NULL it
 * out if it's a pointer, if it's not, we do nothing.
 */
template<typename T> void create( T t ) { }
template<typename T> void create( T * & t )
{
	t = new T();
}

template<typename T> void destroy( T t ) { }
template<typename T> void destroy( T * & t )
{
	if (t != NULL) {
		delete t;
		t = NULL;
	}
}
}		// end of namespace pool_util
}		// end of namespace dkit

#endif	// __DKIT_FIFO_H
