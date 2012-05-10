/**
 * This is the tests for the SPMC CircularFIFO
 */
//	System Headers
#include <iostream>
#include <string>

//	Third-Party Headers

//	Other Headers
#include "spmc/CircularFIFO.h"
#include "util/timer.h"
#include "hammer.h"
#include "drain.h"

int main(int argc, char *argv[]) {
	bool	error = false;

	// make a circular FIFO of 1024 int32_t values - max
	dkit::spmc::CircularFIFO<int32_t, 10>	q;
	// put 500 values on the queue - and check the size
	if (!error) {
		std::cout << "=== Testing speed and correctness of CircularFIFO ===" << std::endl;

		// get the starting time
		uint64_t	goTime = dkit::util::timer::usecStamp();

		// do this often enough to rotate through the size of the values
		int32_t		trips = 100000;
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
			// now make sure we can't pop() anything
			if (!error) {
				int32_t		v = 0;
				if (!q.pop(v)) {
					if (cycle == 0) {
						std::cout << "Passed - unable to pop from an empty queue" << std::endl;
					}
				} else {
					error = true;
					std::cout << "ERROR - popped " << v << " from an empty queue - shouldn't be possible" << std::endl;
				}
			}
		}

		// get the elapsed time
		goTime = dkit::util::timer::usecStamp() - goTime;
		std::cout << "Passed - did " << (trips * 500) << " push/pop pairs in " << (goTime/1000.0) << "ms = " << ((goTime * 1000.0)/(trips * 500.0)) << "ns/op" << std::endl;
	}

	// check on how the crash recovery works
	if (!error) {
		// make sure it's all cleared out for this test
		q.clear();

		std::cout << "=== Testing crash surviveability CircularFIFO ===" << std::endl;

		// push values starting at 0 up until we can't push any more
		int32_t		v = 0;
		int32_t		lim = 0;
		while (q.push(lim)) {
			++lim;
		}
		std::cout << "Passed - Failed on pushing " << lim << std::endl;
		for (int32_t i = 0; i < lim; ++i) {
			if (!q.pop(v) || (v != i)) {
				error = true;
				std::cout << "ERROR - could not pop the value " << i << std::endl;
				break;
			}
		}
		if (!error) {
			std::cout << "Passed - after crash, still able to recover all values" << std::endl;
		}
	}

	/**
	 * Make a Hammer and a set of Drains and test threading
	 */
	if (!error) {
		Hammer	src(0, &q, 1000);
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
		if (total == 1000) {
			std::cout << "Passed - popped " << total << " integers (" << cnt[0] << "+" << cnt[1] << "+" << cnt[2] << "+" << cnt[3] << "), with four drain threads" << std::endl;
		} else {
			std::cout << "ERROR - popped " << total << " integers (" << cnt[0] << "+" << cnt[1] << "+" << cnt[2] << "+" << cnt[3] << "), with four drain threads - but should have popped 1000" << std::endl;
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
