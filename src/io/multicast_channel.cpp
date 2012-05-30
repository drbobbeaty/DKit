/**
 * multicast_channel.cpp - this file implements a simple UDP multicast channel
 *                         class for DKit - something that identifies an
 *                         address and a port for simple UDP multicast traffic.
 *                         This can be used in reading from multicast channels
 *                         and writing to them as well.
 *
 *                         Since we have decided to use boost in DKit, it
 *                         makes good sense to utilize it for all we can in
 *                         it's ASIO library. This includes the async reading
 *                         and writing as well as the simpler classes like
 *                         endpoints and sockets.
 */

//	System Headers

//	Third-Party Headers

//	Other Headers
#include "multicast_channel.h"

//	Forward Declarations

//	Private Constants

//	Private Datatypes

//	Private Data Constants


namespace dkit {
namespace io {

/**
 * In order to allow this object to be a key in the hash-based containers
 * of boost, it's essential that we have the following function as it's
 * what boost expects from all it's supported classes.
 */
std::size_t hash_value( multicast_channel const & aValue )
{
	return aValue.hash();
}
}		// end of namespace io
}		// end of namespace dkit


/**
 * For debugging purposes, let's make it easy for the user to stream
 * out this value. It basically is just the value of the base data type
 * and puts it into the stream.
 */
std::ostream & operator<<( std::ostream & aStream, const dkit::io::multicast_channel & aValue )
{
	aStream << aValue.toString();
	return aStream;
}
