/**
 * tcp_transmitter.h - this file defines a simple TCP transmitter
 *                     that is configured with a channels and takes
 *                     datagrams that it receives from sources and sends
 *                     them out the socket. It's a subclass of the
 *                     sink<datagram *> class, so it's going to be taking
 *                     datagram pointers and throwing them out to waiting
 *                     TCP receivers.
 */
#ifndef __DKIT_IO_TCP_TRANSMITTER_H
#define __DKIT_IO_TCP_TRANSMITTER_H

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
#include "sink.h"
#include "datagram.h"
#include "channel.h"
#include "pool.h"
#include "aint32.h"

//	Forward Declarations

//	Public Constants
/**
 * We need to have a default socket send buffer size, and this is it -
 * 16MB. You can specify this in the constructor, or the init() method,
 * but if you don't, then you'll get this default which is pretty decent
 * for most applications.
 */
#define	DEFAULT_XMIT_BUFFER_SIZE	16777216

//	Public Datatypes
#ifndef __DKIT_IO_SVC_PTR
#define __DKIT_IO_SVC_PTR
namespace dkit {
namespace io {
/**
 * It's possible to share a boost::asio::io_service between many
 * TCP transmitters, and it's all done with boost shared_ptr. We will
 * make one - if we don't have one to use, but if we are given another
 * TCP transmitter, we'll copy it's smart pointer for the io_service and
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
 * When we start to send data to TCP sockets, we need to have a running
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
 * In order to make the TCP transmitter a simple object to use, we need to
 * have a "private" class - 'detail' namespace in the boost parlance, that
 * subclasses the sink<T>, and is itself, a template class. This class will
 * have the recv() method implemented to call the virtual method onMessage()
 * for the datatype we expect to handle: <datagram*>.
 *
 * This class will then be subclassed by the "real" TCP transmitter class
 * which will NOT be a template class - but will subclass the specialized
 * template class we're creating here. This is how we need to wire things
 * up with templates in order to get the virtualization we need on the
 * recv() method.
 */
namespace detail {
template <class T> class basic_tcp_transmitter :
	public sink<T>
{
	public:
		basic_tcp_transmitter() { }
		virtual ~basic_tcp_transmitter() { }

		/**
		 * This is the main receiver method that we need to call out to
		 * a concrete method for the type we're using. It's what we have
		 * to do to really get a virtual template class working for us.
		 */
		virtual bool recv( const T anItem )
		{
			return onMessage(anItem);
		}

		/**
		 * This method is called when we get a new datagram, and because
		 * we are expecting to instantiate this template class with the
		 * type 'T' being a <datagram *>, this is the method we're expecting
		 * to get hit. It's just that simple.
		 */
		virtual bool onMessage( const datagram *dg )
		{
			/**
			 * This method will be overridden by the specialization of
			 * this template class.
			 */
			return true;
		}
};
}		// end of namespace detail



/**
 * This is the main class definition.
 */
class tcp_transmitter :
	public detail::basic_tcp_transmitter<datagram*>
{
	public:
		/*******************************************************************
		 *
		 *                     Constructors/Destructor
		 *
		 *******************************************************************/
		/**
		 * This is the default constructor that sets up the TCP
		 * transmitter with no channel so that it's not really set
		 * up properly to send data. This needs to be set by the accessor
		 * methods, and then it'll be ready to go.
		 */
		tcp_transmitter();
		/**
		 * This version of the constructor takes a single channel
		 * and attempts to hook up to that address and port, ready to write
		 * datagrams on the wire. When one arrives, this class will bundle
		 * it up in a shared pointer and send it out async on the TCP channel.
		 */
		tcp_transmitter( const channel & aChannel, uint32_t aXmitBuffSize = DEFAULT_XMIT_BUFFER_SIZE );
		/**
		 * This version of the constructor takes a single channel
		 * and an existing TCP transmitter and attempts to hook up to that
		 * address and port, ready to send datagrams on the wire. The
		 * boost::io_service of the passed-in TCP transmitter will be used
		 * as the io_service of this instance to allow the one thread to
		 * service the original and this new TCP transmitter. When a datagram
		 * arrives, this class will bundle it up in a shared pointer and
		 * send it out async on the TCP channel.
		 */
		tcp_transmitter( const tcp_transmitter & anOther, const channel & aChannel );
		/**
		 * This is the standard copy constructor and needs to be in every
		 * class to make sure that we don't have too many things running
		 * around.
		 */
		tcp_transmitter( const tcp_transmitter & anOther );

		/**
		 * This is the standard destructor and needs to be virtual to make
		 * sure that if we subclass off this the right destructor will be
		 * called.
		 */
		virtual ~tcp_transmitter();

		/**
		 * When we want to process the result of an equality we need to
		 * make sure that we do this right by always having an equals
		 * operator on all classes.
		 */
		tcp_transmitter & operator=( const tcp_transmitter & anOther );

		/*******************************************************************
		 *
		 *                         Accessor Methods
		 *
		 *******************************************************************/
		/**
		 * This method returns the channel that we are sending
		 * to or will be sending out datagrams. If it's empty then we are
		 * waiting for some valid channel to be given to us in order to
		 * start the sending process.
		 */
		const channel & getChannel() const;
		/**
		 * This method returns the channel as a URL in the form
		 * 'tcp://addr:port' so that it can be easily used in logging, etc.
		 */
		std::string getURL() const;

		/*******************************************************************
		 *
		 *                       Connectivity Methods
		 *
		 *******************************************************************/
		/**
		 * This method instructs this instance to share the io_service of
		 * the provided TCP transmitter, and that MIGHT mean tearing down the
		 * existing socket, if it has already been established. That is
		 * because the socket much be initialized with the io_service
		 * that is going to handle all the async calls for the socket, and
		 * a change requires a re-creation on the socket.
		 */
		void shareService( const tcp_transmitter & anOther );
		/**
		 * This version of the method takes the boost shared_ptr of the
		 * io_service directly in case you had a separate run loop planned
		 * for a group of ASIO instances.
		 */
		void shareService( const io_svc_ptr & aService );

		/**
		 * This method is used to initialize the TCP socket for sending
		 * to the TCP traffic on the provided channel.
		 * It will return 'true' if the socket can be opened and we are
		 * ready to start the sending process.
		 */
		bool init( const channel & aChannel, uint32_t aXmitBuffSize = DEFAULT_XMIT_BUFFER_SIZE );
		/**
		 * This method shuts down the sending, if it's active, and then
		 * closes out the socket and cleans everything up so that this
		 * transmitter is back at the state of having a channel
		 * defined, but not initialized and not sending.
		 */
		void shutdown();

		/********************************************************
		 *
		 *                Processing Methods
		 *
		 ********************************************************/
		/**
		 * This is the specialized template method that will be called from
		 * the generalized recv() method, above, and will, in fact, be the
		 * place where we do all the work of sending out the datagram to the
		 * TCP channel.
		 */
		virtual bool onMessage( const datagram *aDatagram );

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
		 * This method checks to see if two transmitters are equal in their
		 * contents and not their pointer values. This is how you'd likely
		 * expect equality to work.
		 */
		bool operator==( const tcp_transmitter & anOther ) const;
		/**
		 * This method checks to see if two transmitters are NOT equal in their
		 * contents and not their pointer values. This is how you'd likely
		 * expect equality to work.
		 */
		bool operator!=( const tcp_transmitter & anOther ) const;

	protected:
		/*******************************************************************
		 *
		 *                 Low-Level Communication Methods
		 *
		 *******************************************************************/
		/**
		 * This method is the "no-lock" version of the init() method and
		 * is the place where all the magic REALY happens. It is used to
		 * initialize the TCP socket for sending to the TCP
		 * traffic on the provided channel. It will return 'true'
		 * if the socket can be opened and we are ready to start the
		 * sending process.
		 */
		bool init_nl( const channel & aChannel, uint32_t aXmitBuffSize = DEFAULT_XMIT_BUFFER_SIZE );
		/**
		 * This method is the "no-lock" version of the shutdown() method
		 * and shuts down the sending, if it's active, and then
		 * closes out the socket and cleans everything up so that this
		 * receiver is back at the state of having a channel
		 * defined, but not initialized and not sending.
		 */
		void shutdown_nl();

		/*******************************************************************
		 *
		 *                 ASIO Run-Loop Processing Methods
		 *
		 *******************************************************************/
		/**
		 * This method is called when you have a datagram that needs to be
		 * sent out to the TCP channel, and you want to do so in
		 * an async manner. This will start the process with the provided
		 * datagram, and will copy the data so that it's not necessary to
		 * keep the datagram around through the duration of the async send.
		 */
		bool asyncSend( const datagram *aDatagram );
		/**
		 * This method is the "no-lock" version of the asyncSend()
		 * method and is called when you have a datagram that needs to be
		 * sent out to the TCP channel, and you want to do so in
		 * an async manner. This will start the process with the provided
		 * datagram, and will copy the data so that it's not necessary to
		 * keep the datagram around through the duration of the async send.
		 */
		bool asyncSend_nl( const datagram *aDatagram );
		/**
		 * This method will be called by the io_service thread when a complete
		 * TCP datagram is sent at the socket and it's time to pass it back to
		 * us for processing. This method takes the datagram that has been
		 * sent and recycles it to the pool to be used on the next send.
		 */
		void asyncSendComplete( datagram *aPayload,
								const boost::system::error_code & anError,
								size_t aBytesReceived );

		/**
		 * This method looks at the provided io_service, and if it's
		 * not currently running, it makes sure to start a boost::thread
		 * to run it, and then adds it to the map of running threads. This
		 * is a very simple way for the code to ensure that each io_service
		 * of a TCP receiver is running without the code getting too complex.
		 */
		static bool verifyProcessing( const io_svc_ptr & aService );
		/**
		 * This static method is the target of the boost::thread that will
		 * take the shared_ptr<io_service> to use in it's run loop, and will
		 * keep calling it's run() method, with the option to add a fixed
		 * work unit to the io_service, so that the ASIO events for those
		 * sockets and devices continue to be serviced and processed. This
		 * will continue until it's forced to stop so that it's something
		 * akin to "fire and forget".
		 */
		static void process( io_svc_ptr & aService );
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
		 * We are going to allow the caller to set the send buffer size
		 * that we'll ask for from the OS. If the OS isn't set up for this,
		 * we'll get the biggest it'll allow, but that's out of our control.
		 */
		uint32_t			_xmit_buff_size;

		/**
		 * This is the channel that we will be sending on. It's
		 * set in the constructor or the init() method, and then used to make
		 * the connection and log what's happening.
		 */
		channel				_channel;
		/**
		 * The boost ASIO library handles async operations by having each
		 * socket created (bound) to an io_service and then that service's
		 * run() method handles all pending actions. We are using the boost
		 * shared_ptr to allow the first TCP receiver to create the
		 * io_service, and then when another is initialized with this
		 * existing TCP transmitter, then it will grab the shared_ptr, and
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
		tcp::socket			*_socket;
		// this is a simple sinlock that makes sure we're thread-safe
		mutable spinlock	_mutex;

		/**
		 * All transmitters need to know the specific boost::thread for a
		 * given boost::io_service - and this map is how they find that
		 * out. When a receiver is asked to "start sending", it looks
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
		 * available to be used - in a SP/MC strategy. This is what
		 * we need because MANY threads can be giving is datagrams,
		 * so it has to be MC, but there's only one thread that's
		 * recycling them - that's SP.
		 */
		static pool<datagram *, 16, dkit::sp_mc>	_pool;
};

/**
 * In order to allow this object to be a key in the hash-based containers
 * of boost, it's essential that we have the following function as it's
 * what boost expects from all it's supported classes.
 */
std::size_t hash_value( tcp_transmitter const & aValue );
}		// end of namespace io
}		// end of namespace dkit

/**
 * For debugging purposes, let's make it easy for the user to stream
 * out this value. It basically is just the value of the base data type
 * and puts it into the stream.
 */
std::ostream & operator<<( std::ostream & aStream, const dkit::io::tcp_transmitter & aValue );

#endif		// __DKIT_IO_TCP_TRANSMITTER_H
