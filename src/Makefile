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
SO_EXT = so
SO_FLAG = -shared
endif
ifeq ($(shell uname),Linux)
CXX = g++
CXX_DEFS = -D_REENTRANT -g -O3
CXX_FLAGS = -m64
LDD_FLAGS = -m64
SO_EXT = so
SO_FLAG = -shared
endif
ifeq ($(shell uname),Darwin)
CXX = g++
CXX_DEFS = -D_REENTRANT -g -O3
CXX_FLAGS = -arch i386 -arch x86_64
LDD_FLAGS = -arch i386 -arch x86_64
SO_EXT = dylib
SO_FLAG = -dynamiclib
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
LIBS = -L$(LIB_DIR) -lstdc++
LDFLAGS = -fPIC $(LIBS) $(LDD_FLAGS)

#
# These are all the components of DKit
#
.SUFFIXES: .h .cpp .o
OBJS = abool.o aint8.o aint16.o aint32.o aint64.o
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
