/**
 * source.cpp - this file implements the general 'source' of an object. This is
 *              all about templates, so what that 'object' is depends on the
 *              user, but the idea is that this class is the base class for all
 *              things that GENERATE instances of class T INTO the process
 *              space. There is a simple listener pattern set up between the
 *              sources and the sinks - the sources have a send() method and
 *              the sinks have an recv() method and the send() sends the one
 *              instance of T to all registered listeners by calling their
 *              recv() method. It's pretty simple. The goal is to make this
 *              the foundation for a more general message passing architecture.
 */

//	System Headers
#include <sstream>

//	Third-Party Headers
#include <boost/foreach.hpp>

//	Other Headers
#include "sink.h"
#include "source.h"

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
 * This is the default constructor that sets up the source
 * with NO listeners, but ready to take on as many as you need.
 */
template <class T> source<T>::source() :
	mName("source"),
	mSinks(),
	mMutex(),
	mOnline(true)
{
}


/**
 * This is the standard copy constructor and needs to be in every
 * class to make sure that we don't have too many things running
 * around.
 */
template <class T> source<T>::source( const source<T> & anOther ):
	mName("source"),
	mSinks(),
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
template <class T> source<T>::~source()
{
	// remove all the listeners of this guy - no dangling pointers
	removeAllListeners();
}


/**
 * When we want to process the result of an equality we need to
 * make sure that we do this right by always having an equals
 * operator on all classes.
 */
