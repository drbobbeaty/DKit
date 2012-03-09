/**
 * abool.h - this file defines the atomic bool class and mimics the simple
 *           bool datatype in the C spec. The class is to be treated like
 *           the regular datatypes in C++ - just when they are read/written,
 *           the operations are atomic in nature.
 */
#ifndef __DKIT_ABOOL_H
#define __DKIT_ABOOL_H

//	System Headers
#include <stdint.h>
#include <ostream>

//	Third-Party Headers

//	Other Headers

//	Forward Declarations
/**
 * Let's place all the declarations here so that the individual definitions
 * of methods, etc. can use these as references in their signatures. This
 * way, the abool can use the auint8_t in an operator=() signature, but not
 * have to worry about the order of the definitions.
 */
class abool;
class auint8_t;
class aint8_t;
class auint16_t;
class aint16_t;
class auint32_t;
class aint32_t;
class auint64_t;
class aint64_t;

//	Public Constants

//	Public Datatypes

//	Public Data Constants

/**
 * Start off with a simple atomic bool value - you will find dozens of uses
 * for an atomic bool.
 */
class abool
{
	public:
		/********************************************************
		 *
		 *                Constructors/Destructor
		 *
		 ********************************************************/
		/**
		 * This is the default constructor that sets up the value of
		 * false (0) in the variable. This is the same default that a
		 * traditional boolean would have.
		 */
		abool();
		/**
		 * This constructor takes a traditional bool and creates a new
		 * atomic bool based on this value. It's not a copy constructor
		 * because we haven't really 'copied' anything - we're just taking
		 * the value.
		 */
		abool( bool aValue );
		/**
		 * This is the standard copy constructor and needs to be in every
		 * class to make sure that we don't have too many things running
		 * around.
		 */
		abool( const abool & anOther );
		/**
		 * This is the standard destructor and needs to be virtual to make
		 * sure that if we subclass off this the right destructor will be
		 * called.
		 */
		virtual ~abool();

		/**
		 * When we want to process the result of an equality we need to
		 * make sure that we do this right by always having an equals
		 * operator on all classes.
		 */
		abool & operator=( const abool & anOther );
		/**
		 * Let's also allow this value to be taken from one of the other
		 * data types defined in stdint.h - again, according to the zero/
		 * non-zero rule for booleans. This just makes it very easy to work
		 * in this atomic bool into existing code.
		 */
		abool & operator=( bool aValue );
		abool & operator=( uint8_t aValue );
		abool & operator=( int8_t aValue );
		abool & operator=( uint16_t aValue );
		abool & operator=( int16_t aValue );
		abool & operator=( uint32_t aValue );
		abool & operator=( int32_t aValue );
		abool & operator=( uint64_t aValue );
		abool & operator=( int64_t aValue );
		/**
		 * Let's also allow this guy to take values from the classes in
		 * this package so that there is no unnecessary casting or method
		 * calling to get these values into the atomic boolean.
		 */
		abool & operator=( const auint8_t & aValue );
		abool & operator=( const aint8_t & aValue );
		abool & operator=( const auint16_t & aValue );
		abool & operator=( const aint16_t & aValue );
		abool & operator=( const auint32_t & aValue );
		abool & operator=( const aint32_t & aValue );
		abool & operator=( const auint64_t & aValue );
		abool & operator=( const aint64_t & aValue );

		/********************************************************
		 *
		 *                Accessor Methods
		 *
		 ********************************************************/
		/**
		 * This method returns the current value at the time of the
		 * call. Since this can change at any time, this is really just
		 * a snapshot of the value, and should not be considered a long-
		 * term condition.
		 */
		bool getValue() const;
		/**
		 * This method allows a more deliberate setting of the value by
		 * the caller. This is typically not used, but it can be in those
		 * times when the explicit method call is cleaner to use.
		 */
		void setValue( bool aValue );

		/********************************************************
		 *
		 *             Useful Operator Methods
		 *
		 ********************************************************/
		/**
		 * This casting operator takes the atomic bool and maps it's
		 * value into a simple bool for all those times when you really
		 * need that bool. This is again a snapshot as the value can change
		 * immediately upon return, but it's as good as you'll get.
		 */
		operator bool() const;
		/**
		 * These map the abool's value into the other basic data types in
		 * the stdint.h package.
		 */
		operator int() const;
		operator uint8_t() const;
		operator int8_t() const;
		operator uint16_t() const;
		operator int16_t() const;
		operator uint32_t() const;
		operator uint64_t() const;
		operator int64_t() const;

		/**
		 * These are the prefix and postfix increment operators, in that
		 * order, and the first returns a reference to the same instance
		 * while the latter has to receive a copy prior to the increment.
		 * In the case of the boolean, these simply "flip" the state of the
		 * bool from true->false->true, etc.
		 */
		abool & operator++();
		abool operator++(int);
		/**
		 * These are the prefix and postfix decrement operators, in that
		 * order, and the first returns a reference to the same instance
		 * while the latter has to receive a copy prior to the decrement.
		 * In the case of the boolean, these simply "flip" the state of the
		 * bool from true->false->true, etc.
		 */
		abool & operator--();
		abool operator--(int);
		/**
		 * These are the standard RHS operators for this guy, and are all
		 * handled in a consistent, thread-safe way. The way this works for
		 * a boolean is that each move "flips" the state, so a simple
		 * modulo 2 gives us whether or not to flip this value at all.
		 */
		abool & operator+=( bool aValue );
		abool & operator+=( uint8_t aValue );
		abool & operator+=( int8_t aValue );
		abool & operator+=( uint16_t aValue );
		abool & operator+=( int16_t aValue );
		abool & operator+=( uint32_t aValue );
		abool & operator+=( int32_t aValue );
		abool & operator+=( uint64_t aValue );
		abool & operator+=( int64_t aValue );

