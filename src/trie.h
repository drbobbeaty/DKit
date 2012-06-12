/**
 * trie.h - this file defines a 64-bit keyed trie organized as 8 levels
 *          each level having up to 256 branches (8-bits). The structure
 *          of a trie is built as-needed by adding elements to the
 *          structure, and while it's possible to remove elements from
 *          the trie, the majority of the storage space is in the branches
 *          and leaves, and that's not going to be reclaimed until the
 *          trie itself is completely cleared out.
 *
 *          The 64-bit key value needs to be provided by a function called:
 *
 *            uint64_t key_value( const T & t );
 *
 *          and just needs to be defined for the 'T' that you are using.
 */
#ifndef __DKIT_TRIE_H
#define __DKIT_TRIE_H

//	System Headers
#include <stdint.h>
#include <ostream>
#include <sstream>
#include <string>
#include <stdexcept>

//	Third-Party Headers
#include <boost/smart_ptr/detail/spinlock.hpp>
#include <boost/type_traits/is_pointer.hpp>

//	Other Headers
#include "abool.h"

//	Forward Declarations

//	Public Constants

//	Public Datatypes

//	Public Data Constants
/**
 * We need to have a simple enum for the different "types" of queues that
 * we can use for the pool - all based on the complexity of the access. This
 * is meant to allow the user to have complete flexibility in how to ask for,
 * and recycle items from the pool.
 */
namespace dkit {
enum trie_key_size {
	uint16_key = 2,
	uint32_key = 4,
	uint64_key = 8,
	uint128_key = 16,
};
}		// end of namespace dkit


/**
 * Main class definition
 */
