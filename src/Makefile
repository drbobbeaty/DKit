#
# Makefile for DKit src directory
#
#
# We need to have a few system-level dependencies to make things cleanly
# compile on a few different systems.
#
ifeq ($(shell uname),SunOS)
CXX = g++
CXX_DEFS = -D_REENTRANT -g -O3
CXX_FLAGS =
LDD_FLAGS =
ifeq ($(shell uname -r),5.6)
OS_LIBS = -lsocket -lnsl -lposix4
else
OS_LIBS = -lsocket -lnsl -lrt
endif
SO_EXT = so
SO_FLAG = -shared
endif
ifeq ($(shell uname),Linux)
CXX = g++
ifeq ($(BOOST_ROOT),)
CXX_DEFS = -D_REENTRANT -g -O3
CXX_FLAGS = -m64
LDD_FLAGS = -m64
else
CXX_DEFS = -D_REENTRANT -g -O3 -I$(BOOST_ROOT)/include
CXX_FLAGS = -m64
LDD_FLAGS = -m64 -L$(BOOST_ROOT)/lib
endif
OS_LIBS = -lpthread -lrt
SO_EXT = so
SO_FLAG = -shared
endif
ifeq ($(shell uname),Darwin)
CXX = g++
CXX_DEFS = -D_REENTRANT -DBOOST_ASIO_HAS_STD_ATOMIC -g -O3 -Wunused-local-typedef
CXX_FLAGS = -arch x86_64
LDD_FLAGS = -arch x86_64
OS_LIBS =
SO_EXT = so
SO_FLAG = -shared
endif

#
# These are the locations of the directories we'll use
#
SRC_DIR = .
LIB_DIR = ../lib
BIN_DIR = ../bin

#
# This is the ultimate target we're making - the library with the entire
# DKit contained within it.
#
LIB_FILE = $(LIB_DIR)/libDKit.$(SO_EXT)

#
# These are the pretty standard C++-style defines for a makefile - the
# includes, the libs, the compiler flags, etc.
#
INCLUDES = -I.
DEFINES = $(CXX_DEFS)
CXXFLAGS = -fPIC -Wall $(INCLUDES) $(DEFINES)
LIBS = -L$(LIB_DIR) $(OS_LIBS) -lboost_thread-mt -lboost_system-mt -lstdc++
LDFLAGS = -fPIC $(LIBS) $(LDD_FLAGS)

#
# These are all the components of DKit
#
.SUFFIXES: .h .cpp .o
OBJS = abool.o aint8.o aint16.o aint32.o aint64.o \
	   io/datagram.o io/multicast_channel.o io/channel.o \
	   io/tcp_receiver.o io/tcp_transmitter.o \
	   io/udp_receiver.o io/udp_transmitter.o
SRCS = $(OBJS:%.o=%.cpp)

#
# These are the main targets that we'll be making
#
all: $(LIB_FILE)

clean:
	rm -f $(OBJS) $(LIB_FILE)

depend:
	makedepend -Y -o.o -- $(INCLUDES) -- $(SRCS) ; rm Makefile.bak

.cpp.o:
	$(CXX) $(CXX_FLAGS) -c $(CXXFLAGS) $(DEBUG) $< -o $@

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(LIB_FILE): $(LIB_DIR) $(OBJS)
	$(CXX) $(SO_FLAG) $(OBJS) -o $(LIB_FILE) $(LDFLAGS)

# DO NOT DELETE

abool.o: abool.h aint8.h aint16.h aint32.h aint64.h
aint8.o: abool.h aint8.h aint16.h aint32.h aint64.h
aint16.o: abool.h aint8.h aint16.h aint32.h aint64.h
aint32.o: abool.h aint8.h aint16.h aint32.h aint64.h
aint64.o: abool.h aint8.h aint16.h aint32.h aint64.h
io/datagram.o: io/datagram.h util/timer.h
io/multicast_channel.o: io/multicast_channel.h abool.h
io/channel.o: io/channel.h abool.h
io/tcp_receiver.o: io/tcp_receiver.h source.h abool.h sink.h io/datagram.h
io/tcp_receiver.o: util/timer.h io/channel.h aint32.h pool.h FIFO.h
io/tcp_receiver.o: spsc/CircularFIFO.h mpsc/CircularFIFO.h
io/tcp_receiver.o: spmc/CircularFIFO.h
io/tcp_transmitter.o: io/tcp_transmitter.h sink.h abool.h source.h
io/tcp_transmitter.o: io/datagram.h util/timer.h io/channel.h
io/tcp_transmitter.o: pool.h FIFO.h spsc/CircularFIFO.h mpsc/CircularFIFO.h
io/tcp_transmitter.o: spmc/CircularFIFO.h aint32.h
io/udp_receiver.o: io/udp_receiver.h source.h abool.h sink.h io/datagram.h
io/udp_receiver.o: util/timer.h io/multicast_channel.h aint32.h pool.h FIFO.h
io/udp_receiver.o: spsc/CircularFIFO.h mpsc/CircularFIFO.h
io/udp_receiver.o: spmc/CircularFIFO.h
io/udp_transmitter.o: io/udp_transmitter.h sink.h abool.h source.h
io/udp_transmitter.o: io/datagram.h util/timer.h io/multicast_channel.h
io/udp_transmitter.o: pool.h FIFO.h spsc/CircularFIFO.h mpsc/CircularFIFO.h
io/udp_transmitter.o: spmc/CircularFIFO.h aint32.h
