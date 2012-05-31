/**
 * datagram.h - this file defines a simple UDP multicast datagram - that
 *              atomic block of data read in/written to a UDP Multicast
 *              channel for processing/transmission. The implementation is
 *              really quite simple - a block of data, but with the added
 *              timestamp of when it was read off the OS socket for timing
 *              purposes.
 */
#ifndef __DKIT_IO_DATAGRAM_H
#define __DKIT_IO_DATAGRAM_H

//	System Headers
#include <string>
#include <stdint.h>
#include <sstream>
#include <string.h>
#include <strings.h>

//	Third-Party Headers
#include <boost/functional/hash.hpp>

//	Other Headers
#include "util/timer.h"

//	Forward Declarations

//	Public Constants
/**
 * This is the default datagram size for a default constructed datagram.
 * You can resize it, and you can create one any size you wish, but this
 * is the default that should work for most UDP multicast traffic.
 */
#define DEFAULT_DATAGRAM_SIZE	1024

//	Public Datatypes

//	Public Data Constants


namespace dkit {
namespace io {
/**
 * This is the main class definition.
 */
struct datagram
{
	/**
	 * The data coming from a UDP Multicast Channel is a simple datagram.
	 * These are a simple series of bytes, but in order to make them a
	 * little more usable, we're going to include the microseconds since
	 * epoch timestamp of when they arrived from the NIC/OS level socket
	 * into this process space. We will try to minimize the delay so as
	 * to keep this as true to the arrival time as possible.
	 *
	 * The resources used by this datagram will be cleaned up in the
	 * destructor, as needed.
	 */
	uint64_t	when;
	uint32_t	size;
	uint32_t	capacity;
	char		*what;

	public:
		/*******************************************************************
		 *
		 *                     Constructors/Destructor
		 *
		 *******************************************************************/
		/**
		 * This is the default constructor that sets up the datagram
		 * with NO data and a timestamp of epoch. It's needed for being
		 * able to assign values, but that's about it.
		 */
		datagram() :
			when(0),
			size(0),
			capacity(0),
			what(NULL)
		{
			// make it the default size
			if ((what = new char[DEFAULT_DATAGRAM_SIZE]) != NULL) {
				// if successful, save the capacity
				capacity = DEFAULT_DATAGRAM_SIZE;
				// ...and zero out the data itself
				bzero(what, capacity);
			}
		}


		/**
		 * This form of the constructor takes a buffer capacity to
		 * create. If it's successful, the buffer will be non-NULL, but
		 * you might want to check it before using it.
		 */
		datagram( size_t aCapacity ) :
			when(0),
			size(0),
			capacity(0),
			what(NULL)
		{
			// try to create the buffer of the requested size
			if (aCapacity > 0) {
				// make it as big as the user has requested
				if ((what = new char[aCapacity]) != NULL) {
					// if successful, save the capacity
					capacity = aCapacity;
					// ...and zero out the data itself
					bzero(what, capacity);
				}
			}
		}


		/**
		 * This is the standard copy constructor and needs to be in every
		 * class to make sure that we don't have too many things running
		 * around.
		 */
		datagram( const datagram & anOther ) :
			when(0),
			size(0),
			capacity(0),
			what(NULL)
		{
			// let the '=' operator do the heavy lifting...
			*this = anOther;
		}


		/**
		 * This is the standard destructor and needs to be virtual to make
		 * sure that if we subclass off this the right destructor will be
		 * called.
		 */
		virtual ~datagram()
		{
			// we just need to drop what it is we have allocated
			if (what != NULL) {
				delete [] what;
				what = NULL;
			}
		}


		/**
		 * When we want to process the result of an equality we need to
		 * make sure that we do this right by always having an equals
		 * operator on all classes.
		 */
		datagram & operator=( const datagram & anOther )
		{
			// don't do this to myself...
			if (this != & anOther) {
				// first, clear out the data we're holding in the buffer
				clear();
				// now, if there's something to copy in, let's do that.
				if (!anOther.empty()) {
					// make sure we're big enough to hold the data
					if (ensureCapacity(anOther.capacity) && (what != NULL)) {
						// copy in the data from him...
						memcpy(what, anOther.what, anOther.size);
						// ...and then his size and timestamp
						size = anOther.size;
						when = anOther.when;
					}
				}
			}
			return *this;
		}