		abool & operator-=( bool aValue );
		abool & operator-=( uint8_t aValue );
		abool & operator-=( int8_t aValue );
		abool & operator-=( uint16_t aValue );
		abool & operator-=( int16_t aValue );
		abool & operator-=( uint32_t aValue );
		abool & operator-=( int32_t aValue );
		abool & operator-=( uint64_t aValue );
		abool & operator-=( int64_t aValue );

		/**
		 * These versions are the same operators, but using the components
		 * in this package as opposed to the basic data types they are modeled
		 * after.
		 */
		abool & operator+=( const abool & aValue );
		abool & operator+=( const auint8_t & aValue );
		abool & operator+=( const aint8_t & aValue );
		abool & operator+=( const auint16_t & aValue );
		abool & operator+=( const aint16_t & aValue );
		abool & operator+=( const auint32_t & aValue );
		abool & operator+=( const aint32_t & aValue );
		abool & operator+=( const auint64_t & aValue );
		abool & operator+=( const aint64_t & aValue );

		abool & operator-=( const abool & aValue );
		abool & operator-=( const auint8_t & aValue );
		abool & operator-=( const aint8_t & aValue );
		abool & operator-=( const auint16_t & aValue );
		abool & operator-=( const aint16_t & aValue );
		abool & operator-=( const auint32_t & aValue );
		abool & operator-=( const aint32_t & aValue );
		abool & operator-=( const auint64_t & aValue );
		abool & operator-=( const aint64_t & aValue );

		/********************************************************
		 *
		 *                Utility Methods
		 *
		 ********************************************************/
		/**
		 * This method checks to see if the two values are equal to one
		 * another based on the values they represent and *not* on the
		 * actual pointers themselves. If they are equal, then this method
		 * returns true, otherwise it returns false.
		 */
		bool operator==( const abool & anOther ) const;
		/**
		 * This method checks to see if the two values are not equal to
		 * one another based on the values they represent and *not* on the
		 * actual pointers themselves. If they are not equal, then this
		 * method returns true, otherwise it returns false.
		 */
		bool operator!=( const abool & anOther ) const;

		/**
		 * These are the equals and not equals operators for the atomic
		 * bool w.r.t. the plain data types in the stdint.h package.
		 */
		bool operator==( bool aValue ) const;
		bool operator==( uint8_t aValue ) const;
		bool operator==( int8_t aValue ) const;
		bool operator==( uint16_t aValue ) const;
		bool operator==( int16_t aValue ) const;
		bool operator==( uint32_t aValue ) const;
		bool operator==( int32_t aValue ) const;
		bool operator==( uint64_t aValue ) const;
		bool operator==( int64_t aValue ) const;

		bool operator!=( bool aValue ) const;
		bool operator!=( uint8_t aValue ) const;
		bool operator!=( int8_t aValue ) const;
		bool operator!=( uint16_t aValue ) const;
		bool operator!=( int16_t aValue ) const;
		bool operator!=( uint32_t aValue ) const;
		bool operator!=( int32_t aValue ) const;
		bool operator!=( uint64_t aValue ) const;
		bool operator!=( int64_t aValue ) const;
		/**
		 * These are the equals and not equals operators for the atomic
		 * bool w.r.t. the classes in this package. This is done to make
		 * the difference between an atomic and non-atomic version as
		 * small as possible.
		 */
		bool operator==( const auint8_t & aValue ) const;
		bool operator==( const aint8_t & aValue ) const;
		bool operator==( const auint16_t & aValue ) const;
		bool operator==( const aint16_t & aValue ) const;
		bool operator==( const auint32_t & aValue ) const;
		bool operator==( const aint32_t & aValue ) const;
		bool operator==( const auint64_t & aValue ) const;
		bool operator==( const aint64_t & aValue ) const;

		bool operator!=( const auint8_t & aValue ) const;
		bool operator!=( const aint8_t & aValue ) const;
		bool operator!=( const auint16_t & aValue ) const;
		bool operator!=( const aint16_t & aValue ) const;
		bool operator!=( const auint32_t & aValue ) const;
		bool operator!=( const aint32_t & aValue ) const;
		bool operator!=( const auint64_t & aValue ) const;
		bool operator!=( const aint64_t & aValue ) const;

	private:
		/**
		 * This is the actual value - a single unsigned byte that will
		 * be 0 for false and 1 for true. Simple and easy to use.
		 */
		uint8_t				_value;
};

/**
 * For debugging purposes, let's make it easy for the user to stream
 * out this value. It basically is just the value of the base data type
 * and puts it into the stream.
 */
std::ostream & operator<<( std::ostream & aStream, const abool & aValue );

#endif		// __DKIT_ABOOL_H
