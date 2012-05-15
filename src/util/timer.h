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
#include <string>

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
		static inline uint64_t usecSinceEpoch()
		{
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
		static inline uint64_t usecStamp()
		{
			uint64_t	now = 0;
			#ifdef __MACH__
				/**
				 * For Mac OS X systems, we're going to use the MUCH
				 * FASTER mach_absolute_time(), and the biggest downside
				 * is that it's got no particular reference. That's why
				 * it works here for us nicely.
				 */
				static mach_timebase_info_data_t	__timebase;
				// get the current usec time - absolute to some reference
				now = mach_absolute_time()/1000;
				// convert to nanoseconds.
				if (__timebase.denom == 0) {
					(void) mach_timebase_info(&__timebase);
				}
				// now convert the time to real time (usec)
				now = (now * __timebase.numer)/__timebase.denom;
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


		/**
		 * This method takes a timestamp as usec since Epoch and formats
		 * it into a nice, human-readable timestamp: '2012-02-12 11:34:15'
		 * or '2012-02-12 11:34:15.032451' - depending on if you ask for
		 * the usec to be displayed. This is very useful for converting
		 * these timestamps into useful strings for logs, etc.
		 */
		static inline std::string formatTimestamp( uint64_t aTimestamp, bool inclUSec = false )
		{
			// convert the usec since epoch to sec since epoch...
			time_t		secs = aTimestamp / 1000000L;
			// ...and get the usec remaining after that
			uint32_t	usec = (aTimestamp - secs * 1000000L);
			// get the struct and breack out the secs into it's components
			struct tm	when;
			localtime_r(&secs, &when);
			// now build up the string based on all this data
			char	buff[128];
			if (inclUSec) {
				snprintf(buff, 127, "%4d-%02d-%02d %02d:%02d:%02d.%06d",
						 (when.tm_year + 1900), (when.tm_mon + 1),
						 when.tm_mday, when.tm_hour, when.tm_min, when.tm_sec,
						 usec);
			} else {
				snprintf(buff, 127, "%4d-%02d-%02d %02d:%02d:%02d",
						 (when.tm_year + 1900), (when.tm_mon + 1),
						 when.tm_mday, when.tm_hour, when.tm_min, when.tm_sec);
			}
			// return what we have created.
			return buff;
		}


		/**
		 * This method takes a timestamp as usec since Epoch and formats
		 * it into a nice, human-readable date: '2012-02-12'. This totally
		 * ignores the TIME component of the timestamp, and just looks at
		 * the date. This is very useful for converting these timestamps
		 * into useful strings for logs, etc.
		 */
		static inline std::string formatDate( uint64_t aTimestamp )
		{
			// convert the usec since epoch to sec since epoch...
			time_t		secs = aTimestamp / 1000000L;
			// get the struct and breack out the secs into it's components
			struct tm	when;
			localtime_r(&secs, &when);
			// now build up the string based on all this data
			char	buff[80];
			snprintf(buff, 79, "%4d-%02d-%02d",
					 (when.tm_year + 1900), (when.tm_mon + 1), when.tm_mday);
			// return what we have created.
			return buff;
		}


		/**
		 * This method takes a timestamp as usec since Epoch and formats
		 * it into a nice, human-readable time: '11:34:15' or '11:34:15.342567'
		 * depending on if you ask for the usec to be displayed. This is very
		 * useful for converting these timestamps into useful strings for
		 * logs, etc.
		 */
		static inline std::string formatTime( uint64_t aTimestamp, bool inclUSec = false )
		{
			// convert the usec since epoch to sec since epoch...
			time_t		secs = aTimestamp / 1000000L;
			// ...and get the usec remaining after that
			uint32_t	usec = (aTimestamp - secs * 1000000L);
			// get the struct and breack out the secs into it's components
			struct tm	when;
			localtime_r(&secs, &when);
			// now build up the string based on all this data
			char	buff[80];
			if (inclUSec) {
				snprintf(buff, 79, "%02d:%02d:%02d.%06d",
						 when.tm_hour, when.tm_min, when.tm_sec, usec);
			} else {
				snprintf(buff, 79, "%02d:%02d:%02d",
						 when.tm_hour, when.tm_min, when.tm_sec);
			}
			// return what we have created.
			return buff;
		}
};
}		// end of namespace util
}		// end of namespace dkit

#endif		// __DKIT_UTIL_TIMER_H