		/**
		 * This method allows the caller to clone this instance into an
		 * exact duplicate of the data contained in this instance which is
		 * now under the control of the caller. This is often needed when
		 * processing through a queue.
		 */
		datagram *clone() const
		{
			return new datagram(*this);
		}


		/*******************************************************************
		 *
		 *                         Accessor Methods
		 *
		 *******************************************************************/
		/**
		 * This method can be called on the datagram to make sure that there
		 * is sufficient capacity for any pending operations. If there is
		 * sufficient capacity in the datagram, this method will do nothing
		 * and return 'true'. If there isn't, a new buffer will attempt to
		 * be created of sufficient size, and the current contents of this
		 * datagram will be moved over into that new space. This is not an
		 * efficient operation, but it can be quite useful at times - if used
		 * wisely.
		 */
		bool ensureCapacity( size_t aCapacity )
		{
			bool		error = false;

			// see if we need to do anything
			if ((what == NULL) || (capacity < aCapacity)) {
				// create a new buffer of the right size
				char	*temp = new char[aCapacity];
				if (temp == NULL) {
					// trouble - couldn't get it, gotta fail
					error = true;
				} else {
					// see if we have something to move into the new space
					if (what != NULL) {
						memcpy(temp, what, capacity);
						delete [] what;
					}
					// save the new as my current buffer with capacity
					what = temp;
					capacity = aCapacity;
				}
			}

			return !error;
		}


		/**
		 * This method clears out the contents of the datagram - ignoring
		 * any existing data and treating this as a newly created datagram
		 * with a similar capacity.
		 */
		void clear()
		{
			// reset things for this datagram
			when = 0;
			size = 0;
			if (what != NULL) {
				bzero(what, capacity);
			}
		}


		/**
		 * This method will return 'true' of the datagram hold no data. This
		 * is not to say that it's got no buffer, though that's a distinct
		 * possibility, this is to say that it's holding nothing of interest.
		 */
		bool empty() const
		{
			return ((what == NULL) || (size == 0));
		}


		/**
		 * This method sets the time and size of the datagram, and can be
		 * used when a number of bytes has been deposited into the datagram.
		 * Such might be done on an async read when the buffer is the
		 * contents of this datagram, and this sets the number of bytes
		 * read and when it was read.
		 */
		void markTimeAndSize( uint32_t aSize )
		{
			size = aSize;
			when = util::timer::usecSinceEpoch();
		}


		/*******************************************************************
		 *
		 *                         Utility Methods
		 *
		 *******************************************************************/
		/**
		 * There are a lot of times that a human-readable version of
		 * this instance will come in handy. This is that method. It's
		 * not necessarily meant to be something to process, but most
		 * likely what a debugging system would want to write out for
		 * this guy.
		 */
		virtual std::string toString() const
		{
			std::ostringstream	msg;
			msg << "[datagram size=" << size << ", capacity=" << capacity;
			if (when > 0) {
				msg << ", when=" << util::timer::formatTimestamp(when, true);
			}
			msg << "]";
			return msg.str();
		}


		/**
		 * There will be times when we want to see EVERYTHING about the
		 * datagram - the size, capacity, timestamp and even the contents.
		 * This method gives us that in a convenient way because the datagram
		 * contents is really a bunch of bytes - so we include them in hex
		 * format so that it's easy to see what's what.
		 */
		virtual std::string contents() const
		{
			std::string		msg("[datagram ");
			char	buff[128];
			snprintf(buff, 127, "size=%u/%u at:%s (", size, capacity,
					 util::timer::formatTimestamp(when, true).c_str());
			msg.append(buff);
			std::string		ascii("\"");
			for (uint32_t i = 0; i < size; ++i) {
				// add in the hex values
				snprintf(buff, 127, "%s%02x", (i > 0 ? " " : ""), what[i]);
				msg.append(buff);
				// ...and make the ascii equivalent as well
				if (isprint(what[i])) {
					ascii.push_back(what[i]);
				} else {
					ascii.push_back('.');
				}
			}
			ascii.push_back('"');
			msg.append(") = ");
			msg.append(ascii);
			msg.append("]");
			return msg;
		}


