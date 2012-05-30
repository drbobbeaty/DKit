/**
 * multicast_channel.h - this file defines a simple UDP multicast channel
 *                       class for DKit - something that identifies an
 *                       address and a port for simple UDP multicast traffic.
 *                       This can be used in reading from multicast channels
 *                       and writing to them as well.
 *
 *                       Since we have decided to use boost in DKit, it
 *                       makes good sense to utilize it for all we can in
 *                       it's ASIO library. This includes the async reading
 *                       and writing as well as the simpler classes like
 *                       endpoints and sockets.
 */
#ifndef __DKIT_IO_MULTICAST_CHANNEL_H
#define __DKIT_IO_MULTICAST_CHANNEL_H

//	System Headers
#include <string>
#include <stdint.h>
#include <sstream>

//	Third-Party Headers
#include <boost/asio.hpp>
#include <boost/functional/hash.hpp>

//	Other Headers
#include "abool.h"

//	Forward Declarations

//	Public Constants

//	Public Datatypes

//	Public Data Constants


using namespace boost::asio::ip;

namespace dkit {
namespace io {
/**
 * This is the main class definition.
 */
struct multicast_channel
{
	/**
	 * Since we're using boost, and we're all about a UDP multicast
	 * channel, it makes sense to use a boost endpoint here to hold
	 * the details of the address and port as well as a few other
	 * housekeeping things.
	 */
	udp::endpoint	endpoint;
	/**
	 * In addition to the endpoint, we typically have some channel
	 * metadata that helps in the use of this channel in a larger
	 * context. For instance, in exchange feeds there is a name and
	 * a primary channel in a group - these things need to be taken
	 * into account here so that this can be used for exchange feeds.
	 */
	// this is the simple channel identifier - A, B, etc.
	char			id;
	// this is the boolean saying if this channel is the primary in a group
	abool			primary;

	public:
		/*******************************************************************
		 *
		 *                     Constructors/Destructor
		 *
		 *******************************************************************/
		/**
		 * This is the default constructor that sets up the multicast channel
		 * with no endpoint or associated metadata. This isn't very good by
		 * itself, but the values can be set, and this default constructor
		 * is necessary if we are going to place these into an STL container.
		 */
		multicast_channel() :
			endpoint(),
			id('\0'),
			primary(false)
		{
		}


		/**
		 * This form of the constructor sets up the multicast channel with
		 * the provided address and port number. The rest of the metadata
		 * is left in it's default state.
		 */
		multicast_channel( const std::string & anAddr, uint32_t aPort ) :
			endpoint(address::from_string(anAddr), aPort),
			id('\0'),
			primary(false)
		{
		}


		/**
		 * This form of the constructor sets up the multicast channel with
		 * the provided address and port number embedded in the URL. The
		 * rest of the metadata is left in it's default state.
		 */
		multicast_channel( const std::string & aURL ) :
			endpoint(),
			id('\0'),
			primary(false)
		{
			// set the endpoint from the given URL
			setEndpoint(aURL);
		}


		/**
		 * This form of the constructor sets up the multicast channel with
		 * the provided address and port number. The rest of the metadata
		 * is included as well - with the 'primary' flag being optional and
		 * defaulting to 'true'.
		 */
		multicast_channel( const std::string & anAddr, uint32_t aPort, char anID, bool aPrimary = true ) :
			endpoint(address::from_string(anAddr), aPort),
			id(anID),
			primary(aPrimary)
		{
		}


		/**
		 * This form of the constructor sets up the multicast channel with
		 * the provided address and port number in a URL. The rest of the
		 * metadata is included as well - with the 'primary' flag being
		 * optional and defaulting to 'true'.
		 */
		multicast_channel( const std::string & aURL, char anID, bool aPrimary = true ) :
			endpoint(),
			id(anID),
			primary(aPrimary)
		{
			// set the endpoint from the given URL
			setEndpoint(aURL);
		}


		/**
		 * This is the standard copy constructor and needs to be in every
		 * class to make sure that we don't have too many things running
		 * around.
		 */
		multicast_channel( const multicast_channel & anOther ) :
			endpoint(),
			id('\0'),
			primary(false)
		{
			// let the '=' operator do the heavy lifting...
			*this = anOther;
		}


