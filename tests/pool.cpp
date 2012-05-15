/**
 * This is the tests for the pool
 */
//	System Headers
#include <iostream>
#include <string>

//	Third-Party Headers

//	Other Headers
#include "pool.h"
#include "util/timer.h"

int main(int argc, char *argv[]) {
	bool	error = false;

	// make a pool of 2^5 (32) std::string pointers
	dkit::pool<std::string *, 5, dkit::sp_sc>	pool;
	// create an array to hold these guys while I'm "using" them
	std::string		*inUse[50];

	// grab a bunch and make sure they are real
	if (!error) {
		std::cout << "=== Getting 50 std::string Pointers ===" << std::endl;
		char	buff[80];
		for (uint8_t i = 0; i < 50; ++i) {
			if ((inUse[i] = pool.next()) == NULL) {
				error = true;
				std::cout << "ERROR - could not get std::string #" << i << std::endl;
				break;
			}
			// fill it in with some data
			snprintf(buff, 79, "string #%d", (i+1));
			inUse[i]->assign(buff);
		}
		// log if we are OK to here
		if (!error) {
			std::cout << "Passed - created 50 new std::strings" << std::endl;
		}
	}

	// make sure they are all there
	if (!error) {
		std::cout << "=== Verifying 50 std::string Pointers ===" << std::endl;
		char	buff[80];
		for (uint8_t i = 0; i < 50; ++i) {
			if (inUse[i] == NULL) {
				error = true;
				std::cout << "ERROR - could not verify std::string #" << i << std::endl;
				break;
			}
			// check what it's loaded with
			snprintf(buff, 79, "string #%d", (i+1));
			if (*inUse[i] != buff) {
				error = true;
				std::cout << "ERROR - std::string #" << i << " contained: '" << (*inUse[i]) << "'!" << std::endl;
				break;
			}
		}
		// log if we are OK to here
		if (!error) {
			std::cout << "Passed - verified the 50 std::strings" << std::endl;
		}
	}

	// check the size of the pool
	if (!error) {
		if (pool.size() == 0) {
			std::cout << "Passed - the pool is still empty" << std::endl;
		} else {
			error = true;
			std::cout << "ERROR - the pool is NOT empty! size=" << pool.size() << std::endl;
		}
	}

	// now start to recycle these back to the pool
	if (!error) {
		std::cout << "=== Recycling 50 std::string Pointers ===" << std::endl;
		for (uint8_t i = 0; i < 50; ++i) {
			// recycle a non-NULL emtry
			if (inUse[i] != NULL) {
				pool.recycle(inUse[i]);
			}
			// make sure it "tops out" at 31 items
			if (i < 31) {
				if (pool.size() != (i+1U)) {
					error = true;
					std::cout << "ERROR - count=" << (i+1) << " ...size=" << pool.size() << std::endl;
				}
			} else if (pool.size() != 31) {
				error = true;
				std::cout << "ERROR - capped count=" << (i+1) << " ...size=" << pool.size() << std::endl;
			}
		}
	}

	std::cout << (error ? "FAILED!" : "SUCCESS") << std::endl;
	return (error ? 1 : 0);
}