template <class T> source<T> & source<T>::operator=( const source<T> & anOther )
{
	/**
	 * Make sure that we don't do this to ourselves...
	 */
	if (this != & anOther) {
		// copy over the name - jsut to be complete
		mName = anOther.mName;
		// for each of his sinks, add them into my stuff
		BOOST_FOREACH( sink<T> *s, anOther.mSinks ) {
			addToListeners(s);
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
 * This method allows the user to set the name of this source
 * for debugging and tracking purposes. It's not required, and
 * there's no requirement that the names are unique, it's just
 * a convenience that can be quite useful at times.
 */
template <class T> void source<T>::setName( const std::string & aName )
{
	boost::detail::spinlock::scoped_lock	lock(mMutex);
	mName = aName;
}


/**
 * This method gets the current name of this source so that it
 * can be logged, or used in whatever nammer desired. There is
 * no other significance to the name.
 */
template <class T> const std::string & source<T>::getName() const
{
	return mName;
}


/**
 * This method is called to add the provided sink as a listener
 * to this source's list of targets for the send() method. It's
 * a one-time registration, meaning, there's no way to get the
 * same message twice if you mistakenly attempted to add yourself
 * twice. If this is the first registration for this sink, then
 * a 'true' will be returned.
 */
template <class T> bool source<T>::addToListeners( sink<T> *aSink )
{
	bool		added = false;
	// first, make sure there's something to do
	if (aSink != NULL) {
		// next, see if we can add us as a source to him
		if (aSink->addToSources(this)) {
			// finally, make sure we can add him to us
			added = addToSinks(aSink);
		}
	}
	return added;
}


/**
 * This method attempts to un-register the provided sink from
 * the list of registered listeners for this source. If the
 * sink is NOT a registered listener, then this method will do
 * nothing and return 'false'. Otherwise, the sink will no longer
 * receive calls, and a 'true' will be returned.
 */
template <class T> bool source<T>::removeFromListeners( sink<T> *aSink )
{
	bool		removed = false;
	// first, make sure we have something to do
	if (isSink(aSink)) {
		// drop me as a source from him
		aSink->removeFromSources(this);
		// now drop him from our list
		removeFomSinks(aSink);
		// finally, say that we removed him
		removed = true;
	}
	return removed;
}


/**
 * This method removes ALL the registered sinks from the list
 * for this source. This effectively puts this source "offline"
 * for the time-being as there's nothing for him to do.
 */
template <class T> void source<T>::removeAllListeners()
{
	// lock this up for running the removals
	boost::detail::spinlock::scoped_lock	lock(mMutex);
	// for each sink, remove me as the source
	BOOST_FOREACH( sink<T> *s, mSinks ) {
		if (s != NULL) {
			s->removeFromSources(this);
		}
	}
	// at this point, we can drop all the sinks as they are free
	mSinks.clear();
}


/**
 * This method, and it's convenience methods, are here to allow
 * the user to leave the listener/subscriber connections in place,
 * but shut down the flow of data from source to sink by taking
 * this guy offline. This will simple make the send() a no-op, and
 * the caller won't know the difference. It's a clean way to simply
 * stop the flow of data.
 */
template <class T> void source<T>::setOnline( bool aFlag )
{
	mOnline = aFlag;
}


template <class T> void source<T>::takeOnline()
{
	mOnline = true;
}


template <class T> void source<T>::takeOffline()
{
	mOnline = false;
}


/**
 * This method is used to see if the source is online or not.
 * By default, it's online but it's possible that it's been taken
 * offline for some reason, and this is a good way to find out.
 */
template <class T> bool source<T>::isOnline() const
{
	return (bool)mOnline;
}


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
template <class T> bool source<T>::send( const T anItem )
{
	bool		ok = true;
	if (mOnline) {
		// lock this up for running the sends
		boost::detail::spinlock::scoped_lock	lock(mMutex);
		// for each sink, send them the item and let them use it
		BOOST_FOREACH( sink<T> *s, mSinks ) {
			if (s != NULL) {
				if (!s->recv(anItem)) {
					ok = false;
				}
			}
		}
	}
	return ok;
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
template <class T> std::string source<T>::toString() const
{
	std::ostringstream	msg;
	msg << "[source '" << mName << "' w/ " << mSinks.size() << " sinks]";
	return msg.str();
}


/**
 * This method checks to see if the two sources are equal to one
 * another based on the values they represent and *not* on the
 * actual pointers themselves. If they are equal, then this method
 * returns true, otherwise it returns false.
 */
template <class T> bool source<T>::operator==( const source<T> & anOther ) const
{
	bool		equals = false;
	if ((this == & anOther) ||
		((mName == anOther.mName) &&
		 (mSinks == anOther.mSinks) &&
		 (mOnline == anOther.mOnline))) {
		equals = true;
	}
	return equals;
}


/**
 * This method checks to see if the two sources are not equal to
 * one another based on the values they represent and *not* on the
 * actual pointers themselves. If they are not equal, then this
 * method returns true, otherwise it returns false.
 */
template <class T> bool source<T>::operator!=( const source<T> & anOther ) const
{
	return !operator==(anOther);
}


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
template <class T> bool source<T>::addToSinks( sink<T> *aSink )
{
	// lock this up for the POSSIBLE addition
	boost::detail::spinlock::scoped_lock	lock(mMutex);
	// if it doesn't exist, add it into the set
	return mSinks.insert(aSink).second;
}


/**
 * This method is used to actually remove the sink from the set
 * of sinks that we are tracking. Again, this is ony half the
 * battle, as we need to tell the sink to remove us from it's
 * list of subscribers, but that's in the public API.
 */
template <class T> void source<T>::removeFromSinks( sink<T> *aSink )
{
	// lock this up for the POSSIBLE removal
	boost::detail::spinlock::scoped_lock	lock(mMutex);
	// erase it if it exists
	mSinks.erase(aSink);
}


/**
 * This method is used to actually clear out all the sinks we
 * have in the set, but it's assumed that they have been told
 * to remove us as a publisher as well.
 */
template <class T> void source<T>::removeAllSinks()
{
	// lock this up for the clearing
	boost::detail::spinlock::scoped_lock	lock(mMutex);
	mSinks.clear();
}


/**
 * This method looks at the current list of sinks and returns
 * 'true' if the provided sink is in the registered list. This
 * is the thread-safe way to know if a given sink is in the list.
 */
template <class T> bool source<T>::isSink( sink<T> *aSink )
{
	// lock this up for the check
	boost::detail::spinlock::scoped_lock	lock(mMutex);
	return ((aSink != NULL) && (mSinks.find(aSink) != mSinks.end()));
}
}		// end of namespace dkit
