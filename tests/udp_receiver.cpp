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

		virtual bool recv( const T anItem )
		{
			return onMessage(anItem);
		}

		bool onMessage( const datagram *dg ) {
			if (dg == NULL) {
				std::cout << "got a NULL" << std::endl;
			} else {
				std::cout << "got: " << dg->contents() << std::endl;
				_last_time = dkit::util::timer::usecStamp();
				++_cnt;
			}
			return true;
		}

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



int main(int argc, char *argv[]) {
	bool	error = false;

	MySink<datagram*>	dump;
	udp_receiver	rcvr(multicast_channel("udp://239.255.0.1:30001"));
	rcvr.addToListeners(&dump);
	rcvr.listen();
	udp_receiver	hold;
	hold.shareService(rcvr);
	hold.init();
	while (rcvr.isListening() && !dump.allDone()) {
		sleep(1);
	}
	std::cout << "shutting down due to inactivity..." << std::endl;
	rcvr.shutdown();

	std::cout << (error ? "FAILED!" : "SUCCESS") << std::endl;
	return (error ? 1 : 0);
}
