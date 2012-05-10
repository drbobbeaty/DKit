/**
 * In order to test the SC queue, we need to have a "drain" - something
 * that will pop() and count everything off the queue, and then
 * we can look at the results and see if what we have removed is equal
 * to all the data that all the "hammers" have been able to place on the
 * queue.
 */
#ifndef __DKIT_TEST_DRAIN_H
#define __DKIT_TEST_DRAIN_H

//	System Headers
#include <iostream>
#include <sstream>
#include <string>

//	Third-Party Headers
#include <boost/thread.hpp>

//	Other Headers
#include "FIFO.h"


/**
 * Main class definition - and implementation...
 */
class Drain
{
	public:
		/**
		 * This is the standard constructor - it takes a pointer to a queue
		 * and a count of the number of things to push onto that queue at
		 * the proper time. This sets up this hammer to be ready to go when
		 * the time comes, but DOES NOT start the hammering at this time.
		 */
		Drain( uint32_t anID, dkit::FIFO<int32_t> *aQueue ) :
			mID(anID),
			mQueue(aQueue),
			mThread(),
			mStopOnEmpty(false),
			mCount(0),
			mDone(false)
		{
		}

		/**
		 * This is the standard destructor that needs to be on all classes.
		 */
		virtual ~Drain()
		{
		}

		/**
		 * This method actually STARTS the hammering this class is going
		 * to do. Clearly, if there's no queue to hammer, then there's not
		 * a lot this guys is GOING to do, but that's just a trivial case.
		 */
		void start()
		{
			mDone = false;
			mStopOnEmpty = false;
			std::ostringstream msg;
			msg << "[Drain::start(" << mID << ")] - starting the drain thread..." << std::endl;
			std::cout << msg.str();
			mThread = boost::thread(&Drain::doIt, this);
		}

		/**
		 * This method will be called to tell the Drain that the next time
		 * it tries to pop() something from the queue, and fails, to just
		 * stop as the queue has no more active producers, and it's just time
		 * to call it a day.
		 */
		void stopOnEmpty()
		{
			mStopOnEmpty = true;
		}

		/**
		 * This method can be used to see if the hammer is done with all
		 * the work it was assigned to do. Simple, but very effective.
		 */
		bool isDone()
		{
			return mDone;
		}

		/**
		 * This method will return the number of SUCCESSFUL pop()s we have
		 * done on the queue. It's a way to know if we have balanced the
		 * pushed data with the popped data.
		 */
		uint32_t getCount()
		{
			return mCount;
		}

	private:
		/**
		 * Because the default constructor doesn't allow for a queue, it's
		 * really the trivial case of nothing to do. So let's not let anyone
		 * get at it. This is just not something we need to support.
		 */
		Drain() :
			mID(0),
			mQueue(NULL),
			mThread(),
			mStopOnEmpty(false),
			mCount(0),
			mDone(false)
		{
		}

		/**
		 * This is the method that actually pushes the elements onto the
		 * provided queue. It's nothing much, but it makes sure that all
		 * the push() calls are successful - or logs what's up.
		 */
		void doIt()
		{
			// if we have a queue, push on what we've been asked to
			if (mQueue != NULL) {
				int32_t		val;
				while (true) {
					// pop() until we are empty...
					while (mQueue->pop(val)) {
						++mCount;
					};
					// ...and then wait a bit to try it all again
					if (!mStopOnEmpty) {
						usleep(100000);
					} else {
						break;
					}
				}
			}
			// now set the flag that we are done
			std::ostringstream msg;
			msg << "[Drain::doIt(" << mID << ")] - popped " << mCount << " items off the queue" << std::endl;
			std::cout << msg.str();
			mDone = true;
		}

		// these are the ivars I'm going to need for the hammer
		uint32_t			mID;
		dkit::FIFO<int32_t> *mQueue;
		// ...and this is the thread to do all the work
		boost::thread		mThread;
		// this indicates if we are to stop once we are empty
		bool				mStopOnEmpty;
		// this is the count of successful pop()s we have done
		uint32_t			mCount;
		// when we are done, this will be 'true', which can be monitored
		bool				mDone;
};

#endif		// __DKIT_TEST_DRAIN_H
