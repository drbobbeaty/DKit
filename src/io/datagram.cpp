/**
 * datagram.cpp - this file implements a simple UDP multicast datagram - that
 *                atomic block of data read in/written to a UDP Multicast
 *                channel for processing/transmission. The implementation is
 *                really quite simple - a block of data, but with the added
 *                timestamp of when it was read off the OS socket for timing
 *                purposes.
 */

//	System Headers

//	Third-Party Headers

//	Other Headers
#include "datagram.h"

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
std::size_t hash_value( datagram const & aValue )
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
std::ostream & operator<<( std::ostream & aStream, const dkit::io::datagram & aValue )
{
	aStream << aValue.toString();
	return aStream;
}
