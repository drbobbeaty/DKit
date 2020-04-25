# DKit

<p align="center">
  <img src="doc/img/dexter.jpeg" width="477" height="250" border="0" />
</p>

[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-Ready--to--Code-blue?logo=gitpod)](https://gitpod.io/#https://github.com/drbobbeaty/DKit) 

### Introduction

In all the work we've done over the years, nothing has been quite as liberating
as thread-safe containers. They are an amazing experience for a developer
because they allow the multi-threaded programmer to not have to worry about
locks, lock contention, deadlocks, dirty reads, and a whole host of other
issues that for years have made writing multi-threaded code difficult. These
containers and objects free the user from having to worry about locking
because they use the _compare-and-swap_ (CAS) operations in GCC to ensure
that the data accessed is done properly, and securely.

### Library Dependencies

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

### Base Container Template Classes

In order to make the conversion of the code as simple as possible, we have
made a base class for each type of container in this library. These base
classes are most easily used as instance variables, or method arguments, and
then the specific type of container can be instantiated and used as if it
were just the base class.

At the current time, we have the following base classes:

*	`dkit::FIFO` a simple first-in, first-out queue

### Async I/O Package

As we get away from the low-level classes in DKit, we start to see how these
components can be put together to form more significant components. One such
group is the I/O package based on the [boost](www.boost.org)'s ASIO package.
The problem with the [boost](www.boost.org) package is that it _still_ takes
a non-trivial amount of work to put together some of the most fundamental
things you'd need in a socket I/O library. Certainly, this is for flexibility,
but when you're trying to handle exchange feeds, it makes it a lot easier if
you don't start from that point, but maybe a little higher up the food chain.

This is the purpose of the DKit I/O package - to bring a set of completed,
high-performance socket components that makes buidling these types of systems
much easier.

Atomic Integers
---------------

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
-------------------------------------------

Separated into a different namespace, the single-producer, single-consumer,
containers have taken advantage of the fact that they are only _filled_ by
one __and only one__ thread, and that they are _emptied_ by one __and only
one__ thread. This means that their safe use in the code is the responsibility
of the developer as they _must_ be sure of the threads that are using these
containers. However, when those conditions are met, these queues and other
containers are exceptionally efficient, and perform their task very fast.

### dkit::spsc::CircularFIFO

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
---------------------------------------------

Separated into a different namespace, the multiple-producer, single-consumer,
containers have taken advantage of the fact that they are _filled_ by
many threads, but they are _emptied_ by one *and only one* thread. This means
that their safe use in the code is the responsibility of the developer as
they _must_ be sure of the threads that are using these containers. However,
when those conditions are met, these queues and other containers are
very efficient, and perform their task admirably.

### dkit::mpsc::LinkedFIFO

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

### dkit::mpsc::CircularFIFO

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
---------------------------------------------

Separated into a different namespace, the single-producer, multiple-consumer,
containers have taken advantage of the fact that they are _filled_ by
one thread, but they are _emptied_ by _many_ threads. This means
that their safe use in the code is the responsibility of the developer as
they _must_ be sure of the threads that are using these containers. However,
when those conditions are met, these queues and other containers are
very efficient, and perform their task admirably.

### dkit::spmc::LinkedFIFO

The simplest SPMC (single-producer, multiple-consumer) queue is a simple linked
FIFO (first-in, first-out) queue. Like the LinkedIFO on the MPSC namespace,
this LinkedFIFO allows for an unlimited size, constrained only by
the available memory of the device. The trade-off is, of course, that the
elements placed in this queue have their storage allocated on the heap.

Yet, there will be times when this is not a significant downside, and the
fact that the implementation is simple, and efficient makes up for the slight
inefficiency in the accessing of elements in the queue.

### dkit::spmc::CircularFIFO

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

Conflation Queue
----------------

When dealing with a stream of data that's coming _faster_ than you can process
it, it's often very helpful to have a _conflation queue_ that conflates the
data in the stream. Basically, each item going into the queue has a
`key_value()` associated with it. This `key_value()` can be of varying sizes,
but it's unique for this __type__ of data.

Say we have market data. IBM's trade price is coming in faster than we can
process it, so we want to be able to have the _latest_ price of IBM, but I
don't care if I skip some values that I didn't have time to process anyway.

This conflation queue is built on one of the `CircularFIFO` queues as well as
a `trie` (see below). The combination is that we can update existing values
on the queue while maintaining their position in the queue, and do it all in
a lockless way. This queue is very fast for what it does.

Using it is primarily a matter of configuration. After that, it's just a
`FIFO<T>` queue, and you can `push()` and `pop()` just like any other `FIFO<T>`
queue. But configuration is a little more involved:

```cpp
namespace dkit {
/**
 * This is the main class definition. The paramteres are as follows:
 *   T = the type of data to store in the conflation queue
 *   N = power of 2 for the size of the queue (2^N)
 *   Q = the type of access the queue has to have (SP/MP & SC/MC)
 *   KS = the size of the key for the value 'T'
 *   PN = the power of two of pooled keys to have in reserve (default: 2^17)
 */
template <class T, uint8_t N, queue_type Q, trie_key_size KS, uint8_t PN = 17> class cqueue :
    public FIFO<T>
{
};
}   // end of namespace dkit
```

You can think of this as the configuration of the queue, the trie, and then
a pool of key values for the queue.

Variable Key Sized Trie
-----------------------

One of the _most_ efficient lockless data structures I've used in the past
couple of years is the [trie](http://en.wikipedia.org/wiki/Trie). This is
an _exceptionally_ efficient way of organizing a lot of data with very fast
access times into, and out of, the structure. I have found that placing
pointers into the trie makes it very easy as those are then CAS-ed into place
and read as easily. The structure of the trie is built out once, and then
left in place, even if the contents are NULL-ed out. It's simple, but very
effective.

### dkit::trie<T, N>

The trie in DKit takes two parameters: the data type that's going to be held,
and the size of the key used to reference these data elements. The size can be
one of the following:

```cpp
namespace dkit {
enum trie_key_size {
	uint16_key = 2,
	uint32_key = 4,
	uint64_key = 8,
	uint128_key = 16,
};
}		// end of namespace dkit
```

The memory management of the contents of the trie is __always__ the
responsibility of the trie. Therefore, if the template type is a pointer,
then the storage associated with these pointers will be freed by the trie
when the values are overwritten, or simply deleted. For non-pointers, the
same is true, but there isn't the impact to leaking and memory management.

The way values are placed into the trie is dictated by the `key` that is
generated for each value. For the template value type, it's required that
a method be implemented to provide the key for a given value:

```cpp
uint64_t key_value( const blob *aValue )
{
	return (*aValue).getValue();
}
```

Of course, this example assumes you're using the 64-bit key size, but it's
possible to make it for any of the associated key sizes.

Once the trie is created, adding values is fairly simple:

```cpp
dkit::trie<blob *, dkit::uint64_key>	m;
for (uint64_t i = 0; i < cnt; ++i) {
	blob	*b = new blob(i);
	m.put(b);
}
```

There are methods for accessing the data as well, and because it's possible to
ask for a value that's simply not available, we need to return a `bool`
indicating whether or not we found a value to return. In practice, this looks
something like this:

```cpp
blob		*bp = NULL;
for (uint64_t i = 0; i < cnt; ++i) {
	if (!m.get(i, bp)) {
		error = true;
		std::cout << "ERROR - failed to get key=" << i << "!" << std::endl;
		break;
	}
}
```

The final significant feature of the trie is it's functor-based access to the
contents of the trie. All that's required is to subclass the trie's functor
class, implement the `process()` method, and then work on the `Node` structure
as desired:

```cpp
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
```
and then:
```cpp
counter		worker;
m.apply(worker);
```

With this general structure it's possible to make a great number of storage
containers, and they all should be very high performance.

Source, Sink and Adapter Base Classes
-------------------------------------

In addition to the atomic integers and the lockless containers, this library
starts to combine these things into some interesting components that we've
used in data feeds and high through-put systems. To make use of the queues,
it's nice to have a framework to stitch them together, so we have created the
`dkit::source`, `dkit::sink` and the `dkit::adapter`. These base classes are
template classes that work together to have a very simple, but very effective,
pub/sub system.

The naming convention is from the perspective of the running process. A
`dkit::source` primarily takes things from any source and generates the
template values for the `dkit::sink` instances to listen to. An example of
a `dkit::source` is a UDP multicast listener. One might subclass the
`dkit::source` and specialize it to pass `std::string` instances, and then
simply build the code that opened up the UDP socket, listened for the
datagrams, and when one came in, convert it to a `std::string` and then
call `send()` with it.

Each `dkit::source` can have as many `dkit::sink` instances registered as
listeners as needed. When a call is made to `dkit::source::send()`, all the
registered listeners are sent the same item, in turn. The order is not
guaranteed, but all will be messaged with the data. With these classes being
templates, it's easy to make them move integers, or pointer to classes, etc.

The `dkit::adapter` class is a `dkit::source` and a `dkit::sink`
_back-to-back_ so that it _takes_ one template type, and generates another
template type. This could be in-line, or it could be a buffered operation,
or it could be that one input generates many outputs - it's totally up to
the implementation. The use of this is just as easy as you'd think - as a
multiple inheritance template class, it's just got the same methods that
the `dkit::source` and `dkit::sink` have - just in one object.

The real utility of these classes is that they provide all the necessary
infrastructure that's needed for you to build on. You simple subclass the
appropriate class, and even specialize it with a given type, and you can add
in all the behaviors you need.

Pools
-----

One of the components we can build with the components already described is
a simple _object pooler_. This _pooler_ is a template class that allows the
user to specify what it is that's being pooled, how many to keep (by a
power of 2), and what's the _scope_ of the usage. The idea is that if you
are building a data reader, and have the need for a lot of `std::string`
instances - but you only need them for a little bit, and then back into a
pool they can go, then you can make a pool very simply:

```c++
#include "pool.h"

// make a pool of up to 2^5 (=32) std::string pointers
dkit::pool<std::string *, 5, dkit::sp_sc>	pool

std::string	*n = pool.next();

// ...do something with the std::string pointer

pool.recycle(n);
```

This usage is very common in a lot of data handling and messaging systems.
So much so that it was one of the reasons that these lockless queues were
written in the first place.

With this, the user can easily make a pool of just about anything. It
properly handles pointers as well as plain-old-datatypes.

Async I/O Components
--------------------

The first addition to DKit's I/O package is the UDP Receiver. This is a
subclass of the `source<T>` class, discussed above, and it's specialized to
deliver the UDP datagrams that will be arriving on a UDP multicast channel.
There are several classes that work together to make this happen:

### dkit::io::channel

The basic TCP channel can be described as an address and a port,
or it can be combined into a URL of the form: `tcp://<addr>:<port>` like:
`tcp://239.255.0.1:30001`. These are easily constructed either way:

```c++
#include "channel.h"

dkit::io::channel	chan_a("239.255.1.1", 8081);

dkit::io::channel	chan_b("tcp://239.255.1.1:8081");
```

The second form is probably going to be more useful in the long-run as it's
the easiest way to have the channels defined in some persistence system or
config file and have all parts identified clearly and plainly.

### dkit::io::multicast_channel

The basic UDP multicast channel can be described as an address and a port,
or it can be combined into a URL of the form: `udp://<addr>:<port>` like:
`udp://239.255.0.1:30001`. These are easily constructed either way:

```c++
#include "multicast_channel.h"

dkit::io::multicast_channel	chan_a("239.255.1.1", 30001);

dkit::io::multicast_channel	chan_b("udp://239.255.1.1:30001");
```

The second form is probably going to be more useful in the long-run as it's
the easiest way to have the channels defined in some persistence system or
config file and have all parts identified clearly and plainly.

### dkit::io::datagram

This is the primary transport container for the UDP datagrams coming out of
the UDP reveicer. These are created within the UDP receiver and maintained
in a pool so that there is a steady supply available at any time. This has
been found to be a critical point in high-performace systems as it virtually
removes all creation/destruction slowdowns.

The important data in the datagram is the data, it's size, and when it was
captured off the host OS socket buffer. This is the first point we can
possibly tag it, and it's as close to the actual arrival time as we can be.

### dkit::io::udp_transmitter

This is the main class for the UDP transmitter, and it's use is fairly simple:
create an instance, deciding to share an ASIO thread for processing - or not,
and then wire it up to a `dkit::source<datagram*>` with `addToListeners()`.
When the source sends a datagram to our transmitter, it'll take the contents
of that datagram and send it out on the configured UDP multicast channel using
boost's ASIO for sending.

```c++
udp_transmitter	xmit(multicast_channel("udp://239.255.0.1:30001"));
rcvr.addToListeners(&xmit);
```

When you're done, remove it as a listener from the source, tell it to shutdown.
That's it.

### dkit::io::udp_receiver

This is the main class for the UDP receiver, and it's use is fairly simple:
create an instance, deciding to share an ASIO thread for processing - or not,
and then start it listening. When you're done, tell it to shutdown. That's
it.

```c++
MySink<datagram*>	dump;
udp_receiver	rcvr(multicast_channel("udp://239.255.0.1:30001"));
rcvr.addToListeners(&dump);
rcvr.listen();
// now let's stay in this loop as long as we need to...
while (rcvr.isListening() && !dump.allDone()) {
	sleep(1);
}
std::cout << "shutting down due to inactivity..." << std::endl;
rcvr.shutdown();
```

The only real tricky part is how to make the receiver of the datagrams.

### Creating the sink<T> for Datagrams

Because of the way that the template classes are constructed, the way to
make a `sink<t>` subclass for UDP datagrams is to subclass the template
class `sink<T>` and _leave it as a template class_ and then when you
instantiate it, you specialize it to listen for the data type `<datagram*>`.

This is highlighted in this minimal implementation:

```c++
template <class T> class MySink :
	public dkit::sink<T>
{
	public:
		MySink() { }

		/**
		 * This is the main receiver method that we need to call out to
		 * a concrete method for the type we're using. It's what we have
		 * to do to really get a virtual template class working for us.
		 */
		virtual bool recv( const T anItem )
		{
			return onMessage(anItem);
		}

		/**
		 * This method is called when we get a new datagram, and because
		 * we are expecting to instantiate this template class with the
		 * type 'T' being a <datagram *>, this is the method we're expecting
		 * to get hit. It's just that simple.
		 */
		bool onMessage( const datagram *dg ) {
			if (dg == NULL) {
				std::cout << "got a NULL" << std::endl;
			} else {
				std::cout << "got: " << dg->contents() << std::endl;
			}
			return true;
		}
};
```


Utility/Helper Classes
----------------------

In order to make the code in the classes a little easier to use and work with,
we've also created a few utility classes put in a separate namespace: `util`,
and located in the util directory.

### dkit::util::timer

This is a simple timer class that allows fast and accurate timing on the
different platforms (linux, Mac OS X) that this codebase is targeted for.
The reason for having this is that many of the test apps need to have accurate
and lightweight timings to test for efficiencies as well as just to gather
stats on the code.

There are referenced (epoch) and non-referenced timers because when you are
doing interval timing you don't need to take the time to get the referenced
timestamps, and it's all about lightweight measurements.

With these timestamps, it's important to be able to have them be
human-readable, so the `timer` class has static methods for converting the
timestamps into nicely formatted strings. You can get the complete timestamp
(date and time) with and without microseconds, just the date, and just the
time. While not meant to be a general time format class, it's nice to have
something like this in the timer.

License
-------

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
