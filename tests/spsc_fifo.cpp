/**
 * This is the tests for the SPSC CircularFIFO
 */
//	System Headers
#include <iostream>
#include <string>

//	Third-Party Headers

//	Other Headers
#include "spsc/CircularFIFO.h"
#include "util/timer.h"

int main(int argc, char *argv[]) {
	bool	error = false;

	// make a circular FIFO or 1024 int32_t - max
	dkit::spsc::CircularFIFO<int32_t, 10>	q;
	// put 500 values on the queue - and check the size
	if (!error) {
		std::cout << "=== Testing speed and correctness of CircularFIFO ===" << std::endl;

		// get the starting time
		uint64_t	goTime = dkit::util::timer::usecStamp();

		// do this often enough to rotate through the size of the values
		int32_t		trips = 500000;
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
	}

	std::cout << (error ? "FAILED!" : "SUCCESS") << std::endl;
	return (error ? 1 : 0);
}
