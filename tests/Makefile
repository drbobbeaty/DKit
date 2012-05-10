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
CXX_DEFS = -D_REENTRANT -g -O3 -arch i386
LIB_EXT = dylib
OS_LIBS =
SHARED = -dynamiclib
endif
ifeq ($(shell uname),Linux)
CXX = g++
CXX_DEFS = -g -O3
LIB_EXT = so
OS_LIBS =
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
INCLUDES = -I. -I$(SRC_DIR)
DEFINES = $(CXX_DEFS)
CXXFLAGS = -fPIC -Wall $(INCLUDES) $(DEFINES)

LIBS = -L$(LIB_DIR) $(OS_LIBS) -lstdc++ -lDKit
LDFLAGS =

#
# These are the main targets that we'll be making
#
APPS = atomic spsc_fifo mpsc_fifo linkedFIFO spmc_fifo
SRCS = $(APPS:%=%.cpp)

all: $(APPS)

clean:
	rm -f $(APPS)
	rm -rf *.dSYM

depend:
	makedepend -Y -- $(INCLUDES) -- $(SRCS); rm Makefile.bak

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

atomic: atomic.cpp $(LIB_FILE)
	$(CXX) $(CXXFLAGS) atomic.cpp -o atomic $(LIBS) $(LDFLAGS)

linkedFIFO: linkedFIFO.cpp $(LIB_FILE) hammer.h drain.h
	$(CXX) $(CXXFLAGS) linkedFIFO.cpp -lboost_thread -o linkedFIFO $(LIBS) $(LDFLAGS)

mpsc_fifo: mpsc_fifo.cpp $(LIB_FILE)
	$(CXX) $(CXXFLAGS) mpsc_fifo.cpp -o mpsc_fifo $(LIBS) $(LDFLAGS)

spsc_fifo: spsc_fifo.cpp $(LIB_FILE)
	$(CXX) $(CXXFLAGS) spsc_fifo.cpp -o spsc_fifo $(LIBS) $(LDFLAGS)

spmc_fifo: spmc_fifo.cpp $(LIB_FILE)
	$(CXX) $(CXXFLAGS) spmc_fifo.cpp -o spmc_fifo $(LIBS) $(LDFLAGS)

# DO NOT DELETE

atomic.o: ../src/atomic.h ../src/abool.h ../src/aint8.h ../src/aint16.h
atomic.o: ../src/aint32.h ../src/aint64.h
spsc_fifo.o: ../src/spsc/CircularFIFO.h ../src/FIFO.h ../src/util/timer.h
mpsc_fifo.o: ../src/mpsc/CircularFIFO.h ../src/FIFO.h ../src/util/timer.h
linkedFIFO.o: ../src/mpsc/LinkedFIFO.h ../src/FIFO.h ../src/spmc/LinkedFIFO.h
linkedFIFO.o: ../src/util/timer.h hammer.h drain.h
spmc_fifo.o: ../src/spmc/CircularFIFO.h ../src/FIFO.h ../src/util/timer.h
