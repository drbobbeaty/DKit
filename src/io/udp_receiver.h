/**
 * udp_receiver.h - this file defines a simple UDP multicast receiver that
 *                  is configured with a multicast_channels and sends
 *                  downstream udp datagrams that it receives from the
 *                  socket. It's a subclass of the source<datagram *>
 *                  class, so it's going to be sending datagram pointers
 *                  to the listeners that they can use or copy, but CANNOT
 *                  keep as we are going to be returning them to the pool
 *                  to keep news and deletes to a minimum.
 */
#ifndef __DKIT_IO_UDP_RECEIVER_H
#define __DKIT_IO_UDP_RECEIVER_H

//	System Headers
#include <string>
#include <stdint.h>
#include <sstream>

//	Third-Party Headers
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>
#include <boost/smart_ptr/detail/spinlock.hpp>

//	Other Headers
#include "source.h"
#include "datagram.h"
#include "multicast_channel.h"
#include "aint32.h"
#include "pool.h"

//	Forward Declarations

//	Public Constants
/**
 * We need to have a default socket receive buffer size, and this is it -
 * 16MB. You can specify this in the constructor, or the listen() method,
 * but if you don't, then you'll get this default which is pretty decent
 * for most applications.
 */
#define	DEFAULT_RCV_BUFFER_SIZE		16777216

//	Public Datatypes
#ifndef __DKIT_IO_SVC_PTR
#define __DKIT_IO_SVC_PTR
namespace dkit {
namespace io {
/**
 * It's possible to share a boost::asio::io_service between many
 * UDP receivers, and it's all done with boost shared_ptr. We will
 * make one - if we don't have one to use, but if we are given another
 * UDP receiver, we'll copy it's smart pointer for the io_service and
 * use it. That way, we have one io_service, and when everyone is done
 * with it, it will automatically be reclaimed.
 */
typedef boost::shared_ptr<boost::asio::io_service> io_svc_ptr;
/**
 * Because we can't really depend on the reference counting of the
 * boost shared_ptr for counting up how many sockets are on any one
 * io_service, and it's associated thread, we need to have a little
 * structure that is the value of the thread map, and contains not
 * only the thread, but also the count of the number of sockets that
 * we are currently processing. This will be updated in the code as
 * sockets are added and removed, and makes it very easy to see when
 * the io_service thread is no longer needed.
 */
typedef struct base_thread_info {
	mutable boost::thread	thread;
	auint32_t				use_count;
	/**
	 * We should throw down the default constructor and copy constructor
	 * as the boost container is going to need these, and we can't allow
	 * the thread itself to be copied. That's just not right. So we mask
	 * that and just let the count be copied. This isn't great, but we
	 * have to have this capability, and this is the simplest way to
	 * make it work.
	 */
	base_thread_info() :
		thread(),
		use_count(0)
	{ }
	base_thread_info( const base_thread_info &anOther ) :
		thread(),
		use_count(anOther.use_count)
	{ }
	virtual ~base_thread_info()
	{ }
} thread_info;
/**
 * When we start to listen to UDP sockets, we need to have a running
 * thread PER io_service. In order to make sure that we keep track of
 * all these threads without making them shared_ptr instances as well,
 * we need to have a static map of all the shared io_service instances
 * to their running boost::threads. This is that map.
 */
typedef boost::unordered_map<io_svc_ptr, thread_info> thread_map;
}		// end of namespace io
}		// end of namespace dkit
#endif	// __DKIT_IO_SVC_PTR

//	Public Data Constants



using namespace boost::asio::ip;
using namespace boost::detail;

