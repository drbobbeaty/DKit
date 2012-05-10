/**
 * In order to test the MP queue, we need to have a "hammer" - something
 * that will push() on a fixed number of "things" to the queue, and then
 * we can look at the results and see if all these "hammers" have been
 * able to place their data on the queue.
 */
#ifndef __DKIT_TEST_HAMMER_H
#define __DKIT_TEST_HAMMER_H

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
class Hammer
{
	public:
		/**
		 * This is the standard constructor - it takes a pointer to a queue
		 * and a count of the number of things to push onto that queue at
		 * the proper time. This sets up this hammer to be ready to go when
		 * the time comes, but DOES NOT start the hammering at this time.
		 */
		Hammer( uint32_t anID, dkit::FIFO<int32_t> *aQueue, uint32_t aCount ) :
			mID(anID),
			mQueue(aQueue),
			mCount(aCount),
			mThread(),
			mDone(false)
		{
		}

		/**
		 * This is the standard destructor that needs to be on all classes.
		 */
		virtual ~Hammer()
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
			std::ostringstream msg;
			msg << "[Hammer::start(" << mID << ")] - starting the hammer thread..." << std::endl;
			std::cout << msg.str();
			mThread = boost::thread(&Hammer::doIt, this);
		}

		/**
		 * This method can be used to see if the hammer is done with all
		 * the work it was assigned to do. Simple, but very effective.
		 */
		bool isDone()
		{
			return mDone;
		}

	private:
		/**
		 * Because the default constructor doesn't allow for a queue, it's
		 * really the trivial case of nothing to do. So let's not let anyone
		 * get at it. This is just not something we need to support.
		 */
		Hammer() :
			mID(0),
			mQueue(NULL),
			mCount(500),
			mThread(),
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
				for (uint32_t i = 0; i < mCount; ++i) {
					if (!mQueue->push(i)) {
						std::ostringstream msg;
						msg << "[Hammer::doIt(" << mID << ")] - unable to push " << i << " onto queue!" << std::endl;
						std::cerr << msg.str();
						break;
					}
				}
			}
			// now set the flag that we are done
			std::ostringstream msg;
			msg << "[Hammer::doIt(" << mID << ")] - done pushing " << mCount << " items onto queue" << std::endl;
			std::cout << msg.str();
			mDone = true;
		}

		// these are the ivars I'm going to need for the hammer
		uint32_t			mID;
		dkit::FIFO<int32_t>	*mQueue;
		uint32_t			mCount;
		// ...and this is the thread to do all the work
		boost::thread		mThread;
		// when we are done, this will be 'true', which can be monitored
		bool				mDone;
};

#endif		// __DKIT_TEST_HAMMER_H
