/**
 * This is the tests for the atomic integers and boolean values
 */
//	System Headers
#include <iostream>
#include <string>

//	Third-Party Headers

//	Other Headers
#include "atomic.h"

int main(int argc, char *argv[]) {
	bool	error = false;

	/**
	 * Check the atomic bool - setting values, flipping values, etc.
	 */
	abool	b = true;
	if (!error) {
		if (!b) {
			error = true;
			std::cout << "ERROR - the abool could not be assigned the initial value 'true'!" << std::endl;
		} else {
			std::cout << "Passed - the abool can be assigned an initial value" << std::endl;
		}
	}
	if (!error) {
		b = false;
		if (b) {
			error = true;
			std::cout << "ERROR - the abool could not be assigned the value 'false'!" << std::endl;
		} else {
			std::cout << "Passed - the abool can be assigned a value" << std::endl;
		}
	}
	if (!error) {
		b = true;
		if (++b) {
			error = true;
			std::cout << "ERROR - the abool could not be pre-incremented!" << std::endl;
		} else {
			std::cout << "Passed - the abool can be pre-incremented" << std::endl;
		}
	}
	if (!error) {
		if (b++) {
			error = true;
			std::cout << "ERROR - the abool could not be post-incremented!" << std::endl;
		} else {
			std::cout << "Passed - the abool can be post-incremented" << std::endl;
		}
		if (!b) {
			error = true;
			std::cout << "ERROR - the abool is not correct post-incremented!" << std::endl;
		} else {
			std::cout << "Passed - the abool is corrent post-incremented" << std::endl;
		}
	}
	if (!error) {
		if (--b) {
			error = true;
			std::cout << "ERROR - the abool could not be pre-decremented!" << std::endl;
		} else {
			std::cout << "Passed - the abool can be pre-decremented" << std::endl;
		}
	}
	if (!error) {
		if (b--) {
			error = true;
			std::cout << "ERROR - the abool could not be post-decremented!" << std::endl;
		} else {
			std::cout << "Passed - the abool can be post-decremented" << std::endl;
		}
		if (!b) {
			error = true;
			std::cout << "ERROR - the abool is not correct post-decremented!" << std::endl;
		} else {
			std::cout << "Passed - the abool is correct post-decremented" << std::endl;
		}
	}
	if (!error) {
		b = true;
		b += 3;
		if (b) {
			error = true;
			std::cout << "ERROR - the abool could not be incremented by 3 (to false)!" << std::endl;
		} else {
			std::cout << "Passed - the abool can be incremented by 3 (to " << b << ")" << std::endl;
		}
	}

	/**
	 * Check the atomic int32_t - setting values, changing values, etc.
	 */
	aint32_t	i;
	if (!error) {
		if (i != 0) {
			error = true;
			std::cout << "ERROR - the aint32_t does not have the correct default value (0)!" << std::endl;
		} else {
			std::cout << "Passed - the aint32_t has the correct default value (0)" << std::endl;
		}
	}
	if (!error) {
		i = 10;
		if (i != 10) {
			error = true;
			std::cout << "ERROR - the aint32_t could not be assigned the value 10!" << std::endl;
		} else {
			std::cout << "Passed - the aint32_t can be assigned a value (" << i << ")" << std::endl;
		}
	}
	if (!error) {
		if (++i != 11) {
			error = true;
			std::cout << "ERROR - the aint32_t could not be pre-incremented!" << std::endl;
		} else {
			std::cout << "Passed - the aint32_t can be pre-incremented" << std::endl;
		}
	}
	if (!error) {
		if (i++ != 11) {
			error = true;
			std::cout << "ERROR - the aint32_t could not be post-incremented!" << std::endl;
		} else {
			std::cout << "Passed - the aint32_t can be post-incremented" << std::endl;
		}
		if (i != 12) {
			error = true;
			std::cout << "ERROR - the aint32_t is not correct post-incremented!" << std::endl;
		} else {
			std::cout << "Passed - the aint32_t is correct post-incremented" << std::endl;
		}
	}
	if (!error) {
		i = 10;
		i += 10;
		if (i != 20) {
			error = true;
			std::cout << "ERROR - the aint32_t can not be incremented!" << std::endl;
		} else {
			std::cout << "Passed - the aint32_t can be incremented" << std::endl;
		}

		aint32_t	j = 20;
		if (i != j) {
			error = true;
			std::cout << "ERROR - the aint32_t does not properly '!=' itself!" << std::endl;
		} else {
			std::cout << "Passed - the aint32_t properly handles '!='" << std::endl;
		}
	}

	std::cout << (error ? "FAILED!" : "SUCCESS") << std::endl;
	return (error ? 1 : 0);
}
