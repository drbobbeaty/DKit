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
};
}		// end of namespace util
}		// end of namespace dkit

#endif		// __DKIT_UTIL_TIMER_H
