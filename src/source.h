/**
 * source.h - this file defines the general 'source' of an object. This is
 *            all about templates, so what that 'object' is depends on the
 *            user, but the idea is that this class is the base class for all
 *            things that GENERATE instances of class T INTO the process
 *            space. There is a simple listener pattern set up between the
 *            sources and the sinks - the sources have a send() method and
 *            the sinks have an recv() method and the send() sends the one
 *            instance of T to all registered listeners by calling their
 *            recv() method. It's pretty simple. The goal is to make this
 *            the foundation for a more general message passing architecture.
 */
#ifndef __DKIT_SOURCE_H
#define __DKIT_SOURCE_H

//	System Headers
#include <stdint.h>
#include <ostream>
#include <string>

//	Third-Party Headers
#include <boost/unordered_set.hpp>
#include <boost/smart_ptr/detail/spinlock.hpp>

//	Other Headers
#include "abool.h"

//	Forward Declarations
/**
 * Because the sources and sinks are really dependent on one another, it
 * makes sense to have them both defined in the forward sense so that the
 * definitions are simple, not circular, and we don't have to include the
 * one header in the other, making it a mess.
 */
template <class T> class source;
template <class T> class sink;

//	Public Constants

//	Public Datatypes

//	Public Data Constants

/**
 * Main class definition
 */
namespace dkit {
template <class T> class source
{
	public:
		/********************************************************
		 *
		 *                Constructors/Destructor
		 *
		 ********************************************************/
		/**
		 * This is the default constructor that sets up the source
		 * with NO listeners, but ready to take on as many as you need.
		 */
		source();
		/**
		 * This is the standard copy constructor and needs to be in every
		 * class to make sure that we don't have too many things running
		 * around.
		 */
		source( const source<T> & anOther );
		/**
		 * This is the standard destructor and needs to be virtual to make
		 * sure that if we subclass off this the right destructor will be
		 * called.
		 */
		virtual ~source();

		/**
		 * When we want to process the result of an equality we need to
		 * make sure that we do this right by always having an equals
		 * operator on all classes.
		 */
		source<T> & operator=( const source<T> & anOther );

		/********************************************************
		 *
		 *                Accessor Methods
		 *
		 ********************************************************/
		/**
		 * This method allows the user to set the name of this source
		 * for debugging and tracking purposes. It's not required, and
		 * there's no requirement that the names are unique, it's just
		 * a convenience that can be quite useful at times.
		 */
		virtual void setName( const std::string & aName );
		/**
		 * This method gets the current name of this source so that it
		 * can be logged, or used in whatever nammer desired. There is
		 * no other significance to the name.
		 */
		virtual const std::string & getName() const;

		/**
		 * This method is called to add the provided sink as a listener
		 * to this source's list of targets for the send() method. It's
		 * a one-time registration, meaning, there's no way to get the
		 * same message twice if you mistakenly attempted to add yourself
		 * twice. If this is the first registration for this sink, then
		 * a 'true' will be returned.
		 */
		virtual bool addToListeners( sink<T> *aSink );
		/**
		 * This method attempts to un-register the provided sink from
		 * the list of registered listeners for this source. If the
		 * sink is NOT a registered listener, then this method will do
		 * nothing and return 'false'. Otherwise, the sink will no longer
		 * receive calls, and a 'true' will be returned.
		 */
		virtual bool removeFromListeners( sink<T> *aSink );
		/**
		 * This method removes ALL the registered sinks from the list
		 * for this source. This effectively puts this source "offline"
		 * for the time-being as there's nothing for him to do.
		 */
		virtual void removeAllListeners();

		/**
		 * This method, and it's convenience methods, are here to allow
		 * the user to leave the listener/subscriber connections in place,
		 * but shut down the flow of data from source to sink by taking
		 * this guy offline. This will simple make the send() a no-op, and
		 * the caller won't know the difference. It's a clean way to simply
		 * stop the flow of data.
		 */
		virtual void setOnline( bool aFlag );
		virtual void takeOnline();
		virtual void takeOffline();
		/**
		 * This method is used to see if the source is online or not.
		 * By default, it's online but it's possible that it's been taken
		 * offline for some reason, and this is a good way to find out.
		 */
		virtual bool isOnline() const;

		/********************************************************
		 *
		 *              Distribution Methods
		 *
		 ********************************************************/
		/**
		 * This method sends the item out to all the registered users
		 * of this sender - one at a time, in no particular order, so
		 * that they might be able to process/handle this item as best
		 * they see fit. This is a constant, so the sinks are NOT going
		 * to be able to modify the data, but they will get the chance
		 * to copy it, if they wish, and update that copy.
		 */
		virtual bool send( const T anItem );

		/********************************************************
		 *
		 *                Utility Methods
		 *
		 ********************************************************/
		/**
		 * There are a lot of times that a human-readable version of
		 * this instance will come in handy. This is that method. It's
		 * not necessarily meant to be something to process, but most
		 * likely what a debugging system would want to write out for
		 * this guy.
		 */
		virtual std::string toString() const;

		/**
		 * This method checks to see if the two sources are equal to one
		 * another based on the values they represent and *not* on the
		 * actual pointers themselves. If they are equal, then this method
		 * returns true, otherwise it returns false.
		 */
		bool operator==( const source<T> & anOther ) const;
		/**
		 * This method checks to see if the two sources are not equal to
		 * one another based on the values they represent and *not* on the
		 * actual pointers themselves. If they are not equal, then this
		 * method returns true, otherwise it returns false.
		 */
		bool operator!=( const source<T> & anOther ) const;

	protected:
		friend class sink<T>;

		/********************************************************
		 *
		 *              Non-Feedback Accessor Methods
		 *
		 ********************************************************/
		/**
		 * This method is used to actually add the sink to the set of
		 * sinks that we are tracking in this source. This is not
		 * typically for general consumption as it's just doing half
		 * the job - the other is to let the sink know I'm one of it's
		 * publishers.
		 */
		bool addToSinks( sink<T> *aSink );
		/**
		 * This method is used to actually remove the sink from the set
		 * of sinks that we are tracking. Again, this is ony half the
		 * battle, as we need to tell the sink to remove us from it's
		 * list of subscribers, but that's in the public API.
		 */
		void removeFromSinks( sink<T> *aSink );
		/**
		 * This method is used to actually clear out all the sinks we
		 * have in the set, but it's assumed that they have been told
		 * to remove us as a publisher as well.
		 */
		void removeAllSinks();
		/**
		 * This method looks at the current list of sinks and returns
		 * 'true' if the provided sink is in the registered list. This
		 * is the thread-safe way to know if a given sink is in the list.
		 */
		bool isSink( sink<T> *aSink );

	private:
		/**
		 * There will be times that naming the sources will be very useful.
		 * For this reason, we'll have a name here - initially just "source",
		 * but the user can change this as they see fit. It's just a useful
		 * thing to have when dealng with a great number of sources.
		 */
		std::string							_name;
		/**
		 * This is the set of all the sinks we know about. They will
		 * be added to in the protected accessor methods, and the public
		 * API will do that, and the feedback part of making sure the
		 * sinks know that we are one of their publishers.
		 */
		boost::unordered_set< sink<T> * >	_sinks;
		// ...and a spinlock to protect the list
		boost::detail::spinlock				_mutex;
		/**
		 * We can take this source offline, and have it "idle", but
		 * in order to do that, we need a nice boolean that we can
		 * flip without any threading issues. This is it.
		 */
		abool								_online;
};
}		// end of namespace dkit

#endif		// __DKIT_SOURCE_H
