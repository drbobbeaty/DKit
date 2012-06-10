/**
 * This is the tests for the trie
 */
//	System Headers
#include <iostream>
#include <string>

//	Third-Party Headers

//	Other Headers
#include "trie.h"
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

	dkit::trie<blob *, dkit::uint64_key>	m;
	std::cout << "trie<blob *> has been created... adding values..." << std::endl;

	uint64_t	cnt = 65535;
	uint64_t	sz = 0;
	if (!error) {
		// get the starting time
		uint64_t	goTime = dkit::util::timer::usecStamp();
		for (uint64_t i = 0; i < cnt; ++i) {
			blob	*b = new blob(i);
			m.put(b);
		}
		goTime = dkit::util::timer::usecStamp() - goTime;
		std::cout << "insertions took " << goTime << " usec ... "
				  << 1.0*goTime/cnt << " usec/ins" << std::endl;
		if ((sz = m.size()) == cnt) {
			std::cout << "Success - the trie has " << sz << " elements!" << std::endl;
		} else {
			error = true;
			std::cout << "ERROR - the trie has " << sz << " elements, and it should have " << cnt << "!" << std::endl;
		}
	}

	if (!error) {
		for (uint16_t passes = 0; passes < 5; ++passes) {
			// get the starting time
			uint64_t	goTime = dkit::util::timer::usecStamp();
			blob		*bp = NULL;
			for (uint64_t i = 0; i < cnt; ++i) {
				if (!m.get(i, bp)) {
					error = true;
					std::cout << "ERROR - failed to get key=" << i << "!" << std::endl;
					break;
				}
			}
			if (!error) {
				goTime = dkit::util::timer::usecStamp() - goTime;
				std::cout << "simple gets took " << goTime << " usec ... "
						  << 1.0*goTime/cnt << " usec/get" << std::endl;
			}
		}
	}

	if (!error) {
		counter		worker;
		// get the starting time
		uint64_t	goTime = dkit::util::timer::usecStamp();
		m.apply(worker);
		goTime = dkit::util::timer::usecStamp() - goTime;
		if (worker.getCount() == cnt) {
			std::cout << "Success - the counter worker found: " << worker.getCount() << " elements in the trie in " << goTime/1000.0 << " msec" << std::endl;
		} else {
			error = true;
			std::cout << "ERROR - the counter worker found: " << worker.getCount() << " elements in the trie, and it should have found " << cnt << std::endl;
		}
	}

	std::cout << (error ? "FAILED!" : "SUCCESS") << std::endl;
	return (error ? 1 : 0);
}
