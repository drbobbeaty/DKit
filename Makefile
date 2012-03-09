#
# Makefile for DKit a simple C++ library of fast, lightweight, lock-less or
# near lock-less datastructures.
#
#
# We need to have a few system-level dependencies to make things cleanly
# compile on a few different systems.
#
ifeq ($(shell uname),SunOS)
MAKE = make
CTAGS = ctags
endif
ifeq ($(shell uname),Linux)
MAKE = make
CTAGS = ctags
endif
ifeq ($(shell uname),Darwin)
MAKE = make
CTAGS = ectags --excmd=number --tag-relative=no --fields=+a+m+n+S -R
endif

#
# These are the locations of the directories we'll use
#
SRC_DIR = src
TESTS_DIR = tests

#
# These are the main targets that we'll be making
#
all:
	@ cd $(SRC_DIR); $(MAKE) all
	@ cd $(TESTS_DIR); $(MAKE) all

clean:
	@ cd $(SRC_DIR); $(MAKE) clean
	@ cd $(TESTS_DIR); $(MAKE) clean

depend:
	@ cd $(SRC_DIR); $(MAKE) depend
	@ cd $(TESTS_DIR); $(MAKE) depend

ctags:
	@ $(CTAGS) `pwd`/src

