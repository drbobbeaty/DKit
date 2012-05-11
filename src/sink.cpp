/**
 * sink.cpp - this file implements the general 'sink' of an object. This is
 *            all about templates, so what that 'object' is depends on the
 *            user, but the idea is that this class is the base class for all
 *            things that CONSUME instances of class T OUT OF the process
 *            space. There is a simple listener pattern set up between the
 *            sources and the sinks - the sources have a send() method and
 *            the sinks have an recv() method and the send() sends the one
 *            instance of T to all registered listeners by calling their
 *            recv() method. It's pretty simple. The goal is to make this
 *            the foundation for a more general message passing architecture.
 */

//	System Headers
#include <sstream>

//	Third-Party Headers
#include <boost/foreach.hpp>

//	Other Headers
#include "source.h"
#include "sink.h"

//	Forward Declarations

//	Private Constants

//	Private Datatypes

//	Private Data Constants


namespace dkit {
/********************************************************
 *
 *                Constructors/Destructor
 *
 ********************************************************/
/**
 * This is the default constructor that sets up the sink
 * with NO publishers, but ready to take on as many as you need.
 */
template <class T> sink<T>::sink() :
	mName("sink"),
	mSources(),
	mMutex(),
	mOnline(true)
{
}


/**
 * This is the standard copy constructor and needs to be in every
 * class to make sure that we don't have too many things running
 * around.
 */
template <class T> sink<T>::sink( const sink<T> & anOther ) :
	mName("sink"),
	mSources(),
	mMutex(),
	mOnline(true)
{
	// let the '=' operator do all the heavy lifting
	*this = anOther;
}


/**
 * This is the standard destructor and needs to be virtual to make
 * sure that if we subclass off this the right destructor will be
 * called.
 */
template <class T> sink<T>::~sink()
{
	// remove all the sources for this guy - no dangling pointers
	removeAllPublishers();
}


/**
 * When we want to process the result of an equality we need to
 * make sure that we do this right by always having an equals
 * operator on all classes.
 */
template <class T> sink<T> & sink<T>::operator=( const sink<T> & anOther )
{
	/**
	 * Make sure that we don't do this to ourselves...
	 */
	if (this != & anOther) {
		// copy over the name - jsut to be complete
		mName = anOther.mName;
		// for each of his sources, add them into my stuff
		BOOST_FOREACH( source<T> *s, anOther.mSources ) {
			addToPublishers(s);
		}
		// now just copy over the online status
		mOnline = anOther.mOnline;
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
template <class T> void sink<T>::setName( const std::string & aName )
{
	boost::detail::spinlock::scoped_lock	lock(mMutex);
	mName = aName;
}


/**
 * This method gets the current name of this sink so that it
 * can be logged, or used in whatever nammer desired. There is
 * no other significance to the name.
 */
template <class T> const std::string & sink<T>::getName() const
{
	return mName;
}


/**
 * This method is called to add the provided source as a publisher
 * to this sink's recv() method. It's a one-time registration,
 * meaning, there's no way to register the same publisher twice
 * if you mistakenly attempted to add yourself twice. If this is
 * the first registration for this source, then a 'true' will be
 * returned.
 */
template <class T> bool sink<T>::addToPublishers( source<T> *aSource )
{
	bool		added = false;
	// first, make sure there's something to do
	if (aSource != NULL) {
		// next, see if we can add us as a sink to him
		if (aSource->addToSinks(this)) {
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
template <class T> void sink<T>::removeFromPublishers( source<T> *aSource )
{
	bool		removed = false;
	// first, make sure we have something to do
	if (isSource(aSource)) {
		// drop me as a sink from him
		aSource->removeFromSinks(this);
		// now drop him from our list
		removeFomSources(aSource);
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
template <class T> void sink<T>::removeAllPublishers()
{
	// lock this up for running the removals
	boost::detail::spinlock::scoped_lock	lock(mMutex);
	// for each source, remove me as the sink
	BOOST_FOREACH( source<T> *s, mSources ) {
		if (s != NULL) {
			s->removeFromSinks(this);
		}
	}
	// at this point, we can drop all the sources as they are free
	mSources.clear();
}


/**
 * This method, and it's convenience methods, are here to allow
 * the user to leave the listener/subscriber connections in place,
 * but shut down the flow of data from source to sink by taking
 * this guy offline. This will simple make the recv() a no-op, and
 * the sender won't know the difference. It's a clean way to simply
 * stop the flow of data.
 */
template <class T> void sink<T>::setOnline( bool aFlag )
{
	mOnline = aFlag;
}


template <class T> void sink<T>::takeOnline()
{
	mOnline = true;
}


template <class T> void sink<T>::takeOffline()
{
	mOnline = false;
}


/**
 * This method is used to see if the sink is online or not.
 * By default, it's online but it's possible that it's been taken
 * offline for some reason, and this is a good way to find out.
 */
template <class T> bool sink<T>::isOnline() const
{
	return (bool)mOnline;
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
template <class T> bool sink<T>::recv( const T anItem )
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
template <class T> std::string sink<T>::toString() const
{
	std::ostringstream	msg;
	msg << "[sink '" << mName << "' w/ " << mSources.size() << " senders]";
	return msg.str();
}


/**
 * This method checks to see if the two sinks are equal to one
 * another based on the values they represent and *not* on the
 * actual pointers themselves. If they are equal, then this method
 * returns true, otherwise it returns false.
 */
template <class T> bool sink<T>::operator==( const sink<T> & anOther ) const
{
	bool		equals = false;
	if ((this == & anOther) ||
		((mName == anOther.mName) &&
		 (mSources == anOther.mSources) &&
		 (mOnline == anOther.mOnline))) {
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
template <class T> bool sink<T>::operator!=( const sink<T> & anOther ) const
{
	return !operator==(anOther);
}


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
template <class T> bool sink<T>::addToSources( source<T> *aSource )
{
	// lock this up for the POSSIBLE addition
	boost::detail::spinlock::scoped_lock	lock(mMutex);
	// if it doesn't exist, add it into the set
	return mSources.insert(aSource).second;
}


/**
 * This method is used to actually remove the source from the set
 * of sources that we are tracking. Again, this is ony half the
 * battle, as we need to tell the source to remove us from it's
 * list of listeners, but that's in the public API.
 */
template <class T> void sink<T>::removeFromSources( source<T> *aSource )
{
	// lock this up for the POSSIBLE removal
	boost::detail::spinlock::scoped_lock	lock(mMutex);
	// erase it if it exists
	mSources.erase(aSource);
}


/**
 * This method is used to actually clear out all the sources we
 * have in the set, but it's assumed that they have been told
 * to remove us as a listener as well.
 */
template <class T> void sink<T>::removeAllSources()
{
	// lock this up for the clearing
	boost::detail::spinlock::scoped_lock	lock(mMutex);
	mSources.clear();
}


/**
 * This method looks at the current list of sources and returns
 * 'true' if the provided source is in the registered list. This
 * is the thread-safe way to know if a given source is in the list.
 */
template <class T> bool sink<T>::isSource( source<T> *aSource )
{
	// lock this up for the check
	boost::detail::spinlock::scoped_lock	lock(mMutex);
	return ((aSource != NULL) && (mSources.find(aSource) != mSources.end()));
}
}		// end of namespace dkit