		/**
		 * When we have a custom '==' operator, it's wise to have a hash
		 * method as well. This makes it much easier to used hash-based
		 * containers like boost, etc. It's a requirement that if two
		 * instances are '==', then their hash() methods must be '==' as
		 * well.
		 */
		size_t hash() const
		{
			size_t	ans = murmur_hash_2(what, size, capacity);
			boost::hash_combine(ans, size);
			boost::hash_combine(ans, capacity);
			boost::hash_combine(ans, when);
			return ans;
		}


		/**
		 * This method is an improved hasher over the FNV hash that we
		 * have used in the past. It's support is significant, and the
		 * studies have show it to be far faster and more evenly distributed
		 * than FNV. For this reason, we'll be using this as the primary
		 * hashing function for data in DKit.
		 */
		static inline uint32_t murmur_hash_2( const void *key, uint32_t len, uint32_t seed )
		{
			// 'm' and 'r' are mixing constants generated offline.
			// They're not really 'magic', they just happen to work well.
			const uint32_t	m = 0x5bd1e995;
			const int32_t	r = 24;

			// Initialize the hash to a 'random' value
			uint32_t	h = seed ^ len;

			// Mix 4 bytes at a time into the hash
			const uint8_t	*data = (const uint8_t *)key;
			if (data != NULL) {
				while (len >= 4) {
					uint32_t	k = *(uint32_t *)data;

					k *= m;
					k ^= k >> r;
					k *= m;

					h *= m;
					h ^= k;

					data += 4;
					len -= 4;
				}
			}

			// Handle the last few bytes of the input array
			switch(len) {
				case 3: h ^= data[2] << 16;
				case 2: h ^= data[1] << 8;
				case 1: h ^= data[0];
						h *= m;
			};

			// Do a few final mixes of the hash to ensure the last few
			// bytes are well-incorporated.
			h ^= h >> 13;
			h *= m;
			h ^= h >> 15;

			return h;
		}


		/**
		 * This method is the "classic" FNV 32-bit hashing function. I have
		 * read online that this has some serious problems with both speed
		 * and collisions, so I'm not planning on using this unless it
		 * becomes necessary for compatibility reasons. It's still a decent
		 * hash, but the Murmur Hash is faster and better.
		 */
		#define FNV_PRIME_32	16777619
		#define FNV_OFFSET_32	2166136261U
		static inline uint32_t FNV32( const char *s, uint32_t len )
		{
			uint32_t	hash = FNV_OFFSET_32;
			uint32_t	i = 0;
			if ((s != NULL) && (len > 0)) {
				for (i = 0; i < len; i++) {
					// xor next byte into the bottom of the hash
					hash = hash ^ (s[i]);
					// Multiply by prime number found to work well
					hash = hash * FNV_PRIME_32;
				}
			}
			return hash;
		}


		/**
		 * This method checks to see if two channels are equal in their
		 * contents and not their pointer values. This is how you'd likely
		 * expect equality to work.
		 */
		bool operator==( const datagram & anOther ) const
		{
			bool	equals = (this == & anOther);

			if (!equals && (size == anOther.size) &&
				(capacity == anOther.capacity) &&
				(when == anOther.when)) {
				if ((what == NULL) && (anOther.what == NULL)) {
					equals = true;
				} else if ((what != NULL) && (anOther.what != NULL)) {
					equals = (memcmp(what, anOther.what, size) == 0);
				}
			}

			return equals;
		}


		/**
		 * This method checks to see if two channels are NOT equal in their
		 * contents and not their pointer values. This is how you'd likely
		 * expect equality to work.
		 */
		bool operator!=( const datagram & anOther ) const
		{
			return !operator==(anOther);
		}
};

/**
 * In order to allow this object to be a key in the hash-based containers
 * of boost, it's essential that we have the following function as it's
 * what boost expects from all it's supported classes.
 */
std::size_t hash_value( datagram const & aValue );
}		// end of namespace io
}		// end of namespace dkit

/**
 * For debugging purposes, let's make it easy for the user to stream
 * out this value. It basically is just the value of the base data type
 * and puts it into the stream.
 */
std::ostream & operator<<( std::ostream & aStream, const dkit::io::datagram & aValue );

#endif		// __DKIT_IO_DATAGRAM_H
