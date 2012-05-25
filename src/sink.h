/**
 * sink.h - this file defines the general 'sink' of an object. This is
 *          all about templates, so what that 'object' is depends on the
 *          user, but the idea is that this class is the base class for all
 *          things that CONSUME instances of class T OUT OF the process
 *          space. There is a simple listener pattern set up between the
 *          sources and the sinks - the sources have a send() method and
 *          the sinks have an recv() method and the send() sends the one
 *          instance of T to all registered listeners by calling their
 *          recv() method. It's pretty simple. The goal is to make this
 *          the foundation for a more general message passing architecture.
 */
#ifndef __DKIT_SINK_H
#define __DKIT_SINK_H

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
 * Because the sources and sinks are really dependent on one another, we
 * have to make sure that we can reference one another. The way to do that
 * is to forward reference the class we're defining, and include the
 * template class we are using, and then we should be good to go.
 */
namespace dkit {
template <class T> class sink;
}		// end of namespace dkit
#include "source.h"

//	Public Constants

//	Public Datatypes

//	Public Data Constants

/**
 * Main class definition
 */
namespace dkit {
template <class T> class sink
{
	public:
		/********************************************************
		 *
		 *                Constructors/Destructor
		 *
		 ********************************************************/
		/**
		 * This is the default constructor that sets up the sink
		 * with NO publishers, but ready to take on as many as you need.
		 */
		sink() :
			_name("sink"),
			_sources(),
			_mutex(),
			_online(true)
		{
		}


		/**
		 * This is the standard copy constructor and needs to be in every
		 * class to make sure that we don't have too many things running
		 * around.
		 */
		sink( const sink<T> & anOther ) :
			_name("sink"),
			_sources(),
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
		virtual ~sink()
		{
			// remove all the sources for this guy - no dangling pointers
			removeAllPublishers();
		}


		/**
		 * When we want to process the result of an equality we need to
		 * make sure that we do this right by always having an equals
		 * operator on all classes.
		 */
		sink<T> & operator=( const sink<T> & anOther )
		{
			/**
			 * Make sure that we don't do this to ourselves...
			 */
			if (this != & anOther) {
				// copy over the name - jsut to be complete
				_name = anOther._name;
				// for each of his sources, add them into my stuff
				BOOST_FOREACH( source<T> *s, anOther._sources ) {
					addToPublishers(s);
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
		 * This method allows the user to set the name of this sink
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
		 * This method gets the current name of this sink so that it
		 * can be logged, or used in whatever nammer desired. There is
		 * no other significance to the name.
		 */
		virtual const std::string & getName() const
		{
			return _name;
		}


		/**
		 * This method is called to add the provided source as a publisher
		 * to this sink's recv() method. It's a one-time registration,
		 * meaning, there's no way to register the same publisher twice
		 * if you mistakenly attempted to add yourself twice. If this is
		 * the first registration for this source, then a 'true' will be
		 * returned.
		 */
		virtual bool addToPublishers( source<T> *aSource )
		{
			bool		added = false;
			// first, make sure there's something to do
			if (aSource != NULL) {
				// next, see if we can add us as a sink to him
				if (aSource->addToSinks((const sink<T>*)this)) {
					// finally, make sure we can add him to us
					added = addToSources(aSource);
				}
			}
			return added;
		}


		/**
		 * This method attempts to un-register the provided source from
		 * the list of registered publishers for this sink. If the
		 * source is NOT a registered publisher, then this method will do
		 * nothing and return 'false'. Otherwise, the source will no longer
		 * send data to this instance, and a 'true' will be returned.
		 */
		virtual bool removeFromPublishers( source<T> *aSource )
		{
			bool		removed = false;
			// first, make sure we have something to do
			if (isSource(aSource)) {
				// drop me as a sink from him
				aSource->removeFromSinks(this);
				// now drop him from our list
				removeFromSources(aSource);
				// finally, say that we removed him
				removed = true;
			}
			return removed;
		}


		/**
		 * This method removes ALL the registered sources from the list
		 * for this sink. This effectively puts this sink "offline"
		 * for the time-being as there's nothing for him to do.
		 */
		virtual void removeAllPublishers()
		{
			// lock this up for running the removals
			boost::detail::spinlock::scoped_lock	lock(_mutex);
			// for each source, remove me as the sink
			BOOST_FOREACH( source<T> *s, _sources ) {
				if (s != NULL) {
					s->removeFromSinks((const sink<T>*)this);
				}
			}
			// at this point, we can drop all the sources as they are free
			_sources.clear();
		}


		/**
		 * This method, and it's convenience methods, are here to allow
		 * the user to leave the listener/subscriber connections in place,
		 * but shut down the flow of data from source to sink by taking
		 * this guy offline. This will simple make the recv() a no-op, and
		 * the sender won't know the difference. It's a clean way to simply
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
		 * This method is used to see if the sink is online or not.
		 * By default, it's online but it's possible that it's been taken
		 * offline for some reason, and this is a good way to find out.
		 */
		virtual bool isOnline() const
		{
			return (bool)_online;
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
		virtual bool recv( const T anItem )
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
			msg << "[sink '" << _name << "' w/ " << _sources.size() << " senders]";
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
			BOOST_FOREACH( source<T> *s, _sources ) {
				if (s != NULL) {
					boost::hash_combine(ans, s);
				}
			}
			boost::hash_combine(ans, (bool)_online);
			return ans;
		}


		/**
		 * This method checks to see if the two sinks are equal to one
		 * another based on the values they represent and *not* on the
		 * actual pointers themselves. If they are equal, then this method
		 * returns true, otherwise it returns false.
		 */
		bool operator==( const sink<T> & anOther ) const
		{
			bool		equals = false;
			if ((this == & anOther) ||
				((_name == anOther._name) &&
				 (_sources == anOther._sources) &&
				 (_online == anOther._online))) {
				equals = true;
			}
			return equals;
		}


		/**
		 * This method checks to see if the two sinks are not equal to
		 * one another based on the values they represent and *not* on the
		 * actual pointers themselves. If they are not equal, then this
		 * method returns true, otherwise it returns false.
		 */
		bool operator!=( const sink<T> & anOther ) const
		{
			return !operator==(anOther);
		}


	protected:
		friend class source<T>;

		/********************************************************
		 *
		 *              Non-Feedback Accessor Methods
		 *
		 ********************************************************/
		/**
		 * This method is used to actually add the source to the set of
		 * sources that we are tracking in this sink. This is not
		 * typically for general consumption as it's just doing half
		 * the job - the other is to let the source know I'm one of it's
		 * listeners.
		 */
		bool addToSources( const source<T> *aSource )
		{
			// lock this up for the POSSIBLE addition
			boost::detail::spinlock::scoped_lock	lock(_mutex);
			// if it doesn't exist, add it into the set
			return _sources.insert((source<T> *)aSource).second;
		}


		/**
		 * This method is used to actually remove the source from the set
		 * of sources that we are tracking. Again, this is ony half the
		 * battle, as we need to tell the source to remove us from it's
		 * list of listeners, but that's in the public API.
		 */
		void removeFromSources( const source<T> *aSource )
		{
			// lock this up for the POSSIBLE removal
			boost::detail::spinlock::scoped_lock	lock(_mutex);
			// erase it if it exists
			_sources.erase((source<T> *)aSource);
		}


		/**
		 * This method is used to actually clear out all the sources we
		 * have in the set, but it's assumed that they have been told
		 * to remove us as a listener as well.
		 */
		void removeAllSources()
		{
			// lock this up for the clearing
			boost::detail::spinlock::scoped_lock	lock(_mutex);
			_sources.clear();
		}


		/**
		 * This method looks at the current list of sources and returns
		 * 'true' if the provided source is in the registered list. This
		 * is the thread-safe way to know if a given source is in the list.
		 */
		bool isSource( const source<T> *aSource )
		{
			// lock this up for the check
			boost::detail::spinlock::scoped_lock	lock(_mutex);
			return ((aSource != NULL) && (_sources.find((source<T> *)aSource) != _sources.end()));
		}


	private:
		/**
		 * There will be times that naming the sources will be very useful.
		 * For this reason, we'll have a name here - initially just "source",
		 * but the user can change this as they see fit. It's just a useful
		 * thing to have when dealng with a great number of sources.
		 */
		std::string							_name;
		/**
		 * This is the set of all the sources we know about. They will
		 * be added to in the protected accessor methods, and the public
		 * API will do that, and the feedback part of making sure the
		 * sources know that we are one of their listeners.
		 */
		boost::unordered_set< source<T> * >	_sources;
		// ...and a spinlock to protect the list
		boost::detail::spinlock				_mutex;
		/**
		 * We can take this sink offline, and have it "idle", but
		 * in order to do that, we need a nice boolean that we can
		 * flip without any threading issues. This is it.
		 */
		abool								_online;
};
}		// end of namespace dkit

#endif		// __DKIT_SINK_H
