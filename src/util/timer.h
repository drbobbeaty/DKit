/**
 * timer.h - this file defines a simple timer class for DKit - something
 *           to use to time functions and operations as well as allowing
 *           the general user to take advantage of the same tools.
 */
#ifndef __DKIT_UTIL_TIMER_H
#define __DKIT_UTIL_TIMER_H

//	System Headers
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <CoreServices/CoreServices.h>
#endif

//	Third-Party Headers

//	Other Headers

//	Forward Declarations

//	Public Constants

//	Public Datatypes

//	Public Data Constants


namespace dkit {
namespace util {
/**
 * This is the main class definition.
 */
class timer
{
	public:
		/**
		 * This method can be used to simply get the number of microseconds
		 * since epoch at the time of the calling. It's a very handy way to
		 * do performance measurements as it's lightweight on the process and
		 * it's very precise.
		 */
		static inline uint64_t usecSinceEpoch() {
			timespec	ts;
			/**
			 * OS X does not have clock_gettime, use clock_get_time and then
			 * stuff the same values into the timespec structure for a
			 * compatible result.
			 */
			#ifdef __MACH__
				clock_serv_t	cclock;
				mach_timespec_t	mts;
				host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
				clock_get_time(cclock, &mts);
				mach_port_deallocate(mach_task_self(), cclock);
				// now we can save the values in the timespec
				ts.tv_sec = mts.tv_sec;
				ts.tv_nsec = mts.tv_nsec;
			#else
				clock_gettime(CLOCK_REALTIME, &ts);
			#endif
			return ((uint64_t)ts.tv_sec * 1000000LL +
					(uint64_t)ts.tv_nsec / 1000LL);
		}


		/**
		 * There will be times that we don't need to have the time since
		 * epoch, but just need a very fast RELATIVE timestamp for intervals
		 * and performance checks. In these cases, there are a few ways to
		 * get this value faster than that which has a known reference.
		 */
		static inline uint64_t usecStamp() {
			uint64_t	now = 0;
			#ifdef __MACH__
				/**
				 * For Mac OS X systems, we're going to use the MUCH
				 * FASTER mach_absolute_time(), and the biggest downside
				 * is that it's got no particular reference. That's why
				 * it works here for us nicely.
				 */
				static mach_timebase_info_data_t	__timebase;
				static uint64_t						__factor = 0;
				// get the current time - absolute to some reference
				now = mach_absolute_time();
				// convert to nanoseconds.
				if (__timebase.denom == 0) {
					(void) mach_timebase_info(&__timebase);
					__factor = __timebase.numer/(1000 * __timebase.denom);
				}
				// now convert the time to usec
				now *= __factor;
			#else
				/**
				 * For linux systems, we'll stick with the tried and true
				 * clock_gettime() and just leave it at that.
				 */
				clock_gettime(CLOCK_REALTIME, &ts);
				now = ((uint64_t)ts.tv_sec * 1000000LL +
					   (uint64_t)ts.tv_nsec / 1000LL);
			#endif
			return now;
		}
};
}		// end of namespace util
}		// end of namespace dkit

#endif		// __DKIT_UTIL_TIMER_H
