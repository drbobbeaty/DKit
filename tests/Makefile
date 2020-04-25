#
# Makefile for DKit tests
#
# We need to have a few system-level dependencies to make things cleanly
# compile on a few different systems.
#
ifeq ($(shell uname),SunOS)
CXX = g++
CXX_DEFS = -g -O3
LIB_EXT = so
ifeq ($(shell uname -r),5.6)
OS_LIBS = -lsocket -lnsl -lposix4
else
OS_LIBS = -lsocket -lnsl -lrt
endif
SHARED = -shared
endif
ifeq ($(shell uname),Darwin)
CXX = g++
CXX_DEFS = -D_REENTRANT -DBOOST_ASIO_HAS_STD_ATOMIC -g -O3 -arch x86_64
LIB_EXT = so
OS_LIBS =
SHARED = -shared
endif
ifeq ($(shell uname),Linux)
CXX = g++
ifeq ($(BOOST_ROOT),)
CXX_DEFS = -g -O3
LIB_EXT = so
OS_LIBS = -lpthread -lrt -Wl,-rpath=../lib
else
CXX_DEFS = -g -O3 -I$(BOOST_ROOT)/include
LIB_EXT = so
OS_LIBS = -lpthread -lrt -L$(BOOST_ROOT)/lib -Wl,-rpath=../lib
endif
SHARED = -shared
endif

#
# These are the locations of certain directories in the package
#
LIB_DIR = ../lib
SRC_DIR = ../src

#
# This is the ultimate target we're making - the library with the entire
# DKit contained within it.
#
LIB_FILE = $(LIB_DIR)/libDKit.$(LIB_EXT)

#
# These are the pretty standard C++-style defines for a makefile - the
# includes, the libs, the compiler flags, etc.
#
INCLUDES = -I$(SRC_DIR)
DEFINES = $(CXX_DEFS)
CXXFLAGS = -fPIC -Wall $(INCLUDES) $(DEFINES)

LIBS = -L$(LIB_DIR) $(OS_LIBS) -lboost_thread-mt -lboost_system-mt -lstdc++ \
	   -lDKit
LDFLAGS =

#
# These are the main targets that we'll be making
#
APPS = atomic spsc_fifo mpsc_fifo linkedFIFO spmc_fifo pool sender receiver \
	   udp_receiver trie cqueue
SRCS = $(APPS:%=%.cpp)

all: $(APPS)

clean:
	rm -f $(APPS)
	rm -rf *.dSYM

tests: all
	@ echo '========= Atomic Integer Tests ========='
	@ ./atomic
	@ echo '=========   SP/SC FIFO Tests   ========='
	@ ./spsc_fifo
	@ echo '========= MP/SC CircularFIFO Tests ========='
	@ ./mpsc_fifo
	@ echo '========= SP/MC CircularFIFO Tests ========='
	@ ./spmc_fifo
	@ echo '========= LinkedFIFO Tests ========='
	@ ./linkedFIFO
	@ echo '========= Pool<std::string *> Tests ========='
	@ ./pool

depend:
	makedepend -Y -o\  -- $(INCLUDES) -- $(SRCS); rm Makefile.bak

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

atomic: atomic.cpp $(LIB_FILE)
	$(CXX) $(CXXFLAGS) atomic.cpp -o atomic $(LIBS) $(LDFLAGS)

cqueue: cqueue.cpp $(LIB_FILE)
	$(CXX) $(CXXFLAGS) cqueue.cpp -o cqueue $(LIBS) $(LDFLAGS)

linkedFIFO: linkedFIFO.cpp $(LIB_FILE)
	$(CXX) $(CXXFLAGS) linkedFIFO.cpp -o linkedFIFO $(LIBS) $(LDFLAGS)

mpsc_fifo: mpsc_fifo.cpp $(LIB_FILE)
	$(CXX) $(CXXFLAGS) mpsc_fifo.cpp -o mpsc_fifo $(LIBS) $(LDFLAGS)

pool: pool.cpp $(LIB_FILE)
	$(CXX) $(CXXFLAGS) pool.cpp -o pool $(LIBS) $(LDFLAGS)

trie: trie.cpp $(LIB_FILE)
	$(CXX) $(CXXFLAGS) trie.cpp -o trie $(LIBS) $(LDFLAGS)

sender: sender.cpp $(LIB_FILE)
	$(CXX) $(CXXFLAGS) sender.cpp -o sender $(LIBS) $(LDFLAGS)

receiver: receiver.cpp $(LIB_FILE)
	$(CXX) $(CXXFLAGS) receiver.cpp -o receiver $(LIBS) $(LDFLAGS)

spsc_fifo: spsc_fifo.cpp $(LIB_FILE)
	$(CXX) $(CXXFLAGS) spsc_fifo.cpp -o spsc_fifo $(LIBS) $(LDFLAGS)

spmc_fifo: spmc_fifo.cpp $(LIB_FILE)
	$(CXX) $(CXXFLAGS) spmc_fifo.cpp -o spmc_fifo $(LIBS) $(LDFLAGS)

udp_receiver: udp_receiver.cpp $(LIB_FILE)
	$(CXX) $(CXXFLAGS) udp_receiver.cpp -o udp_receiver $(LIBS) $(LDFLAGS)

# DO NOT DELETE

atomic : ../src/atomic.h ../src/abool.h ../src/aint8.h ../src/aint16.h
atomic : ../src/aint32.h ../src/aint64.h
spsc_fifo : ../src/spsc/CircularFIFO.h ../src/FIFO.h ../src/util/timer.h
mpsc_fifo : ../src/mpsc/CircularFIFO.h ../src/FIFO.h ../src/util/timer.h
mpsc_fifo : hammer.h drain.h
linkedFIFO : ../src/mpsc/LinkedFIFO.h ../src/FIFO.h ../src/spmc/LinkedFIFO.h
linkedFIFO : ../src/util/timer.h hammer.h drain.h
spmc_fifo : ../src/spmc/CircularFIFO.h ../src/FIFO.h ../src/util/timer.h
spmc_fifo : hammer.h drain.h
pool : ../src/pool.h ../src/FIFO.h ../src/spsc/CircularFIFO.h
pool : ../src/mpsc/CircularFIFO.h ../src/spmc/CircularFIFO.h
pool : ../src/util/timer.h
udp_receiver : ../src/io/udp_receiver.h ../src/source.h ../src/abool.h
udp_receiver : ../src/sink.h ../src/io/datagram.h ../src/util/timer.h
udp_receiver : ../src/io/multicast_channel.h ../src/aint32.h ../src/pool.h
udp_receiver : ../src/FIFO.h ../src/spsc/CircularFIFO.h
udp_receiver : ../src/mpsc/CircularFIFO.h ../src/spmc/CircularFIFO.h
udp_receiver : ../src/io/udp_transmitter.h ../src/adapter.h
trie : ../src/trie.h ../src/abool.h ../src/util/timer.h
cqueue : ../src/cqueue.h ../src/FIFO.h ../src/spsc/CircularFIFO.h
cqueue : ../src/mpsc/CircularFIFO.h ../src/spmc/CircularFIFO.h ../src/trie.h
cqueue : ../src/abool.h ../src/pool.h ../src/util/timer.h