namespace dkit {
namespace io {
/**
 * This is the main class definition.
 */
class udp_receiver :
	public source<datagram *>
{
	public:
		/*******************************************************************
		 *
		 *                     Constructors/Destructor
		 *
		 *******************************************************************/
		/**
		 * This is the default constructor that sets up the UDP multicast
		 * receiver with no multicast channel so that it's not really set
		 * up properly to receive data. This needs to be set by the accessor
		 * methods, and then it'll be ready to go.
		 */
		udp_receiver();
		/**
		 * This version of the constructor takes a single multicast channel
		 * and attempts to hook up to that address and port, listening for
		 * datagrams on the wire. When one arrives, this class will bundle
		 * it up in a datagram instance and message all registered listeners
		 * so that they can process the datagram.
		 */
		udp_receiver( const multicast_channel & aChannel, uint32_t aRcvBuffSize = DEFAULT_RCV_BUFFER_SIZE );
		/**
		 * This version of the constructor takes a single multicast channel
		 * and an existing UDP receiver and attempts to hook up to that
		 * address and port, listening for datagrams on the wire. The
		 * boost::io_service of the passed-in UDP receiver will be used
		 * as the io_service of this instance to allow the one thread to
		 * service the original and this new UDP receiver. When a datagram
		 * arrives, this class will bundle it up in a datagram instance
		 * and message all registered listeners so that they can process
		 * the datagram.
		 */
		udp_receiver( const udp_receiver & anOther, const multicast_channel & aChannel );
		/**
		 * This is the standard copy constructor and needs to be in every
		 * class to make sure that we don't have too many things running
		 * around.
		 */
		udp_receiver( const udp_receiver & anOther );

		/**
		 * This is the standard destructor and needs to be virtual to make
		 * sure that if we subclass off this the right destructor will be
		 * called.
		 */
		virtual ~udp_receiver();

		/**
		 * When we want to process the result of an equality we need to
		 * make sure that we do this right by always having an equals
		 * operator on all classes.
		 */
		udp_receiver & operator=( const udp_receiver & anOther );

		/*******************************************************************
		 *
		 *                         Accessor Methods
		 *
		 *******************************************************************/
		/**
		 * This method returns the multicast channel that we are listening
		 * to or will be listening to for datagrams to send to all our
		 * registered listeners. If it's empty then we are waiting for some
		 * valid channel to be given to us in order to start the listening
		 * process.
		 */
		const multicast_channel & getMulticastChannel() const;
		/**
		 * This method returns the multicast channel as a URL in the form
		 * 'udp://addr:port' so that it can be easily used in logging, etc.
		 */
		std::string getURL() const;

		/*******************************************************************
		 *
		 *                       Connectivity Methods
		 *
		 *******************************************************************/
		/**
		 * This method instructs this instance to share the io_service of
		 * the provided UDP receiver, and that MIGHT mean tearing down the
		 * existing socket, if it has already been established. That is
		 * because the socket much be initialized with the io_service
		 * that is going to handle all the async calls for the socket, and
		 * a change requires a re-creation on the socket.
		 */
		void shareService( const udp_receiver & anOther );
		/**
		 * This version of the method takes the boost shared_ptr of the
		 * io_service directly in case you had a separate run loop planned
		 * for a group of ASIO instances.
		 */
		void shareService( const io_svc_ptr & aService );

		/**
		 * This method is used to initialize the UDP socket for listening
		 * to the UDP multicast traffic on the provided multicast channel.
		 * It will return 'true' if the socket can be opened and we are
		 * ready to start the listening process.
		 */
		bool init();
		/**
		 * This method is called in the run loop of the receiver to set
		 * the opened and initialized socket into the proper mode for
		 * listening so that we can being the async reading process.
		 */
		bool listen( const multicast_channel & aChannel, uint32_t aRcvBuffSize = DEFAULT_RCV_BUFFER_SIZE );
		/**
		 * This method is called in the run loop of the receiver to set
		 * the opened and initialized socket into the proper mode for
		 * listening so that we can being the async reading process.
		 * This version uses the previously supplied multicast channel
		 * as opposed to requiring one, as it could have been supplied
		 * in the constructor.
		 */
		bool listen();
		/**
		 * This method returns 'true' if the socket is open on a specific
		 * UDP multicast channel, and the thread is running listening to
		 * datagrams arriving at the socket and shipping them downstream.
		 */
		bool isListening() const;
		/**
		 * This method shuts down the listening, if it's active, and then
		 * closes out the socket and cleans everything up so that this
		 * receiver is back at the state of having a multicast channel
		 * defined, but not initialized and not listening.
		 */
		void shutdown();

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
		virtual std::string toString() const;

		/**
		 * When we have a custom '==' operator, it's wise to have a hash
		 * method as well. This makes it much easier to used hash-based
		 * containers like boost, etc. It's a requirement that if two
		 * instances are '==', then their hash() methods must be '==' as
		 * well.
		 */
		size_t hash() const;

		/**
		 * This method checks to see if two receivers are equal in their
		 * contents and not their pointer values. This is how you'd likely
		 * expect equality to work.
		 */
		bool operator==( const udp_receiver & anOther ) const;
		/**
		 * This method checks to see if two receivers are NOT equal in their
		 * contents and not their pointer values. This is how you'd likely
		 * expect equality to work.
		 */
		bool operator!=( const udp_receiver & anOther ) const;

	protected:
		/*******************************************************************
		 *
		 *                 Low-Level Communication Methods
		 *
		 *******************************************************************/
		/**
		 * This method is the "no-lock" version of the init() method and
		 * is the place where all the magic REALY happens. It is used to
		 * initialize the UDP socket for listening to the UDP multicast
		 * traffic on the provided multicast channel. It will return 'true'
		 * if the socket can be opened and we are ready to start the
		 * listening process.
		 */
		bool init_nl();
		/**
		 * This method is the "no-lock" version of the listen() method and
		 * is the place where all the magic REALY happens. It is called
		 * in the run loop of the receiver to set the opened and initialized
		 * socket into the proper mode for listening so that we can being
		 * the async reading process.
		 */
		bool listen_nl( const multicast_channel & aChannel, uint32_t aRcvBuffSize = DEFAULT_RCV_BUFFER_SIZE );
		/**
		 * This method is the "no-lock" version of the isListening() method
		 * and returns 'true' if the socket is open on a specific
		 * UDP multicast channel, and the thread is running listening to
		 * datagrams arriving at the socket and shipping them downstream.
		 */
		bool isListening_nl();
		/**
		 * This method is the "no-lock" version of the shutdown() method
		 * and shuts down the listening, if it's active, and then
		 * closes out the socket and cleans everything up so that this
		 * receiver is back at the state of having a multicast channel
		 * defined, but not initialized and not listening.
		 */
		void shutdown_nl();

		/*******************************************************************
		 *
		 *                 ASIO Run-Loop Processing Methods
		 *
		 *******************************************************************/
		/**
		 * This method is called when you need to START the async process
		 * of reading from the UDP multicast socket. It's going to return
		 * almost immediately, as the async process is handed off to the
		 * io_service thread, but it's still nice to have a single place
		 * where we can start the process of a read.
		 */
		bool startAsyncRead();
		/**
		 * This method is the "no-lock" version of the startAsyncRead()
		 * method and is called when you need to START the async process
		 * of reading from the UDP multicast socket but you have already
		 * obtained the lock on the mutex. It's going to return
		 * almost immediately, as the async process is handed off to the
		 * io_service thread, but it's still nice to have a single place
		 * where we can start the process of a read.
		 */
		bool startAsyncRead_nl();
		/**
		 * This method will be called by the io_service thread when a complete
		 * UDP datagram is read at the socket and it's time to pass it back to
		 * us for processing. This method takes the datagram that now has
		 * the data in it's posession, and a possible error code and the
		 * number of bytes placed in that datagram. This is all we need to
		 * finish packaging up the datagram and sending it on it's way.
		 */
		void handleAsyncRead( datagram *aPayload,
							  const boost::system::error_code & anError,
							  size_t aBytesReceived );

		/**
		 * This method looks at the provided io_service, and if it's
		 * not currently running, it makes sure to start a boost::thread
		 * to run it, and then adds it to the map of running threads. This
		 * is a very simple way for the code to ensure that each io_service
		 * of a UDP receiver is running without the code getting too complex.
		 */
		static bool verifyProcessing( const io_svc_ptr & aService, bool addWork = false );
		/**
		 * This static method is the target of the boost::thread that will
		 * take the shared_ptr<io_service> to use in it's run loop, and will
		 * keep calling it's run() method, with the option to add a fixed
		 * work unit to the io_service, so that the ASIO events for those
		 * sockets and devices continue to be serviced and processed. This
		 * will continue until it's forced to stop so that it's something
		 * akin to "fire and forget".
		 */
		static void process( io_svc_ptr & aService, bool addWork = false );
		/**
		 * This method returns 'true' if the provided io_service is currently
		 * being run by a boost::thread, and we have verified that things
		 * are OK with it. If not, this will return 'false'.
		 */
		static bool isRunning( const io_svc_ptr & aService );
		/**
		 * This method will terminate the running boost::thread that's
		 * currently processing the provided io_service. This is only
		 * done if we are SURE that there are no other receivers using
		 * this io_service, and if that's the case, we can freely shut
		 * this down as it's no longer needed.
		 */
		static void terminate( const io_svc_ptr & aService );

		/**
		 * This method is used when a new socket is created with the
		 * provided io_service shared pointer so that we can increment
		 * the usage count on that io_service. The use_count() of the
		 * shared_ptr itself is a little unreliable, and so we needed
		 * to make one of our own.
		 *
		 * The return value is 'true' if the resulting value is
		 * greater than zero - meaning that there is work to be done.
		 */
		static bool incr_use_count( const io_svc_ptr & aService );
		/**
		 * This method is used when a socket is deleted off the
		 * provided io_service shared pointer so that we can decrement
		 * the usage count on that io_service. The use_count() of the
		 * shared_ptr itself is a little unreliable, and so we needed
		 * to make one of our own.
		 *
		 * The return value is 'true' if the resulting value is
		 * greater than zero - meaning that there is still work to
		 * be done.
		 */
		static bool decr_use_count( const io_svc_ptr & aService );
		/**
		 * This method gets the current use count (in number of sockets
		 * currently using the io_service) of the provided io_service.
		 * This is very useful for knowing whether or not we need to
		 * continue servicing this io_service thread.
		 */
		static uint32_t use_count( const io_svc_ptr & aService );

	private:
		/**
		 * We are going to allow the caller to set the receive buffer size
		 * that we'll ask for from the OS. If the OS isn't set up for this,
		 * we'll get the biggest it'll allow, but that's out of our control.
		 */
		uint32_t			_rcv_buff_size;

		/**
		 * This is the multicast channel that we will be listening to and
		 * packaging datagrams for out registered listeners. It's set
		 * in the constructor or the init() method, and then used to make
		 * the connection and log what's happening.
		 */
		multicast_channel	_channel;
		/**
		 * The boost ASIO library handles async operations by having each
		 * socket created (bound) to an io_service and then that service's
		 * run() method handles all pending actions. We are using the boost
		 * shared_ptr to allow the first UDP receiver to create the
		 * io_service, and then when another is initialized with this
		 * existing UDP receiver, then it will grab the shared_ptr, and
		 * hold onto it as long as it needs.
		 */
		io_svc_ptr			_service;
		/**
		 * This is the pointer to the socket that we'll create with the
		 * correct boost::asio::io_service. The trick is that when we
		 * initialize the socket, we need to know which thread is going
		 * to service the async requests for this socket. You see, the
		 * io_service is the "binding" glue for the sockets to get serviced.
		 * So I need to know which one I'm using.
		 */
		udp::socket			*_socket;
		// this is a simple sinlock that makes sure we're thread-safe
		mutable spinlock	_mutex;

		/**
		 * All receivers need to know the specific boost::thread for a
		 * given boost::io_service - and this map is how they find that
		 * out. When a receiver is asked to "start listening", it looks
		 * to see if it's io_service is in this map, and therefore already
		 * running. If it is, then all is OK, and we continue. If not,
		 * we then start the io_service processing thread and put it in
		 * this map. That way, we can see what's running, and start
		 * what's needed.
		 */
		static thread_map	_threads;
		// this is a simple sinlock that makes sure we're thread-safe
		static spinlock		_threads_mutex;
		/**
		 * This is the datagram pool of up to 2^16 (64k) datagrams
		 * available to be used - in a SP/SC strategy. This is what
		 * we need because the io_service thread is all that's going
		 * to be pulling these off, and placing them back on. It's a
		 * pretty simple arrangement, actually, and there is really
		 * not the need for much of a pool, but we'll have one, just
		 * in case.
		 */
		static pool<datagram *, 16, dkit::sp_sc>	_pool;
};

/**
 * In order to allow this object to be a key in the hash-based containers
 * of boost, it's essential that we have the following function as it's
 * what boost expects from all it's supported classes.
 */
std::size_t hash_value( udp_receiver const & aValue );
}		// end of namespace io
}		// end of namespace dkit

/**
 * For debugging purposes, let's make it easy for the user to stream
 * out this value. It basically is just the value of the base data type
 * and puts it into the stream.
 */
std::ostream & operator<<( std::ostream & aStream, const dkit::io::udp_receiver & aValue );

#endif		// __DKIT_IO_UDP_RECEIVER_H
