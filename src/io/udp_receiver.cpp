/**
 * udp_receiver.cpp - this file implements a simple UDP multicast receiver that
 *                    is configured with a multicast_channels and sends
 *                    downstream udp datagrams that it receives from the
 *                    socket. It's a subclass of the source<datagram *>
 *                    class, so it's going to be sending datagram pointers
 *                    to the listeners that they can use or copy, but CANNOT
 *                    keep as we are going to be returning them to the pool
 *                    to keep news and deletes to a minimum.
 */

//	System Headers

//	Third-Party Headers

//	Other Headers
#include "udp_receiver.h"

//	Forward Declarations

//	Private Constants

//	Private Datatypes

//	Private Data Constants


namespace dkit {
namespace io {

/**
 * When using boost, the namespaces can really get out of hand. These are a
 * few that make it the code a LOT easier to deal with.
 */
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost::detail;

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
thread_map		udp_receiver::_threads;
// this is a simple sinlock that makes sure we're thread-safe
spinlock		udp_receiver::_threads_mutex;
/**
 * This is the datagram pool of up to 2^16 (64k) datagrams
 * available to be used - in a SP/SC strategy. This is what
 * we need because the io_service thread is all that's going
 * to be pulling these off, and placing them back on. It's a
 * pretty simple arrangement, actually, and there is really
 * not the need for much of a pool, but we'll have one, just
 * in case.
 */
pool<datagram *, 16, dkit::sp_sc>	udp_receiver::_pool;

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
udp_receiver::udp_receiver() :
	source<datagram *>(),
	_rcv_buff_size(DEFAULT_RCV_BUFFER_SIZE),
	_channel(),
	_service(),
	_socket(NULL),
	_mutex()
{
}


/**
 * This version of the constructor takes a single multicast channel
 * and attempts to hook up to that address and port, listening for
 * datagrams on the wire. When one arrives, this class will bundle
 * it up in a datagram instance and message all registered listeners
 * so that they can process the datagram.
 */
udp_receiver::udp_receiver( const multicast_channel & aChannel, uint32_t aRcvBuffSize ) :
	source<datagram *>(),
	_rcv_buff_size(aRcvBuffSize),
	_channel(aChannel),
	_service(),
	_socket(NULL),
	_mutex()
{
}


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
udp_receiver::udp_receiver( const udp_receiver & anOther, const multicast_channel & aChannel ):
	source<datagram *>(),
	_rcv_buff_size(DEFAULT_RCV_BUFFER_SIZE),
	_channel(aChannel),
	_service(anOther._service),
	_socket(NULL),
	_mutex()
{
}


/**
 * This is the standard copy constructor and needs to be in every
 * class to make sure that we don't have too many things running
 * around.
 */
udp_receiver::udp_receiver( const udp_receiver & anOther ) :
	source<datagram *>(),
	_rcv_buff_size(DEFAULT_RCV_BUFFER_SIZE),
	_channel(),
	_service(),
	_socket(NULL),
	_mutex()
{
	// let the '=' operator do the heavy lifting...
	*this = anOther;
}


/**
 * This is the standard destructor and needs to be virtual to make
 * sure that if we subclass off this the right destructor will be
 * called.
 */
udp_receiver::~udp_receiver()
{
	// we just need to shut down this guy
	shutdown_nl();
}


/**
 * When we want to process the result of an equality we need to
 * make sure that we do this right by always having an equals
 * operator on all classes.
 */
udp_receiver & udp_receiver::operator=( const udp_receiver & anOther )
{
	/**
	 * Make sure that we don't do this to ourselves...
	 */
	if (this != & anOther) {
		// first, let the super do it's thing...
		source<datagram *>::operator=(anOther);
		// ...and now we can do ours
		shareService(anOther);
		_rcv_buff_size = anOther._rcv_buff_size;
		if (anOther.isListening()) {
			listen(anOther._channel);
		}
	}
	return *this;
}


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
const multicast_channel & udp_receiver::getMulticastChannel() const
{
	return _channel;
}


/**
 * This method returns the multicast channel as a URL in the form
 * 'udp://addr:port' so that it can be easily used in logging, etc.
 */
std::string udp_receiver::getURL() const
{
	return _channel.getURL();
}


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
void udp_receiver::shareService( const udp_receiver & anOther )
{
	shareService(anOther._service);
}


/**
 * This version of the method takes the boost shared_ptr of the
 * io_service directly in case you had a separate run loop planned
 * for a group of ASIO instances.
 */
void udp_receiver::shareService( const io_svc_ptr & aService )
{
	// make sure we're not listening at the time
	if (isListening()) {
		throw std::runtime_error("Can't change service while listening!");
	}

	// lock this guy up for the checks and the possible work
	spinlock::scoped_lock	lock(_mutex);

	/**
	 * Check and see if the socket is initialized. If it is, then
	 * we need to shut it down and make sure to re-initialize it
	 * after we are done setting the service.
	 */
	bool		initDone = false;
	if (_socket != NULL) {
		initDone = true;
		shutdown_nl();
	}

	/**
	 * OK... let's set the service, and if we had already initialized
	 * the socket, do that to make sure that we are in the same state
	 * as we were coming into this method - just with a shared io_service.
	 */
	_service = aService;
	if (initDone) {
		init_nl();
	}
}


/**
 * This method is used to initialize the UDP socket for listening
 * to the UDP multicast traffic on the provided multicast channel.
 * It will return 'true' if the socket can be opened and we are
 * ready to start the listening process.
 */
bool udp_receiver::init()
{
	// lock this guy up for the initialization
	spinlock::scoped_lock	lock(_mutex);
	// ...and then call the "no-lock" version to do the work
	return init_nl();
}


/**
 * This method is called in the run loop of the receiver to set
 * the opened and initialized socket into the proper mode for
 * listening so that we can being the async reading process.
 */
bool udp_receiver::listen( const multicast_channel & aChannel, uint32_t aRcvBuffSize )
{
	// lock this guy up for the listening set-up
	spinlock::scoped_lock	lock(_mutex);
	// ...and then call the "no-lock" version to do the work
	return listen_nl(aChannel, aRcvBuffSize);
}


/**
 * This method is called in the run loop of the receiver to set
 * the opened and initialized socket into the proper mode for
 * listening so that we can being the async reading process.
 * This version uses the previously supplied multicast channel
 * as opposed to requiring one, as it could have been supplied
 * in the constructor.
 */
bool udp_receiver::listen()
{
	// lock this guy up for the listening set-up
	spinlock::scoped_lock	lock(_mutex);
	// ...and then call the "no-lock" version to do the work
	return listen_nl(_channel, _rcv_buff_size);
}


/**
 * This method returns 'true' if the socket is open on a specific
 * UDP multicast channel, and the thread is running listening to
 * datagrams arriving at the socket and shipping them downstream.
 */
bool udp_receiver::isListening() const
{
	bool		listening = true;
	// first, see if we have a socket, and if it's open
	if (listening) {
		spinlock::scoped_lock	lock(_mutex);
		if ((_socket == NULL) || !_socket->is_open()) {
			listening = false;
		}
	}
	// now see if we have the associated service running
	if (listening) {
		if ((_service.get() == NULL) || !isRunning(_service)) {
			listening = false;
		}
	}
	return listening;
}


/**
 * This method shuts down the listening, if it's active, and then
 * closes out the socket and cleans everything up so that this
 * receiver is back at the state of having a multicast channel
 * defined, but not initialized and not listening.
 */
void udp_receiver::shutdown()
{
	// lock this guy up for the tear-down
	spinlock::scoped_lock	lock(_mutex);
	// ...and then call the "no-lock" version to do the work
	return shutdown_nl();
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
std::string udp_receiver::toString() const
{
	std::ostringstream	msg;
	msg << "[udp_receiver on:" << _channel
		<< ((_socket != NULL) ? " open" : "")
		<< (isListening() ? " listening" : "")
		<< "]";
	return msg.str();
}


/**
 * When we have a custom '==' operator, it's wise to have a hash
 * method as well. This makes it much easier to used hash-based
 * containers like boost, etc. It's a requirement that if two
 * instances are '==', then their hash() methods must be '==' as
 * well.
 */
size_t udp_receiver::hash() const
{
	// get the hash code of the super class first...
	size_t	ans = dkit::source<datagram *>::hash();
	// ...and then combine in all the hash values for our ivars
	boost::hash_combine(ans, _rcv_buff_size);
	boost::hash_combine(ans, _channel);
	boost::hash_combine(ans, hash_value(_service));
	boost::hash_combine(ans, (_socket != NULL ? 1 : 0));
	return ans;
}


/**
 * This method checks to see if two receivers are equal in their
 * contents and not their pointer values. This is how you'd likely
 * expect equality to work.
 */
bool udp_receiver::operator==( const udp_receiver & anOther ) const
{
	bool		equals = false;
	if ((this == & anOther) ||
		(source<datagram*>::operator==(anOther) &&
		 (_rcv_buff_size == anOther._rcv_buff_size) &&
		 (_channel == anOther._channel) &&
		 (_service == anOther._service) &&
		 (isListening() == anOther.isListening()))) {
		equals = true;
	}
	return equals;
}


/**
 * This method checks to see if two receivers are NOT equal in their
 * contents and not their pointer values. This is how you'd likely
 * expect equality to work.
 */
bool udp_receiver::operator!=( const udp_receiver & anOther ) const
{
	return !operator==(anOther);
}


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
bool udp_receiver::init_nl()
{
	bool		error = false;
	bool		allDone = false;

	// see if we are already listening, and if we are, we can't do this
	if (!error && !allDone) {
		// if we're listening, then there's no need to init anything
		allDone = isListening_nl();
	}

	/**
	 * The only reason to do any of this is if the socket itself
	 * is not created, and therefore not listening. If it's created,
	 * then our job here is done. If it's not, then we need to make
	 * it and get ready for the listening.
	 */
	if (!error && !allDone) {
		// if we already have a socket, and so we're all done.
		allDone = (_socket != NULL);
	}

	/**
	 * If we have no io_service, then we need to make one because
	 * we HAVE to have ONE for the creation of the socket. It's the
	 * way we're sharing the CPU load on the ASIO threads - if we share
	 * an io_service, then we share it's thread as well.
	 */
	if (!error && !allDone) {
		if (_service.get() == NULL) {
			// we need to make our own - so do that
			_service = io_svc_ptr(new io_service());
			if (_service.get() == NULL) {
				error = true;
			}
		}
	}

	/**
	 * At this point, we can create the socket with the io_service that
	 * we have access to. This can be shared or just for us, but it's
	 * here now, and it's all we need.
	 */
	if (!error && !allDone) {
		_socket = new udp::socket(*_service);
		if (_socket == NULL) {
			error = true;
		} else {
			// up the use count for this io_service as we're using it now
			incr_use_count(_service);
		}
	}

	return !error;
}


/**
 * This method is the "no-lock" version of the listen() method and
 * is the place where all the magic REALY happens. It is called
 * in the run loop of the receiver to set the opened and initialized
 * socket into the proper mode for listening so that we can being
 * the async reading process.
 */
bool udp_receiver::listen_nl( const multicast_channel & aChannel, uint32_t aRcvBuffSize )
{
	bool		error = false;

	// make sure the socket is initialized properly
	if (!error) {
		error = !init_nl();
	}

	// make sure there's something logical to do...
	if (!error) {
		error = aChannel.empty();
	}

	// if we're OK, then fire up the UDP listening socket listening
	if (!error && (_socket != NULL)) {
		using namespace boost::asio;
		try {
			boost::system::error_code	err;
			// ...open up the socket and set SO_REUSEADDR to enabled
			_socket->open(_channel.endpoint.protocol());
			_socket->set_option(udp::socket::reuse_address(true));
			// set the rcv buffer size (allow for error, but ignore)
			_socket->set_option(socket_base::receive_buffer_size(aRcvBuffSize), err);
			// ...now bind that bad boy and join the multicast group
			_socket->bind(_channel.endpoint);
			_socket->set_option(multicast::join_group(_channel.endpoint.address()));
			// save the channel we just started listening to for later
			_channel = aChannel;
			// ...and the buffer size - just in case
			_rcv_buff_size = aRcvBuffSize;
		} catch (std::exception & se) {
			error = true;
		}
	}

	// at this point, make sure the io_service thread is running
	if (!error) {
		startAsyncRead_nl();
		error = !verifyProcessing(_service);
	}

	return !error;
}


/**
 * This method is the "no-lock" version of the isListening() method
 * and returns 'true' if the socket is open on a specific
 * UDP multicast channel, and the thread is running listening to
 * datagrams arriving at the socket and shipping them downstream.
 */
bool udp_receiver::isListening_nl()
{
	bool		listening = true;
	// first, see if we have a socket, and if it's open
	if (listening) {
		if ((_socket == NULL) || !_socket->is_open()) {
			listening = false;
		}
	}
	// now see if we have the associated service running
	if (listening) {
		if (!isRunning(_service)) {
			listening = false;
		}
	}
	return listening;
}


/**
 * This method is the "no-lock" version of the shutdown() method
 * and shuts down the listening, if it's active, and then
 * closes out the socket and cleans everything up so that this
 * receiver is back at the state of having a multicast channel
 * defined, but not initialized and not listening.
 */
void udp_receiver::shutdown_nl()
{
	// now let's close down the socket itself
	boost::system::error_code	err;
	try {
		if (_socket != NULL) {
			// cancel any pending async operations and close it down
			_socket->cancel(err);
			_socket->close(err);
			// now delete it and we're almost done
			delete _socket;
			_socket = NULL;
			// drop the use count for this io_service as we're done
			decr_use_count(_service);
		}
	} catch (...) {
		// do nothing, we're shutting it down
	}

	// see if we need to terminate the io_service thread as it's done
	terminate(_service);
}


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
bool udp_receiver::startAsyncRead()
{
	// lock this guy up for the start of this read
	spinlock::scoped_lock	lock(_mutex);
	// ...and then use the no-lock version
	return startAsyncRead_nl();
}


/**
 * This method is the "no-lock" version of the startAsyncRead()
 * method and is called when you need to START the async process
 * of reading from the UDP multicast socket but you have already
 * obtained the lock on the mutex. It's going to return
 * almost immediately, as the async process is handed off to the
 * io_service thread, but it's still nice to have a single place
 * where we can start the process of a read.
 */
bool udp_receiver::startAsyncRead_nl()
{
	bool		error = false;

	// we need to get a datagram from the pool
	datagram	*dg = NULL;
	if (!error && ((dg = _pool.next()) == NULL)) {
		error = true;
	}

	// make sure that we have a socket to deal with
	if (!error && (_socket == NULL)) {
		error = true;
	}

	// finally, start that reading
	if (!error) {
		_socket->async_receive(buffer(dg->what, dg->capacity),
							   boost::bind(&udp_receiver::handleAsyncRead,
										   this, dg, placeholders::error,
										   placeholders::bytes_transferred)
							  );
	}

	return !error;
}


/**
 * This method will be called by the io_service thread when a complete
 * UDP datagram is read at the socket and it's time to pass it back to
 * us for processing. This method takes the datagram that now has
 * the data in it's posession, and a possible error code and the
 * number of bytes placed in that datagram. This is all we need to
 * finish packaging up the datagram and sending it on it's way.
 */
void udp_receiver::handleAsyncRead( datagram *aPayload,
									const boost::system::error_code & anError,
									size_t aBytesReceived )
{
	bool		error = false;
	bool		bail = false;

	// make sure to handle errors
	if (anError) {
		if (anError == error::operation_aborted) {
			bail = true;
		} else if (anError == error::network_down) {
			bail = true;
		} else {
			error = true;
		}
	}

	// tag the datagram with the size and the current time
	if (!error && !bail && (aPayload != NULL)) {
		aPayload->markTimeAndSize(aBytesReceived);
		// now send it to all the registered listeners
		if (aBytesReceived > 0) {
			send(aPayload);
		}
		// finally, recycle it back to the pool
		_pool.recycle(aPayload);
	}

	// if not bailing out, start another read for the next datagram
	if (!bail) {
		startAsyncRead();
	}
}


/**
 * This method looks at the provided io_service, and if it's
 * not currently running, it makes sure to start a boost::thread
 * to run it, and then adds it to the map of running threads. This
 * is a very simple way for the code to ensure that each io_service
 * of a UDP receiver is running without the code getting too complex.
 */
bool udp_receiver::verifyProcessing( const io_svc_ptr & aService, bool addWork )
{
	bool		error = false;
	bool		allDone = false;

	// first, see if it's already running
	if (!error && !allDone) {
		allDone = isRunning(aService);
	}

	// if we need to start a thread, then let's do that now
	if (!error && !allDone) {
		spinlock::scoped_lock	lock(_threads_mutex);
		_threads[aService].thread = boost::thread(&udp_receiver::process, aService, addWork);
	}

	return !error;
}


/**
 * This static method is the target of the boost::thread that will
 * take the shared_ptr<io_service> to use in it's run loop, and will
 * keep calling it's run() method, with the option to add a fixed
 * work unit to the io_service, so that the ASIO events for those
 * sockets and devices continue to be serviced and processed. This
 * will continue until it's forced to stop so that it's something
 * akin to "fire and forget".
 */
void udp_receiver::process( io_svc_ptr & aService, bool addWork )
{
	/**
	 * This method is the basic run loop for the io_service thread.
	 * This will hit the io_service's run() method over and over
	 * until such time as the io_service is no longer used by any
	 * sockets or receivers. At that point, we will stop trying to
	 * run() it, and just let it die.
	 */
	while ((aService.get() != NULL) && use_count(aService)) {
		try {
			aService->reset();
			aService->run();
			usleep(250000);
		} catch (std::exception & se) {
			// just keep going...
		}
	}
}


/**
 * This method returns 'true' if the provided io_service is currently
 * being run by a boost::thread, and we have verified that things
 * are OK with it. If not, this will return 'false'.
 */
bool udp_receiver::isRunning( const io_svc_ptr & aService )
{
	bool		running = false;
	// lock up the map for this check
	spinlock::scoped_lock	lock(_threads_mutex);
	thread_map::iterator	it = _threads.find(aService);
	if (it != _threads.end()) {
		// if the thread is join-able, then it's executing
		running = (it->second.use_count > 0) && it->second.thread.joinable();
	}
	return running;
}


/**
 * This method will terminate the running boost::thread that's
 * currently processing the provided io_service. This is only
 * done if we are SURE that there are no other receivers using
 * this io_service, and if that's the case, we can freely shut
 * this down as it's no longer needed.
 */
void udp_receiver::terminate( const io_svc_ptr & aService )
{
	// lock up the map for this check
	spinlock::scoped_lock	lock(_threads_mutex);
	thread_map::iterator	it = _threads.find(aService);
	if (it != _threads.end()) {
		// if the thread is join-able, then it's executing
		if ((it->second.use_count < 1) && it->second.thread.joinable()) {
			it->first->stop();
			it->second.thread.detach();
			// at this point, the thread is done, drop it from the map
			_threads.erase(it);
		}
	}
}


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
bool udp_receiver::incr_use_count( const io_svc_ptr & aService )
{
	// lock up the map for this operation
	spinlock::scoped_lock	lock(_threads_mutex);
	// now pre-increment the value and return if it's non-zero
	return (++_threads[aService].use_count);
}


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
bool udp_receiver::decr_use_count( const io_svc_ptr & aService )
{
	// lock up the map for this operation
	spinlock::scoped_lock	lock(_threads_mutex);
	// now pre-decrement the value and return if it's non-zero
	return (--_threads[aService].use_count);
}


/**
 * This method gets the current use count (in number of sockets
 * currently using the io_service) of the provided io_service.
 * This is very useful for knowing whether or not we need to
 * continue servicing this io_service thread.
 */
uint32_t udp_receiver::use_count( const io_svc_ptr & aService )
{
	// lock up the map for this operation
	spinlock::scoped_lock	lock(_threads_mutex);
	// just map the value and return it
	return (uint32_t)_threads[aService].use_count;
}


/**
 * In order to allow this object to be a key in the hash-based containers
 * of boost, it's essential that we have the following function as it's
 * what boost expects from all it's supported classes.
 */
std::size_t hash_value( udp_receiver const & aValue )
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
std::ostream & operator<<( std::ostream & aStream, const dkit::io::udp_receiver & aValue )
{
	aStream << aValue.toString();
	return aStream;
}