		/**
		 * This is the standard destructor and needs to be virtual to make
		 * sure that if we subclass off this the right destructor will be
		 * called.
		 */
		virtual ~multicast_channel()
		{
			// not much to do here - yet
		}


		/**
		 * When we want to process the result of an equality we need to
		 * make sure that we do this right by always having an equals
		 * operator on all classes.
		 */
		multicast_channel & operator=( const multicast_channel & anOther )
		{
			if (this != & anOther) {
				endpoint = anOther.endpoint;
				id = anOther.id;
				primary = anOther.primary;
			}
			return *this;
		}


		/*******************************************************************
		 *
		 *                         Accessor Methods
		 *
		 *******************************************************************/
		/**
		 * This method gets the multicast channel as a URL of the form:
		 * 'udp://<addr>:<port>' so that it can be used as a human-readable
		 * version of the endpoint's data. This is primarily for logging and
		 * such.
		 */
		std::string getURL() const
		{
			char	url[80];
			bzero(url, 80);
			if (!empty()) {
				snprintf(url, 79, "udp://%s:%d",
						 endpoint.address().to_string().c_str(),
						 endpoint.port());
			}
			return url;
		}


		/**
		 * This method returns true if the instance is not yet initialized
		 * to something. This will happen if the default constructor is
		 * used, and no assignments have been made to the endpoint or id.
		 */
		bool empty() const
		{
			return ((endpoint.port() == 0) && (id == '\0'));
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
			msg << "[multicast_channel ";
			if (id != '\0') {
				msg << id << "=";
			}
			msg << getURL() << ((bool)primary ? " Primary" : "") << "]";
			return msg.str();
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
			size_t	ans = boost::hash_value(endpoint.address().to_string());
			boost::hash_combine(ans, endpoint.port());
			boost::hash_combine(ans, id);
			boost::hash_combine(ans, ((bool)primary));
			return ans;
		}


		/**
		 * This method checks to see if two channels are equal in their
		 * contents and not their pointer values. This is how you'd likely
		 * expect equality to work.
		 */
		bool operator==( const multicast_channel & anOther ) const
		{
			bool	equals = false;

			if ((this == & anOther) ||
				((endpoint == anOther.endpoint) &&
				 (id == anOther.id) &&
				 (primary == anOther.primary))) {
				equals = true;
			}

			return equals;
		}


		/**
		 * This method checks to see if two channels are NOT equal in their
		 * contents and not their pointer values. This is how you'd likely
		 * expect equality to work.
		 */
		bool operator!=( const multicast_channel & anOther ) const
		{
			return !operator==(anOther);
		}

	protected:
		/*******************************************************************
		 *
		 *                         Accessor Methods
		 *
		 *******************************************************************/
		/**
		 * This method provides the functionality to set the boost::endpoint
		 * with a single URL - something boost didn't have. We want to be
		 * able to do this just for the simplicity of specifying a new
		 * multicast channel.
		 */
		void setEndpoint( const std::string & aURL )
		{
			/**
			 * We need to parse the URL into the address and port. Start
			 * by looking for the '://' and only keep what's AFTER that,
			 * if it's there.
			 */
			size_t	pos = aURL.find("//");
			if (pos != std::string::npos) {
				// go past the "//" to the address
				pos += 2;
			} else {
				// the address starts at the first character
				pos = 0;
			}
			// now get the address/port separator
			size_t	sep = aURL.find(":", pos);
			if (sep == std::string::npos) {
				throw std::runtime_error("Improperly formatted URL! No port available!");
			} else {
				// get the address and port as std::string values
				std::string	addr = aURL.substr(pos, (sep - pos));
				std::string	pstr = aURL.substr(sep+1);
				// now set the endpoint's values from these
				endpoint.address(address::from_string(addr));
				endpoint.port(atoi(pstr.c_str()));
			}
		}
};

/**
 * In order to allow this object to be a key in the hash-based containers
 * of boost, it's essential that we have the following function as it's
 * what boost expects from all it's supported classes.
 */
std::size_t hash_value( multicast_channel const & aValue );
}		// end of namespace io
}		// end of namespace dkit

/**
 * For debugging purposes, let's make it easy for the user to stream
 * out this value. It basically is just the value of the base data type
 * and puts it into the stream.
 */
std::ostream & operator<<( std::ostream & aStream, const dkit::io::multicast_channel & aValue );

#endif		// __DKIT_IO_MULTICAST_CHANNEL_H
