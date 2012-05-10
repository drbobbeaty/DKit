DKit
====

Introduction
------------

In all the work we've done over the years, nothing has been quite as liberating
as thread-safe containers. They are an amazing experience for a developer
because they allow the multi-threaded programmer to not have to worry about
locks, lock contention, deadlocks, dirty reads, and a whole host of other
issues that for years have made writing multi-threaded code difficult. These
containers and objects free the user from having to worry about locking
because they use the _compare-and-swap_ (CAS) operations in GCC to ensure
that the data accessed is done properly, and securely.

Library Dependencies
--------------------

The focus of this library is **not** to be a general threading library. Nor
is it to attempt to write code that's already written. As such, we're going
to be using [boost](www.boost.org) for several components. This will include
things only used in the test applications - for example, threading code to
test the multi-threaded queues. Since [boost](www.boost.org) is available on
virtually all platforms, and even included in the next C++ standard, this
should not be a significant burden to the user.

There are even instructions to build it on Mac OS X, if you don't wish to use
the [Homebrew](http://mxcl.github.com/homebrew/) version that's freely
available.

Base Container Template Classes
-------------------------------

In order to make the conversion of the code as simple as possible, we have
made a base class for each type of container in this library. These base
classes are most easily used as instance variables, or method arguments, and
then the specific type of container can be instantiated and used as if it
were just the base class.

At the current time, we have the following base classes:

*	`FIFO` a simple first-in, first-out queue

Atomic Integers
===============

The first element of DKit is the atomic integers. The sized integers in
`stdint.h` are very useful for mixed-mode code, and yet there's nothing that
allow simple atomic access to these values. So the library has the following
classes defined in `atomic.h` :

*	`abool` a simple atomic boolean
*	`auint8_t` a simple atomic unsigned 8-bit integer
*	`aint8_t` a simple atomic signed 8-bit integer
*	`auint16_t` a simple atomic unsigned 16-bit integer
*	`aint16_t` a simple atomic signed 16-bit integer
*	`auint32_t` a simple atomic unsigned 32-bit integer
*	`aint32_t` a simple atomic signed 32-bit integer
*	`auint64_t` a simple atomic unsigned 64-bit integer
*	`aint64_t` a simple atomic signed 64-bit integer

These all are simply an `a` on the front of the types defined in `stdint.h`,
and all have atomic access and updating. They all also have casting operators
that make it very easy to get the values out, when it's necessary.

Single-Producer, Single-Consumer Containers
===========================================

Separated into a different namespace, the single-producer, single-consumer,
containers have taken advantage of the fact that they are only _filled_ by
one __and only one__ thread, and that they are _emptied_ by one __and only
one__ thread. This means that their safe use in the code is the responsibility
of the developer as they _must_ be sure of the threads that are using these
containers. However, when those conditions are met, these queues and other
containers are exceptionally efficient, and perform their task very fast.

dkit::spsc::CircularFIFO
------------------------

The simplest SPSC (single-producer, single-consumer) queue is a simple circular
FIFO (first-in, first-out) queue. The implementation of this is really not all
that different from any simple circular FIFO queue, with a few important
exceptions. These exceptions, and the careful use by the user of this class
make this a very nice candidate for simple pools.

One such excample, might be a `std::string` pool where a single thread is
responsible for getting data from some source, placing it into `std::string`
instances, and then when they are processed by another thread, these
`std::string` instances are _recycled_ back to the pool. There's one "filling"
thread that _reads_ from the queue, and another that "recycles" the spent
instances back to the pool.

Multiple-Producer, Single-Consumer Containers
=============================================

Separated into a different namespace, the multiple-producer, single-consumer,
containers have taken advantage of the fact that they are _filled_ by
many threads, but they are _emptied_ by one *and only one* thread. This means
that their safe use in the code is the responsibility of the developer as
they _must_ be sure of the threads that are using these containers. However,
when those conditions are met, these queues and other containers are
very efficient, and perform their task admirably.

dkit::mpsc::LinkedFIFO
----------------------

The simplest MPSC (multiple-producer, single-consumer) queue is a simple linked
FIFO (first-in, first-out) queue. In contrast to the CircularFIFO in this
namespace, the LinkedFIFO allows for an unlimited size, constrained only by
the available memory of the device. The trade-off is, of course, that the
elements placed in this queue have their storage allocated on the heap. In
contrast to the CircularFIFO, whose size is predetermined, and whose locations
are contiguous, the LinkedFIFO can have it's elements scattered around the
memory space, and therefore not as efficient to access as the CircularFIFO.

Yet, there will be times when this is not a significant downside, and the
fact that the implementation is simple, and efficient makes up for the slight
inefficiency in the accessing of elements in the queue.

dkit::mpsc::CircularFIFO
------------------------

The fastest MPSC queue is a simple circular FIFO queue. The implementation
of this is very similar to that of the SPSC circular FIFO queue, with the
significant differences coming into play in the data structures as well as
the implementation of the `push()` and `pop()` methods.

The big difference in this implementation is that in order to allow for
multiple producers, we had to ensure that the `push()` method did ONE and
_only one_ atomic operation to find a new, open, spot for the incoming data.
This means that the "increment and test" scheme in the SPSC CircularFIFO will
NOT work, and we had to come up with something a little more interesting.

SImilarly, the `pop()` method needed to take into account that we have but a
single thread hitting this method, and yet we needed to make sure that we
integrated nicely with the data structures that were necessary for the
multiple producers.

Single-Producer, Multiple-Consumer Containers
=============================================

Separated into a different namespace, the single-producer, multiple-consumer,
containers have taken advantage of the fact that they are _filled_ by
one thread, but they are _emptied_ by _many_ threads. This means
that their safe use in the code is the responsibility of the developer as
they _must_ be sure of the threads that are using these containers. However,
when those conditions are met, these queues and other containers are
very efficient, and perform their task admirably.

dkit::spmc::LinkedFIFO
----------------------

The simplest SPMC (single-producer, multiple-consumer) queue is a simple linked
FIFO (first-in, first-out) queue. Like the LinkedIFO on the MPSC namespace,
this LinkedFIFO allows for an unlimited size, constrained only by
the available memory of the device. The trade-off is, of course, that the
elements placed in this queue have their storage allocated on the heap.

Yet, there will be times when this is not a significant downside, and the
fact that the implementation is simple, and efficient makes up for the slight
inefficiency in the accessing of elements in the queue.

dkit::spmc::CircularFIFO
------------------------

In order to implement a SPMC circular FIFO queue, we had to abandon the use
of `_head` and `_tail` in the calculation of the `size()`. This was because
it was _possible_ to have a lockless queue if we allow that the `_head` and
`_tail` are _at times_ only _potential_ indexes into the queue and not a
definite location at the time. With this, we can _back up_ certain
operations so that it's possible to keep things lockless, and the only real
cost is that the `size()` method can't depend on these values.

To remedy this problem, we added a simple `_size` instance variable and used
the CAS increment and decrement operators when we are _certain_ that the
`push()` and `pop()` methods have succeeded. The downside of this is that
**all** the operations are going to take longer. How much longer? On my
development machine, here's a run of the MPSC CircularFIFO:

	=== Testing speed and correctness of CircularFIFO ===
	Passed - pushed on 500 integers
	Passed - popped all 500 integers
	Passed - unable to pop from an empty queue
	Passed - did 50000000 push/pop pairs in 1127.99ms = 22.5598ns/op

and the overhead of the `_size` maintenance is seen in the SPMC queue:

	=== Testing speed and correctness of CircularFIFO ===
	Passed - pushed on 500 integers
	Passed - popped all 500 integers
	Passed - unable to pop from an empty queue
	Passed - did 50000000 push/pop pairs in 2781.11ms = 55.6222ns/op

It's still important to understand this is far better than the linked FIFO
queues, but there is a speed penalty, and it's important to keep this in mind.

Utility/Helper Classes
======================

In order to make the code in the classes a little easier to use and work with,
we've also created a few utility classes put in a separate namespace: `util`,
and located in the util directory.

dkit::util::timer
-----------------

This is a simple timer class that allows fast and accurate timing on the
different platforms (linux, Mac OS X) that this codebase is targeted for.
The reason for having this is that many of the test apps need to have accurate
and lightweight timings to test for efficiencies as well as just to gather
stats on the code.

There are referenced (epoch) and non-referenced timers because when you are
doing interval timing you don't need to take the time to get the referenced
timestamps, and it's all about lightweight measurements.

License
=======

	Copyright 2012 by Robert E. Beaty

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files (the "Software"),
	to deal in the Software without restriction, including without limitation
	the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
	OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
