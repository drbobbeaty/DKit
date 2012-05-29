/**
 * adapter.h - this file defines the general 'adapter' of two objects: an
 *             incoming object, TIN, and an outgoing object, TOUT. This is
 *             all about templates, so what that the objects are depends on
 *             the user, but the idea is that this class is the base class
 *             for all things that take one object instances, and produce
 *             another object instances. This is a fusion of the source and
 *             sink templates, but no multiple inheritance overhead.
 */
#ifndef __DKIT_ADAPTER_H
#define __DKIT_ADAPTER_H

//	System Headers
#include <stdint.h>
#include <ostream>
#include <string>

//	Third-Party Headers
#include <boost/unordered_set.hpp>
#include <boost/smart_ptr/detail/spinlock.hpp>
#include <boost/foreach.hpp>

//	Other Headers
#include "abool.h"

//	Forward Declarations
/**
 * We are going to be dealing with sources INTO this object, and sinks
 * OUT of this object, so we really need to know what they are so we can
 * properly deal with them.
 */
#include "source.h"
#include "sink.h"

//	Public Constants

//	Public Datatypes

//	Public Data Constants

/**
 * Main class definition
 */
namespace dkit {
template <class TIN, class TOUT> class adapter :
	public sink<TIN>,
	public source<TOUT>
{
	public:
		/********************************************************
		 *
		 *                Constructors/Destructor
		 *
		 ********************************************************/
		/**
		 * This is the default constructor that sets up the adapter
		 * with NO listeners, but ready to take on as many as you need.
		 */
		adapter() :
			sink<TIN>(),
			source<TOUT>(),
			_name("adapter"),
			_mutex(),
			_online(true)
		{
		}


		/**
		 * This is the standard copy constructor and needs to be in every
		 * class to make sure that we don't have too many things running
		 * around.
		 */
		adapter( const adapter<TIN, TOUT> & anOther ) :
			sink<TIN>(),
			source<TOUT>(),
			_name("adapter"),
			_mutex(),
			_online(true)
		{
			// let the '=' operator do all the heavy lifting
			*this = anOther;
		}


		/**
		 * This is the standard destructor and needs to be virtual to make
		 * sure that if we subclass off this the right destructor will be
		 * called.
		 */
		virtual ~adapter()
		{
			// remove all the sources for this guy - no dangling pointers
			removeAllPublishers();
			// remove all the listeners of this guy - no dangling pointers
			removeAllListeners();
		}


		/**
		 * When we want to process the result of an equality we need to
		 * make sure that we do this right by always having an equals
		 * operator on all classes.
		 */
		adapter<TIN, TOUT> & operator=( const adapter<TIN, TOUT> & anOther )
		{
			/**
			 * Make sure that we don't do this to ourselves...
			 */
			if (this != & anOther) {
				// copy over the name - just to be complete
				_name = anOther._name;
				// for each of his sources, add them into my stuff
				BOOST_FOREACH( source<TIN> *s, anOther.getSources() ) {
					addToPublishers(s);
				}
				// for each of his sinks, add them into my stuff
				BOOST_FOREACH( sink<TOUT> *t, anOther.getSinks() ) {
					addToListeners(t);
				}
				// now just copy over the online status
				_online = anOther._online;
			}
			return *this;
		}


		/********************************************************
		 *
		 *                Accessor Methods
		 *
		 ********************************************************/
		/**
		 * This method allows the user to set the name of this adapter
		 * for debugging and tracking purposes. It's not required, and
		 * there's no requirement that the names are unique, it's just
		 * a convenience that can be quite useful at times.
		 */
		virtual void setName( const std::string & aName )
		{
			boost::detail::spinlock::scoped_lock	lock(_mutex);
			_name = aName;
		}


		/**
		 * This method gets the current name of this adapter so that it
		 * can be logged, or used in whatever manner desired. There is
		 * no other significance to the name.
		 */
		virtual const std::string & getName() const
		{
			boost::detail::spinlock::scoped_lock	lock(_mutex);
			return _name;
		}


		/**
		 * This method is called to add the provided sink as a listener
		 * to this adapter's list of targets for the send() method. It's
		 * a one-time registration, meaning, there's no way to get the
		 * same message twice if you mistakenly attempted to add yourself
		 * twice. If this is the first registration for this sink, then
		 * a 'true' will be returned.
		 */
		virtual bool addToListeners( sink<TOUT> *aSink )
		{
			return source<TOUT>::addToListeners(aSink);
		}


		/**
		 * This method attempts to un-register the provided sink from
		 * the list of registered listeners for this adapter. If the
		 * sink is NOT a registered listener, then this method will do
		 * nothing and return 'false'. Otherwise, the sink will no longer
		 * receive calls, and a 'true' will be returned.
		 */
		virtual bool removeFromListeners( sink<TOUT> *aSink )
		{
			return source<TOUT>::removeFromListeners(aSink);
		}


		/**
		 * This method removes ALL the registered sinks from the list
		 * for this adapter. This effectively puts this adapter "offline"
		 * for the time-being as there's nothing for him to do. The
		 * consequence of this might be severe as the incoming events
		 * might be lost or queue up, depending on the implementation.
		 */
		virtual void removeAllListeners()
		{
			source<TOUT>::removeAllListeners();
		}


		/**
		 * This method is called to add the provided source as a publisher
		 * to this adapter's recv() method. It's a one-time registration,
		 * meaning, there's no way to register the same publisher twice
		 * if you mistakenly attempted to add yourself twice. If this is
		 * the first registration for this source, then a 'true' will be
		 * returned.
		 */
		virtual bool addToPublishers( source<TIN> *aSource )
		{
			return sink<TIN>::addToPublishers(aSource);
		}


		/**
		 * This method attempts to un-register the provided source from
		 * the list of registered publishers for this adapter. If the
		 * source is NOT a registered publisher, then this method will do
		 * nothing and return 'false'. Otherwise, the source will no longer
		 * send data to this instance, and a 'true' will be returned.
		 */
		virtual bool removeFromPublishers( source<TIN> *aSource )
		{
			return sink<TIN>::removeFromPublishers(aSource);
		}


		/**
		 * This method removes ALL the registered sources from the list
		 * for this adapter. This effectively puts this sink "offline"
		 * for the time-being as there's nothing for him to process.
		 */
		virtual void removeAllPublishers()
		{
			sink<TIN>::removeAllPublishers();
		}


		/**
		 * This method, and it's convenience methods, are here to allow
		 * the user to leave the listener/subscriber connections in place,
		 * but shut down the flow of data from source to sink by taking
		 * this guy offline. This will simple make the send() a no-op, and
		 * the caller won't know the difference. It's a clean way to simply
		 * stop the flow of data.
		 */
		virtual void setOnline( bool aFlag )
		{
			_online = aFlag;
		}


		virtual void takeOnline()
		{
			_online = true;
		}


		virtual void takeOffline()
		{
			_online = false;
		}


		/**
		 * This method is used to see if the adapter is online or not.
		 * By default, it's online but it's possible that it's been taken
		 * offline for some reason, and this is a good way to find out.
		 */
		virtual bool isOnline() const
		{
			return (bool)_online;
		}


		/********************************************************
		 *
		 *              Distribution Methods
		 *
		 ********************************************************/
		/**
		 * This method sends the item out to all the registered users
		 * of this adapter - one at a time, in no particular order, so
		 * that they might be able to process/handle this item as best
		 * they see fit. This is a constant, so the sinks are NOT going
		 * to be able to modify the data, but they will get the chance
		 * to copy it, if they wish, and update that copy.
		 */
		virtual bool send( const TOUT anItem )
		{
			return source<TOUT>::send(anItem);
		}


		/********************************************************
		 *
		 *                Processing Methods
		 *
		 ********************************************************/
		/**
		 * This method is called when a source has an item to deliver
		 * to this, a registered listener of that sender. It is up to
		 * this method to process that WITH ALL HASTE, so that the
		 * sender can send the SAME item to the next registered listener
		 * in the set. We can copy this item, but we can't mutate it
		 * as it's a constant to us.
		 */
		virtual bool recv( const TIN anItem )
		{
			/**
			 * Here's where we need to process the item and do what we
			 * need to with it. We need to be quick about it, though,
			 * as the source is waiting on us to finish, and send the
			 * same item to all the other registered listeners.
			 */
			return true;
		}


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
		virtual std::string toString() const
		{
			std::ostringstream	msg;
			msg << "[adapter '" << _name << "' w/ "
				<< source<TOUT>::getSinks().size() << " sinks, "
				<< sink<TIN>::getSources().size() << " sources]";
			return msg.str();
		}


		/**
		 * When we have a custom '==' operator, it's wise to have a hash
		 * method as well. This makes it much easier to used hash-based
		 * containers like boost, etc. It's a requirement that if two
		 * instances are '==', then their hash() methods must be '==' as
		 * well.
		 */
		virtual size_t hash() const
		{
			size_t	ans = boost::hash_value(_name);
			BOOST_FOREACH( sink<TOUT> *s, source<TOUT>::getSinks() ) {
				if (s != NULL) {
					boost::hash_combine(ans, s);
				}
			}
			BOOST_FOREACH( source<TIN> *t, sink<TIN>::getSources() ) {
				if (t != NULL) {
					boost::hash_combine(ans, t);
				}
			}
			boost::hash_combine(ans, (bool)_online);
			return ans;
		}


		/**
		 * This method checks to see if the two adapters are equal to one
		 * another based on the values they represent and *not* on the
		 * actual pointers themselves. If they are equal, then this method
		 * returns true, otherwise it returns false.
		 */
		bool operator==( const adapter<TIN, TOUT> & anOther ) const
		{
			bool		equals = false;
			if ((this == & anOther) ||
				((_name == anOther._name) &&
				 (source<TOUT>::getSinks() == ((source<TOUT> &)anOther).getSinks()) &&
				 (sink<TIN>::getSources() == ((sink<TIN> &)anOther).getSources()) &&
				 (_online == anOther._online))) {
				equals = true;
			}
			return equals;
		}


		/**
		 * This method checks to see if the two adapters are not equal to
		 * one another based on the values they represent and *not* on the
		 * actual pointers themselves. If they are not equal, then this
		 * method returns true, otherwise it returns false.
		 */
		bool operator!=( const adapter<TIN, TOUT> & anOther ) const
		{
			return !operator==(anOther);
		}


	private:
		/**
		 * There will be times that naming the sources will be very useful.
		 * For this reason, we'll have a name here - initially just "source",
		 * but the user can change this as they see fit. It's just a useful
		 * thing to have when dealng with a great number of sources.
		 */
		std::string							_name;
		// ...and a spinlock to protect what's left here in the adapter
		mutable boost::detail::spinlock		_mutex;
		/**
		 * We can take this source offline, and have it "idle", but
		 * in order to do that, we need a nice boolean that we can
		 * flip without any threading issues. This is it.
		 */
		abool								_online;
};
}		// end of namespace dkit

#endif		// __DKIT_ADAPTER_H
