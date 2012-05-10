/**
 * This is the tests for the MPSC & SPMC LinkedFIFO
 */
//	System Headers
#include <iostream>
#include <string>

//	Third-Party Headers
#include <boost/thread.hpp>

//	Other Headers
#include "mpsc/LinkedFIFO.h"
#include "spmc/LinkedFIFO.h"
#include "util/timer.h"
#include "hammer.h"
#include "drain.h"


/**
 * This is the main entry point for the testing code
 */
int main(int argc, char *argv[]) {
	bool	error = false;

	/**
	 * First, let's check the MP/SC Linked FIFO...
	 */
	if (!error) {
		// make a linked FIFO of int32_t values

		dkit::mpsc::LinkedFIFO<int32_t>	q;
		// put 500 values on the queue - and check the size
		std::cout << "=== Testing speed and correctness of MP/SC LinkedFIFO ===" << std::endl;

		// get the starting time
		uint64_t	goTime = dkit::util::timer::usecStamp();

		// do this often enough to rotate through the size of the values
		int32_t		trips = 25000;
		for (int32_t cycle = 0; cycle < trips; ++cycle) {
			// put 500 values on the queue - and check the size
			for (int32_t i = 0; i < 500; ++i) {
				if (!q.push(i)) {
					error = true;
					std::cout << "ERROR - could not push the value " << i << std::endl;
					break;
				}
			}
			// now check the size
			if (!error) {
				if (q.size() != 500) {
					error = true;
					std::cout << "ERROR - pushed 500 integers, but size() reports only " << q.size() << std::endl;
				} else {
					if (cycle == 0) {
						std::cout << "Passed - pushed on 500 integers" << std::endl;
					}
				}
			}
			// pop off 500 integers and it should be empty
			if (!error) {
				int32_t		v = 0;
				for (int32_t i = 0; i < 500; ++i) {
					if (!q.pop(v) || (v != i)) {
						error = true;
						std::cout << "ERROR - could not pop the value " << i << std::endl;
						break;
					}
				}
			}
			// now check the size
			if (!error) {
				if (!q.empty()) {
					error = true;
					std::cout << "ERROR - popped 500 integers, but size() reports " << q.size() << std::endl;
				} else {
					if (cycle == 0) {
						std::cout << "Passed - popped all 500 integers" << std::endl;
					}
				}
			}
		}

		// get the elapsed time
		goTime = dkit::util::timer::usecStamp() - goTime;
		std::cout << "Passed - did " << (trips * 500) << " push/pop pairs in " << (goTime/1000.0) << "ms = " << ((goTime * 1000.0)/(trips * 500.0)) << "ns/op" << std::endl;

		/**
		 * Make a set of Hammers and a Drain and test threading
		 */
		Hammer	*src[] = { NULL, NULL, NULL, NULL };
		for (uint32_t i = 0; i < 4; ++i) {
			if ((src[i] = new Hammer(i, &q, 5000)) == NULL) {
				std::cout << "PROBLEM - unable to make Hammer #" << i << "!" << std::endl;
				break;
			}
		}
		Drain	dest(0, &q);
		// now start the drain then all the hammers
		dest.start();
		for (uint32_t i = 0; i < 4; ++i) {
			if (src[i] != NULL) {
				src[i]->start();
			}
		}
		// now let's wait for all the hammers to be done
		bool	allSent = false;
		while (!allSent) {
			// assume done, but check for the first failure
			allSent = true;
			// now let's check all the hammers to see if they are done
			for (uint32_t i = 0; i < 4; ++i) {
				if (src[i] != NULL) {
					if (!src[i]->isDone()) {
						allSent = false;
						break;
					}
				}
			}
			// see if we need to wait a bit to try again
			if (!allSent) {
				usleep(250000);
			}
		}
		// now tell the drain to stop when the queue is empty
		dest.stopOnEmpty();
		while (!dest.isDone()) {
			usleep(250000);
		}
		// now let's see what we have
		uint32_t	cnt = dest.getCount();
		if (cnt == 20000) {
			std::cout << "Passed - popped " << cnt << " integers, from four hammer threads" << std::endl;
		} else {
			std::cout << "ERROR - popped " << cnt << " integers, but should have popped " << 20000 << std::endl;
		}
		// finally, clean things up
		for (uint32_t i = 0; i < 4; ++i) {
			if (src[i] != NULL) {
				delete src[i];
				src[i] = NULL;
			}
		}
	}

	/**
	 * Next, let's check the SP/MC Linked FIFO...
	 */
	if (!error) {
		// make a linked FIFO of int32_t values
		dkit::spmc::LinkedFIFO<int32_t>	q;
		// put 500 values on the queue - and check the size
		std::cout << "=== Testing speed and correctness of SP/MC LinkedFIFO ===" << std::endl;

		// get the starting time
		uint64_t	goTime = dkit::util::timer::usecStamp();

		// do this often enough to rotate through the size of the values
		int32_t		trips = 25000;
		for (int32_t cycle = 0; cycle < trips; ++cycle) {
			// put 500 values on the queue - and check the size
			for (int32_t i = 0; i < 500; ++i) {
				if (!q.push(i)) {
					error = true;
					std::cout << "ERROR - could not push the value " << i << std::endl;
					break;
				}
			}
			// now check the size
			if (!error) {
				if (q.size() != 500) {
					error = true;
					std::cout << "ERROR - pushed 500 integers, but size() reports only " << q.size() << std::endl;
				} else {
					if (cycle == 0) {
						std::cout << "Passed - pushed on 500 integers" << std::endl;
					}
				}
			}
			// pop off 500 integers and it should be empty
			if (!error) {
				int32_t		v = 0;
				for (int32_t i = 0; i < 500; ++i) {
					if (!q.pop(v) || (v != i)) {
						error = true;
						std::cout << "ERROR - could not pop the value " << i << std::endl;
						break;
					}
				}
			}
			// now check the size
			if (!error) {
				if (!q.empty()) {
					error = true;
					std::cout << "ERROR - popped 500 integers, but size() reports " << q.size() << std::endl;
				} else {
					if (cycle == 0) {
						std::cout << "Passed - popped all 500 integers" << std::endl;
					}
				}
			}
		}

		// get the elapsed time
		goTime = dkit::util::timer::usecStamp() - goTime;
		std::cout << "Passed - did " << (trips * 500) << " push/pop pairs in " << (goTime/1000.0) << "ms = " << ((goTime * 1000.0)/(trips * 500.0)) << "ns/op" << std::endl;

		/**
		 * Make a Hammer and a set of Drains and test threading
		 */
		Hammer	src(0, &q, 20000);
		Drain	*dest[] = { NULL, NULL, NULL, NULL };
		for (uint32_t i = 0; i < 4; ++i) {
			if ((dest[i] = new Drain(i, &q)) == NULL) {
				std::cout << "PROBLEM - unable to make Drain #" << i << "!" << std::endl;
				break;
			}
		}
		// now start the drains then the hammer
		for (uint32_t i = 0; i < 4; ++i) {
			if (dest[i] != NULL) {
				dest[i]->start();
			}
		}
		src.start();
		// now let's wait for the hammer to be done
		while (!src.isDone()) {
			usleep(250000);
		}
		// now tell the drains to stop when the queue is empty
		for (uint32_t i = 0; i < 4; ++i) {
			if (dest[i] != NULL) {
				dest[i]->stopOnEmpty();
			}
		}
		// wait for all the drains to be done
		bool		allDone = false;
		uint32_t	cnt[] = { 0, 0, 0, 0 };
		uint32_t	total = 0;
		while (!allDone) {
			// assume done, but check for the first failure
			allDone = true;
			// now let's check all the drains to see if they are done
			for (uint32_t i = 0; i < 4; ++i) {
				if (dest[i] != NULL) {
					if (!dest[i]->isDone()) {
						allDone = false;
						break;
					}
					// tally up the counts
					cnt[i] = dest[i]->getCount();
					total += cnt[i];
				}
			}
			// see if we need to wait a bit to try again
			if (!allDone) {
				usleep(250000);
			}
		}
		// now let's see what we have
		if (total == 20000) {
			std::cout << "Passed - popped " << total << " integers (" << cnt[0] << "+" << cnt[1] << "+" << cnt[2] << "+" << cnt[3] << "), with four drain threads" << std::endl;
		} else {
			std::cout << "Error - popped " << total << " integers (" << cnt[0] << "+" << cnt[1] << "+" << cnt[2] << "+" << cnt[3] << "), with four drain threads - but should have popped 20000" << std::endl;
		}
		// finally, clean things up
		for (uint32_t i = 0; i < 4; ++i) {
			if (dest[i] != NULL) {
				delete dest[i];
				dest[i] = NULL;
			}
		}
	}

	std::cout << (error ? "FAILED!" : "SUCCESS") << std::endl;
	return (error ? 1 : 0);
}
