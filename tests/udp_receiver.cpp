/**
 * This is the tests for the udp_receiver
 */
//	System Headers
#include <iostream>
#include <string>

//	Third-Party Headers

//	Other Headers
#include "io/udp_receiver.h"
#include "sink.h"
#include "adapter.h"
#include "util/timer.h"

using namespace dkit::io;

/**
 * I need to have a subclass of sink<T> that's going to handle my messages.
 * Because it's still a template class, I need to call out to a specialized
 * method. It's all still in the class, but it's required for the virtual
 * template class to actually work.
 */
template <class T> class MySink :
	public dkit::sink<T>
{
	public:
		MySink() :
			_cnt(0),
			_last_time(0)
		{ }

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
		bool onMessage( const datagram *dg )
		{
			if (dg == NULL) {
				std::cout << "got a NULL" << std::endl;
			} else {
				std::cout << "got: " << dg->contents() << std::endl;
				_last_time = dkit::util::timer::usecStamp();
				++_cnt;
			}
			return true;
		}

		/**
		 * This method will return 'true' if we've received ANY datagrams
		 * and if the last one was more than 5 sec ago. That's the timeout
		 * for "no more data is coming our way."
		 */
		bool allDone()
		{
			using namespace dkit::util;
			static uint32_t	__limit = 5 * 1000000L;
			return ((_cnt > 0) && ((timer::usecStamp() - _last_time) > __limit));
		}

	private:
		uint32_t		_cnt;
		uint64_t		_last_time;
};


/**
 * I need to have a subclass of adapter<TIN, TOUT> that's going to handle
 * my messages. Because it's still a template class, I need to call out to
 * a specialized method. It's all still in the class, but it's required for
 * the virtual template class to actually work.
 */
template <class TIN, class TOUT> class MyAdapter :
	public dkit::adapter<TIN, TOUT>
{
	public:
		MyAdapter() { }

		/**
		 * This is the main receiver method that we need to call out to
		 * a concrete method for the type we're using. It's what we have
		 * to do to really get a virtual template class working for us.
		 */
		virtual bool recv( const TIN anItem )
		{
			return dkit::adapter<TIN, TOUT>::send(convert(anItem));
		}

		/**
		 * This method is called when we get a new datagram, and because
		 * we are expecting to instantiate this template class with the
		 * type 'T' being a <datagram *>, this is the method we're expecting
		 * to get hit. It's just that simple.
		 */
		std::string convert( const datagram *dg ) {
			std::string		out = "<null>";
			if (dg != NULL) {
				std::cout << "converting: " << dg->contents() << std::endl;
				out.assign(dg->what, dg->size);
			}
			return out;
		}
};


/**
 * This is the main testing app where we'll listen on a specific URL for
 * UDP multicast data, and then process it until there's a timeout. It's
 * going to also use the "shared io_service" capabilities just to make
 * sure that the reference counting in the udp_receiver is working right.
 */
int main(int argc, char *argv[]) {
	bool	error = false;

	/**
	 * To wire up as a listener to the udp_receiver, we need to be a
	 * subclass of sink<datagram*>... so now that it's made, construct
	 * one with it's own io_service.
	 */
	MySink<datagram*>	dump;
	udp_receiver	rcvr(multicast_channel("udp://239.255.0.1:30001"));
	rcvr.addToListeners(&dump);
	rcvr.listen();
	/**
	 * At this point, make a new udp_receiver but share the io_service
	 * thread from the one we just made. This will mean that both these
	 * sockets are serviced on the same thread. Just a nice way to prove
	 * that the reference counting is working.
	 */
	udp_receiver	hold;
	hold.shareService(rcvr);
	hold.init();
	/**
	 * Make a simple adapter that takes the datagrams and makes std::string
	 * instances of the data. This is just to test the adapter, and not
	 * much else.
	 */
	MyAdapter<datagram*, std::string>	packer;
	rcvr.addToListeners(&packer);

	/**
	 * Now let's stay in this loop as long as we need to...
	 */
	while (rcvr.isListening() && !dump.allDone()) {
		sleep(1);
	}
	std::cout << "shutting down due to inactivity..." << std::endl;
	rcvr.shutdown();

	std::cout << (error ? "FAILED!" : "SUCCESS") << std::endl;
	return (error ? 1 : 0);
}