namespace dkit {
template <class T, trie_key_size N> class trie
{
	public:
		/********************************************************
		 *
		 *           Component Classes for Trie
		 *
		 ********************************************************/
		/**
		 * One of the main components of the trie is the Node that sits in
		 * the leaf classes of the tree in the trie. This Node class handles
		 * all the storage and clean-up of the data in the Nodes, and the
		 * tree in the trie handles all the branches. It's protected as some
		 * subclasses may wish to use it.
		 */
		struct Node {
			volatile T							value;
			abool								valid;
			mutable boost::detail::spinlock		mutex;

			/**
			 * These are the constructors and destructor for the Node
			 */
			Node() : value(), valid(false), mutex() { }
			Node( const T aValue ) : value(aValue), valid(true), mutex() { }
			~Node() { clear(); }

			// this method is the simple clearing out of the value
			void clear()
			{
				if (boost::is_pointer<T>::value) {
					// we have to CAS in a NULL to the value
					T	old = value;
					while (!__sync_bool_compare_and_swap(&value, old, NULL)) {
						old = value;
					}
					// ...and then clean up what we took out
					if (old != NULL) {
						delete old;
					}
				}
				valid = false;
			}

			/**
			 * This takes care of placing a value into the Node in as
			 * efficient a way as possible. For pointers, it's CAS, but
			 * for non-pointers, it's a spinlock to protect the assignment.
			 */
			void assign( const T & t )
			{
				if (boost::is_pointer<T>::value) {
					// do a CAS on the old to new value of this node
					T	old = value;
					while (!__sync_bool_compare_and_swap(&value, old, t)) {
						old = value;
					}
					// if we have a valid old pointer, then delete it
					if (valid && (old != NULL)) {
						delete old;
					}
				} else {
					boost::detail::spinlock::scoped_lock	lock(mutex);
					// save the value as it's a simple data type
					value = t;
				}
				// make sure that we are considering this node valid
				valid = true;
			}

			/**
			 * This takes care of pulling a value out of the Node in as
			 * efficient a way as possible. For pointers, it's CAS, but
			 * for non-pointers, it's a spinlock to protect the assignment.
			 * If there's a value to get, it's copied and 'true' is returned.
			 * If not, then the arg is left untouched, and 'false' is
			 * returned.
			 */
			bool copy( T & t )
			{
				bool		success = false;
				if ((bool)valid) {
					if (boost::is_pointer<T>::value) {
						// CAS out the value after a no-op or-ing.
						t = __sync_or_and_fetch(&value, 0x0);
					} else {
						boost::detail::spinlock::scoped_lock	lock(mutex);
						t = value;
					}
					success = true;
				}
				return success;
			}

			/**
			 * This takes care of pulling a value out of the Node in as
			 * efficient a way as possible, and then invalidating the Node's
			 * contents as if the value is being "removed". For pointers,
			 * it's CAS, but for non-pointers, it's a spinlock to protect
			 * the assignment. If there's a value to get, it's copied and
			 * 'true' is returned. If not, then the arg is left untouched,
			 * and 'false' is returned.
			 */
			bool remove( T & t )
			{
				bool		success = false;
				if ((bool)valid) {
					if (boost::is_pointer<T>::value) {
						// CAS out the value after a no-op or-ing.
						t = value;
						while (!__sync_bool_compare_and_swap(&value, t, NULL)) {
							t = value;
						}
					} else {
						boost::detail::spinlock::scoped_lock	lock(mutex);
						t = value;
					}
					success = true;
					// make sure that we are considering this node invalid
					valid = false;
				}
				return success;
			}

			/**
			 * This method is just a simple debugging tool to be able
			 * to see the value within in the Node at the time.
			 */
			std::string toString() const
			{
				std::ostringstream msg;
				if ((bool)valid) {
					msg << "[" << value << "]";
				} else {
					msg << "[invalid]";
				}
				return msg.str();
			}
		};

		/********************************************************
		 *
		 *            Base Functor Classes for Trie
		 *
		 ********************************************************/
		/**
		 * This base functor class will be subclassed by the user to
		 * operate on a Node, and gather, modify, or manipulate the
		 * Node as necessary. The method to implement is process(),
		 * and it will be called by the trie for each valid Node
		 * in the trie - each of which will contain a value.
		 *
		 * If the process() method returns a 'false', then the
		 * processing of additional values will STOP. This is a simple
		 * 'while'-like condition, and can be used to detect when
		 * enough is enough. If all values need to be processed, then
		 * it's important to make sure that process() ALWAYS returns
		 * 'true'.
		 */
		class functor
		{
			public:
				// the simple operator for standard functor usage
				bool operator()( volatile Node & aNode ) { return process(aNode); }
				// ...and the method most often implemented in subclasses
				virtual bool process( volatile Node & aNode ) = 0;
		};

	protected:
		/**
		 * The base component for the structure of the trie is called a
		 * 'Component', and it will be sub-classed into a Branch and a
		 * Leaf. The reason for this base class is to establish an API
		 * that makes creating and navigating the structure easy.
		 */
		struct Component {
			/**
			 * These are the constructors and destructor for the Branch
			 */
			Component() { }
			virtual ~Component() { }

			/**
			 * These are all the utility methods that we'll need for
			 * each of the components - regardless of the specifics
			 * of it's implementation.
			 */
			// this method returns 'true' if the Component is empty
			virtual bool empty() { return true; }
			// this method counts all the valid values in this Component
			virtual size_t size() { return 0; }
			// this method is the simple clearing out of the value
			virtual void clear() { }

			/**
			 * These methods walk down the trie's tree based on the path
			 * of bytes and the step in that path. They do the walking and
			 * constructing, as needed, based on what the class is that
			 * is implementing these methods.
			 */
			virtual volatile Node *getNodeForKey( const uint8_t aKey[], uint16_t aStep ) { return NULL; }
			virtual volatile Node *getOrCreateNodeForKey( const uint8_t aKey[], uint16_t aStep ) { return NULL; }
			/**
			 * This method walks the trie's path to the Nodes and then
			 * applies the provided functor to each of the valid nodes.
			 */
			virtual bool apply( functor & aFunctor ) { return true; }

			/**
			 * These are the standard equality and inequality operators.
			 */
			bool operator==( const Component & anOther ) const { return false; }
			bool operator!=( const Component & anOther ) const { return !operator==(anOther); }
		};


		/**
		 * The next big component of the trie is the Leaf node in the tree
		 * where there exist the last byte of the key - 1 of 256 values,
		 * for the location of the Node. There will be one of these Leafs
		 * created for each path that leads us to this point in the tree,
		 * and while it's possible to have the nodes here as pointers, it's
		 * really not saving a lot of space as the vast majority of the
		 * time, we'll be storing pointers, and the size of a Node is very
		 * close to a pointer, and so the savings just isn't that great.
		 */
		struct Leaf : public Component {
			volatile Node		nodes[256];

			/**
			 * These are the constructors and destructor for the Leaf
			 */
			Leaf() : Component(), nodes() { }
			virtual ~Leaf() { clear(); }

			/**
			 * These are all the utility methods that we'll need for
			 * each of the components - regardless of the specifics
			 * of it's implementation.
			 */
			// this method returns 'true' if the Component is empty
			virtual bool empty()
			{
				bool		vacant = true;
				for (uint16_t i = 0; i < 256; ++i) {
					if (const_cast<Node &>(nodes[i]).valid) {
						vacant = false;
						break;
					}
				}
				return vacant;
			}

			// this method counts all the valid values in this Component
			virtual size_t size()
			{
				size_t		sz = 0;
				for (uint16_t i = 0; i < 256; ++i) {
					if (const_cast<Node &>(nodes[i]).valid) {
						++sz;
					}
				}
				return sz;
			}

			// this method is the simple clearing out of the value
			virtual void clear()
			{
				for (uint16_t i = 0; i < 256; ++i) {
					const_cast<Node &>(nodes[i]).clear();
				}
			}

			/**
			 * These methods walk down the trie's tree based on the path
			 * of bytes and the step in that path. They do the walking and
			 * constructing, as needed, based on what the class is that
			 * is implementing these methods.
			 */
			virtual volatile Node *getNodeForKey( const uint8_t aKey[], uint16_t aStep )
			{
				/**
				 * For the Leaf, we just need to return the Node as it's
				 * already allocated at this level. It may be empty, but
				 * it's there, and all we need to do is find the right one
				 * and return it's pointer.
				 */
				return &(nodes[aKey[aStep]]);
			}

			virtual volatile Node *getOrCreateNodeForKey( const uint8_t aKey[], uint16_t aStep )
			{
				/**
				 * For the Leaf, we just need to return the Node as it's
				 * already allocated at this level. It may be empty, but
				 * it's there, and all we need to do is find the right one
				 * and return it's pointer.
				 */
				return &(nodes[aKey[aStep]]);
			}

			/**
			 * This method walks the trie's path to the Nodes and then
			 * applies the provided functor to each of the valid nodes.
			 */
			virtual bool apply( functor & aFunctor )
			{
				bool		error = false;
				for (uint16_t i = 0; i < 256; ++i) {
					if ((bool)const_cast<Node &>(nodes[i]).valid && !aFunctor(nodes[i])) {
						error = true;
						break;
					}
				}
				return !error;
			}

			/**
			 * These are the standard equality and inequality operators.
			 */
			bool operator==( const Leaf & anOther ) const
			{
				bool		equals = false;
				bool		keepChecking = true;

				// first, see if we are checking against ourselves...
				if (keepChecking) {
					if (this == & anOther) {
						equals = true;
						keepChecking = false;
					}
				}

				// next, check each of the Nodes...
				if (keepChecking) {
					// assume it's equal, and verify each branch
					equals = true;
					for (uint16_t i = 0; keepChecking && (i < 256); ++i) {
						if (nodes[i] != anOther.nodes[i]) {
							equals = false;
							keepChecking = false;
						}
					}
				}

				return equals;
			}

			bool operator!=( const Leaf & anOther ) const
			{
				return !operator==(anOther);
			}
		};


		/**
		 * The final component for the trie is the branch of the tree to
		 * get through the path of bytes to the Leaf node where we have
		 * the last byte decoded, and then we have access to the actual
		 * value pointed to by the key.
		 */
		struct Branch : public Component {
			Component	*kids[256];

			/**
			 * These are the constructors and destructor for the Branch
			 */
			Branch() : Component(), kids() { }
			virtual ~Branch() { clear(); }

			/**
			 * These are all the utility methods that we'll need for
			 * each of the components - regardless of the specifics
			 * of it's implementation.
			 */
			// this method returns 'true' if the Branch is empty
			virtual bool empty()
			{
				bool		vacant = true;
				for (uint16_t i = 0; i < 256; ++i) {
					if ((kids[i] != NULL) && !kids[i]->empty()) {
						vacant = false;
						break;
					}
				}
				return vacant;
			}

			// this method counts all the valid values in this branch
			virtual size_t size()
			{
				size_t		sz = 0;
				for (uint16_t i = 0; i < 256; ++i) {
					if (kids[i] != NULL) {
						sz += kids[i]->size();
					}
				}
				return sz;
			}

			// this method is the simple clearing out of the value
			virtual void clear()
			{
				for (uint16_t i = 0; i < 256; ++i) {
					if (kids[i] != NULL) {
						delete kids[i];
						kids[i] = NULL;
					}
				}
			}

			/**
			 * These methods walk down the trie's tree based on the path
			 * of bytes and the step in that path. They do the walking and
			 * constructing, as needed, based on what the class is that
			 * is implementing these methods.
			 */
			virtual volatile Node *getNodeForKey( const uint8_t aKey[], uint16_t aStep )
			{
				volatile Node	*n = NULL;
				// see if the root branch is available, and work with that
				Component	*c = kids[aKey[aStep]];
				if (c != NULL) {
					n = c->getNodeForKey(aKey, (aStep + 1));
				}
				// return what we have dug out of the tree
				return n;
			}

			virtual volatile Node *getOrCreateNodeForKey( const uint8_t aKey[], uint16_t aStep )
			{
				volatile Node	*n = NULL;

				// get the index we're working on (re-used a few times)
				uint8_t		idx = aKey[aStep];
				Component	*curr = __sync_or_and_fetch(&kids[idx], 0x0);
				if (curr == NULL) {
					// create a new Branch or Leaf for this part of the trie
					bool	createBranch = true;
					if (aStep < eLastBranch) {
						curr = new Branch();
					} else {
						curr = new Leaf();
						createBranch = false;
					}
					// throw a runtime exception if we couldn't make it
					if (curr == NULL) {
						if (createBranch) {
							throw std::runtime_error("[Branch::getOrCreateNodeForKey] Unable to create new Branch for the trie!");
						} else {
							throw std::runtime_error("[Branch::getOrCreateNodeForKey] Unable to create new Leaf for the trie!");
						}
					}
					// see if we can put this new one in the right place
					if (!__sync_bool_compare_and_swap(&kids[idx], NULL, curr)) {
						// someone beat us to it! Delete what we just made...
						delete curr;
						// ...and get what is there now
						curr = __sync_or_and_fetch(&kids[idx], 0x0);
					}
				}

				// now pass down to that next branch the request to fill
				if (curr != NULL) {
					n = curr->getOrCreateNodeForKey(aKey, (aStep + 1));
				}

				// return what we have dug out of the tree
				return n;
			}

			/**
			 * This method walks the trie's path to the Nodes and then
			 * applies the provided functor to each of the valid nodes.
			 */
			virtual bool apply( functor & aFunctor )
			{
				bool		error = false;
				for (uint16_t i = 0; i < 256; ++i) {
					if ((kids[i] != NULL) && !(kids[i]->apply(aFunctor))) {
						error = true;
						break;
					}
				}
				return !error;
			}

			/**
			 * These are the standard equality and inequality operators.
			 */
			bool operator==( const Branch & anOther ) const
			{
				bool		equals = false;
				bool		keepChecking = true;

				// first, see if we are checking against ourselves...
				if (keepChecking) {
					if (this == & anOther) {
						equals = true;
						keepChecking = false;
					}
				}

				// next, check each of the branches...
				if (keepChecking) {
					// assume it's equal, and verify each branch
					equals = true;
					for (uint16_t i = 0; keepChecking && (i < 256); ++i) {
						if (kids[i] == NULL) {
							if (anOther.kids[i] != NULL) {
								equals = false;
								keepChecking = false;
							}
						} else {
							if (anOther.kids[i] == NULL) {
								equals = false;
								keepChecking = false;
							} else if (*(kids[i]) != *(anOther.kids[i])) {
								equals = false;
								keepChecking = false;
							}
						}
					}
				}

				return equals;
			}

			bool operator!=( const Branch & anOther ) const
			{
				return !operator==(anOther);
			}
		};


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
		trie() :
			_roots()
		{
		}


		/**
		 * This is the standard copy constructor and needs to be in every
		 * class to make sure that we don't have too many things running
		 * around.
		 */
		trie( const trie<T,N> & anOther ) :
			_roots()
		{
			// let the '=' operator do all the heavy lifting
			*this = anOther;
		}


		/**
		 * This is the standard destructor and needs to be virtual to make
		 * sure that if we subclass off this the right destructor will be
		 * called.
		 */
		virtual ~trie()
		{
			// simply clear things out...
			clear();
		}


		/**
		 * When we want to process the result of an equality we need to
		 * make sure that we do this right by always having an equals
		 * operator on all classes.
		 */
		trie<T,N> & operator=( const trie<T,N> & anOther )
		{
			/**
			 * Make sure that we don't do this to ourselves...
			 */
			if (this != & anOther) {
				/**
				 * For now there's just no good way to allow one
				 * trie to be equated to another. If the values are
				 * pointers, there's no way to duplicate the contents
				 * without a shallow copy, and that's going to leak.
				 * So, until we solve this, just ignore it.
				 */
			}
			return *this;
		}


		/********************************************************
		 *
		 *                Accessor Methods
		 *
		 ********************************************************/
		/**
		 * This method takes the provided value, and along with the
		 * key_value( const T & ) function, will place this value
		 * into the trie, possibly replacing the value that may already
		 * be there. If something already exists there, it will be
		 * removed/dropped/deleted as part of the clean-up of this
		 * trie. If you want to know if something is there, use the
		 * get() method.
		 */
		bool put( const T & aValue )
		{
			bool			success = false;
			volatile Node	*n = getOrCreateNodeForKey(key_value(aValue));
			if (n != NULL) {
				const_cast<Node *>(n)->assign(aValue);
				success = true;
			}
			return success;
		}


		/**
		 * This method adds the value to the trie, based on it's
		 * key_value(), but the return value is a little different
		 * from the put(). This method will return 'true' if there
		 * already exists a valid value at this key, or 'false'
		 * if this is a new value at that key. The point is that it
		 * is telling the caller if it's an update (true), or an
		 * insert (false).
		 */
		bool upsert( const T & aValue )
		{
			bool			update = false;
			volatile Node	*n = getOrCreateNodeForKey(key_value(aValue));
			if (n != NULL) {
				update = const_cast<Node *>(n)->valid;
				const_cast<Node *>(n)->assign(aValue);
			}
			return update;
		}


		/**
		 * This method will attempt to find a value for the supplied
		 * key in the trie. If it is successful, a copy will be made,
		 * and placed in the 'aValue' argument and a 'true' returned.
		 * If not, then 'aValue' will be unchanged, and a 'false' will
		 * be returned.
		 */
		bool get( uint16_t aKey, T & aValue )
		{
			return get((uint8_t *)&aKey, aValue);
		}
		bool get( uint32_t aKey, T & aValue )
		{
			return get((uint8_t *)&aKey, aValue);
		}
		bool get( uint64_t aKey, T & aValue )
		{
			return get((uint8_t *)&aKey, aValue);
		}
		bool get( const uint8_t aKey[], T & aValue )
		{
			bool			success = false;
			volatile Node	*n = getNodeForKey(aKey);
			if (n != NULL) {
				success = const_cast<Node *>(n)->copy(aValue);
			}
			return success;
		}


		/**
		 * This method will attempt to find a value for the supplied
		 * key in the trie, and then remove it and return it to the
		 * caller. If it is successful, a 'true' will be returned. This
		 * is only the case if the key existed, and we were able to pull
		 * the value into the supplied reference. If not, then a 'false'
		 * will be returned. In the case of a pointer value, the memory
		 * management for the returned value will become the responsibility
		 * of the caller as the trie will pass control back to it.
		 */
		bool remove( uint16_t aKey, T & aValue )
		{
			return remove((uint8_t *)&aKey, aValue);
		}
		bool remove( uint32_t aKey, T & aValue )
		{
			return remove((uint8_t *)&aKey, aValue);
		}
		bool remove( uint64_t aKey, T & aValue )
		{
			return remove((uint8_t *)&aKey, aValue);
		}
		bool remove( const uint8_t aKey[], T & aValue )
		{
			bool			success = false;
			volatile Node	*n = getNodeForKey(aKey);
			if (n != NULL) {
				success = const_cast<Node *>(n)->remove(aValue);
			}
			return success;
		}


		/**
		 * This method will attempt to find a value for the supplied
		 * key in the trie, and then clear it. If it is successful,
		 * a 'true' will be returned. This is only the case if the
		 * key existed. If not, then a 'false' will be returned. The
		 * value at the key will be disposed of by the trie.
		 */
		bool clear( uint16_t aKey )
		{
			return clear((uint8_t *)&aKey);
		}
		bool clear( uint32_t aKey )
		{
			return clear((uint8_t *)&aKey);
		}
		bool clear( uint64_t aKey )
		{
			return clear((uint8_t *)&aKey);
		}
		bool clear( const uint8_t aKey[] )
		{
			bool			success = false;
			volatile Node	*n = getNodeForKey(aKey);
			if (n != NULL) {
				success = true;
				const_cast<Node *>(n)->clear();
			}
			return success;
		}


		/**
		 * This method returns 'true' if the provided 64-bit key
		 * references a valid value 'T' in the trie at this time.
		 * Since the trie is lockless, it's possible that right
		 * after this call, the value is removed, so some care
		 * should be taken in interpreting the results.
		 */
		bool exists( uint16_t aKey )
		{
			return exists((uint8_t *)&aKey);
		}
		bool exists( uint32_t aKey )
		{
			return exists((uint8_t *)&aKey);
		}
		bool exists( uint64_t aKey )
		{
			return exists((uint8_t *)&aKey);
		}
		bool exists( const uint8_t aKey[] )
		{
			bool			found = false;
			volatile Node	*n = getNodeForKey(aKey);
			if (n != NULL) {
				found = n->valid;
			}
			return found;
		}


		/**
		 * This method is a convenience method fronting the exists()
		 * method where we use the key_value() function to get the
		 * key for the value, and then use the exists() method to see
		 * if this key exists in the trie. If so, a 'true' is returned.
		 */
		bool value_exists( const T & aValue )
		{
			// get the key from the value, and use the other method
			return exists(key_value(aValue));
		}


		/**
		 * This method will return 'true' only if there are NO valid
		 * values stored in this trie at this time. Since this is all
		 * lockless, it's possible that immediately after the scan,
		 * something is added - or removed, it's a result that needs
		 * to be carefully interpreted.
		 */
		virtual bool empty()
		{
			bool		vacant = true;
			for (uint16_t i = 0; i < 256; ++i) {
				if ((_roots[i] != NULL) &&
					!(const_cast<Branch *>(_roots[i]))->empty()) {
					vacant = false;
					break;
				}
			}
			return vacant;
		}


		/**
		 * This method will look at the entire contents of the trie
		 * and return the BEST ESTIMATE at the number of values it
		 * contains. The reason that it's an estimate is because we
		 * can't be sure that things aren't being added and removed
		 * while we do the count as all the accesses are lockless.
		 * But if there's no activity on this trie, then this will
		 * return the accurate number of items contained within.
		 */
		virtual size_t size()
		{
			size_t		sz = 0;
			for (uint16_t i = 0; i < 256; ++i) {
				if (_roots[i] != NULL) {
					sz += const_cast<Branch *>(_roots[i])->size();
				}
			}
			return sz;
		}


		/**
		 * This method will clear out the contents of the trie - dropping
		 * any non-pointers, and deleting any pointers that it might be
		 * holding on to. The result is a trie that's ready to store
		 * more, but won't leak a thing.
		 */
		virtual void clear()
		{
			/**
			 * Delete any root level branches we have and the
			 * cascade effect will take care of the rest.
			 */
			for (uint16_t i = 0; i < 256; ++i) {
				if (_roots[i] != NULL) {
					delete _roots[i];
					_roots[i] = NULL;
				}
			}
		}


		/********************************************************
		 *
		 *                Functor Methods
		 *
		 ********************************************************/
		/**
		 * This method takes the functor subclass instance and applies
		 * it's process() method to all the valid entries in the trie.
		 * Since the processing will take place on the Node level, it's
		 * completely up to the implementor to decide what they want
		 * to do, and how they want to do it.
		 */
		virtual bool apply( functor & aFunctor )
		{
			bool		error = false;
			for (uint16_t i = 0; i < 256; ++i) {
				if (_roots[i] != NULL) {
					if (!const_cast<Branch *>(_roots[i])->apply(aFunctor)) {
						error = true;
						break;
					}
				}
			}
			return !error;
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
			return "<trie>";
		}


		/**
		 * This method checks to see if the two tries are equal to one
		 * another based on the values they represent and *not* on the
		 * actual pointers themselves. If they are equal, then this method
		 * returns true, otherwise it returns false.
		 */
		bool operator==( const trie<T,N> & anOther ) const
		{
			bool		equals = false;
			bool		keepChecking = true;

			// first, see if we are checking against ourselves...
			if (keepChecking) {
				if (this == & anOther) {
					equals = true;
					keepChecking = false;
				}
			}

			// next, check each of the branches...
			if (keepChecking) {
				// assume it's equal, and verify each branch
				equals = true;
				for (uint16_t i = 0; keepChecking && (i < 256); ++i) {
					if (_roots[i] == NULL) {
						if (anOther._roots[i] != NULL) {
							equals = false;
							keepChecking = false;
						}
					} else {
						if (anOther._roots[i] == NULL) {
							equals = false;
							keepChecking = false;
						} else if (*(_roots[i]) != *(anOther._roots[i])) {
							equals = false;
							keepChecking = false;
						}
					}
				}
			}

			return equals;
		}


		/**
		 * This method checks to see if the two tries are not equal to
		 * one another based on the values they represent and *not* on the
		 * actual pointers themselves. If they are not equal, then this
		 * method returns true, otherwise it returns false.
		 */
		bool operator!=( const trie<T,N> & anOther ) const
		{
			return !operator==(anOther);
		}


	protected:
		/********************************************************
		 *
		 *            Scanning/Building Methods
		 *
		 ********************************************************/
		/**
		 * This method takes the key and breaks it up into bytes, and
		 * walks the structure of the trie looking for the leaf, and
		 * then the Node at the corresponding location. If it encounters
		 * a NULL, this method will immediately return NULL, but if
		 * we can find the Leaf, and in it the Node, then we will
		 * return that Node's pointer. It's up to the caller to do
		 * something intelligent with it.
		 */
		volatile Node *getNodeForKey( const uint8_t aKey[] ) {
			volatile Node	*n = NULL;
			// see if the root branch is available, and work with that
			volatile Component	*c = _roots[aKey[0]];
			if (c != NULL) {
				n = const_cast<Component *>(c)->getNodeForKey(aKey, 1);
			}
			// return what we have dug out of the tree
			return n;
		}


		/**
		 * This method takes the key and walks/builds it's way to
		 * the appropriate Node, if it exists, and if not, then this
		 * method will BUILD everything it needs to on the path to
		 * the Node so that it can return a non-NULL Node to the
		 * caller. This is the way of CREATING the trie, and will
		 * do everything it can to fill out the tree of branches to
		 * get to the Node requested.
		 */
		volatile Node *getOrCreateNodeForKey( uint16_t aKey ) {
			return getOrCreateNodeForKey((uint8_t *)&aKey);
		}
		volatile Node *getOrCreateNodeForKey( uint32_t aKey ) {
			return getOrCreateNodeForKey((uint8_t *)&aKey);
		}
		volatile Node *getOrCreateNodeForKey( uint64_t aKey ) {
			return getOrCreateNodeForKey((uint8_t *)&aKey);
		}
		volatile Node *getOrCreateNodeForKey( const uint8_t aKey[] ) {
			volatile Node	*n = NULL;

			// get the index we're working on (re-used a few times)
			uint8_t		idx = aKey[0];
			volatile Component	*curr = __sync_or_and_fetch(&_roots[idx], 0x0);
			if (curr == NULL) {
				// create a new Branch for this part of the trie
				curr = new Branch();
				if (curr == NULL) {
					throw std::runtime_error("[trie<T>::getOrCreateNodeForKey] Unable to create new Branch for the trie!");
				}
				// see if we can put this new one in the right place
				if (!__sync_bool_compare_and_swap(&_roots[idx], NULL, curr)) {
					// someone beat us to it! Delete what we just made...
					delete curr;
					// ...and get what is there now
					curr = __sync_or_and_fetch(&_roots[idx], 0x0);
				}
			}

			// now pass down to that next branch the request to fill
			if (curr != NULL) {
				n = const_cast<Component *>(curr)->getOrCreateNodeForKey(aKey, 1);
			}

			// return what we have dug out of the tree
			return n;
		}


	private:
		/**
		 * Because the size of the key, in bytes, is based on the template
		 * parameter, we need to calculate the last "step" to create a
		 * Branch, and after that, we create a single Leaf node. This is
		 * here just to make that comparison a little faster and easier to
		 * read.
		 */
		enum {
			eLastBranch = (N - 2)
		};
		/**
		 * The trie needs to start with the first byte of the 64-bit
		 * key value being 1 of 256 root branches for the tree. This
		 * simple array is the easiest way to ensure that we have space
		 * for all the Branches - should we need them, but not wasting
		 * too much space by pre-allocating them.
		 */
		volatile Branch		*_roots[256];
};
}		// end of namespace dkit

#endif		// __DKIT_TRIE_H
