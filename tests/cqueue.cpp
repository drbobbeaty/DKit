/**
 * This is the tests for the trie
 */
//	System Headers
#include <iostream>
#include <string>

//	Third-Party Headers

//	Other Headers
#include "cqueue.h"
#include "util/timer.h"


class blob {
	public:
		blob() : _when(0) { }
		blob(uint64_t aWhen) : _when(aWhen) { }
		virtual ~blob() { }
		void setValue(uint64_t aValue) { _when = aValue; }
		uint64_t getValue() const { return _when; }
	private:
		uint64_t		_when;
};

uint64_t key_value( const blob *aValue )
{
	return (*aValue).getValue();
}

class counter : public dkit::trie<blob *, dkit::uint64_key>::functor
{
	public:
		counter() : _cnt(0) { }
		virtual ~counter() { }
		virtual bool process( volatile dkit::trie<blob *, dkit::uint64_key>::Node & aNode )
		{
			++_cnt;
			return true;
		}
		uint64_t getCount() { return _cnt; }
	private:
		uint64_t	_cnt;
};

int main(int argc, char *argv[]) {
	bool	error = false;

	/**
	 * Make a conflation queue:
	 *   - holds (blob *) instances
	 *   - max 2^17 = 128k elements
	 *   - SP/SC
	 *   - 64-bit key for conflation
	 *   - max 2^5 = 32 keys in the pool
	 */
	dkit::cqueue<blob *, 17, dkit::sp_sc, dkit::uint64_key, 5>	q;
	std::cout << "cqueue<> has been created... pushing values..." << std::endl;

	uint64_t	cnt = 65535;
	uint64_t	sz = 0;
	if (!error) {
		// get the starting time
		uint64_t	goTime = dkit::util::timer::usecStamp();
		for (uint64_t i = 0; i < cnt; ++i) {
			blob	*b = new blob(i);
			q.push(b);
		}
		goTime = dkit::util::timer::usecStamp() - goTime;
		std::cout << "pushes took " << goTime << " usec ... "
				  << 1.0*goTime/cnt << " usec/push" << std::endl;
		if ((sz = q.size()) == cnt) {
			std::cout << "Success - the cqueue has " << sz << " elements!" << std::endl;
		} else {
			error = true;
			std::cout << "ERROR - the cqueue has " << sz << " elements, and it should have " << cnt << "!" << std::endl;
		}
	}

	if (!error) {
		counter		worker;
		// get the starting time
		uint64_t	goTime = dkit::util::timer::usecStamp();
		q.apply(worker);
		goTime = dkit::util::timer::usecStamp() - goTime;
		if (worker.getCount() == cnt) {
			std::cout << "Success - the counter worker found: " << worker.getCount() << " elements in the trie in " << goTime/1000.0 << " msec" << std::endl;
		} else {
			error = true;
			std::cout << "ERROR - the counter worker found: " << worker.getCount() << " elements in the trie, and it should have found " << cnt << std::endl;
		}
	}

	if (!error) {
		// get the starting time
		uint64_t	goTime = dkit::util::timer::usecStamp();
		blob		*bp = NULL;
		for (uint64_t i = 0; i < cnt; ++i) {
			if (!q.pop(bp)) {
				error = true;
				std::cout << "ERROR - failed to pop #" << i << "!" << std::endl;
				break;
			}
			if (bp != NULL) {
				delete bp;
				bp = NULL;
			}
		}
		if (!error) {
			goTime = dkit::util::timer::usecStamp() - goTime;
			std::cout << "simple pops took " << goTime << " usec ... "
					  << 1.0*goTime/cnt << " usec/pop" << std::endl;
		}
	}

	// now let's test the conflation aspect
	if (!error) {
		// get the starting time
		uint64_t	goTime = dkit::util::timer::usecStamp();
		cnt = 10;
		sz = 10;
		for (uint64_t i = 0; i < cnt; ++i) {
			blob	*b = new blob(i);
			q.push(b);
		}
		goTime = dkit::util::timer::usecStamp() - goTime;
		std::cout << "pushes took " << goTime << " usec ... "
				  << 1.0*goTime/cnt << " usec/push" << std::endl;
		if ((sz = q.size()) == cnt) {
			std::cout << "Success - the cqueue has " << sz << " elements!" << std::endl;
		} else {
			error = true;
			std::cout << "ERROR - the cqueue has " << sz << " elements, and it should have " << cnt << "!" << std::endl;
		}

		goTime = dkit::util::timer::usecStamp();
		for (uint64_t i = 0; i < cnt; ++i) {
			blob	*b = new blob(i);
			q.push(b);
		}
		goTime = dkit::util::timer::usecStamp() - goTime;
		std::cout << "duplicate pushes took " << goTime << " usec ... "
				  << 1.0*goTime/cnt << " usec/push" << std::endl;
		if ((sz = q.size()) == cnt) {
			std::cout << "Success - the cqueue has " << sz << " elements!" << std::endl;
		} else {
			error = true;
			std::cout << "ERROR - the cqueue has " << sz << " elements, and it should have " << cnt << "!" << std::endl;
		}

		goTime = dkit::util::timer::usecStamp();
		blob		*bp = NULL;
		for (uint64_t i = 0; i < cnt; ++i) {
			if (!q.pop(bp)) {
				error = true;
				std::cout << "ERROR - failed to pop #" << i << "!" << std::endl;
				break;
			}
			if (bp != NULL) {
				delete bp;
				bp = NULL;
			}
		}
		if (!error) {
			goTime = dkit::util::timer::usecStamp() - goTime;
			std::cout << "final size=" << q.size() << " ... simple pops took "
					  << goTime << " usec ... " << 1.0*goTime/cnt
					  << " usec/pop" << std::endl;
		}
	}

	std::cout << (error ? "FAILED!" : "SUCCESS") << std::endl;
	return (error ? 1 : 0);
}
