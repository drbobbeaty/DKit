/**
 * aint16.h - this file defines the two atomic 16-bit integer classes and
 *            mimics the simple int16_t and uint16_t datatypes in the
 *            stdint.h package. The class is to be treated like the regular
 *            datatypes in C++ - just when they are read/written, the
 *            operations are atomic in nature.
 */
#ifndef __DKIT_AINT16_H
#define __DKIT_AINT16_H

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
 * Start off with a simple atomic unsigned 16-bit integer value.
 */
class auint16_t
{
	public:
		/********************************************************
		 *
		 *                Constructors/Destructor
		 *
		 ********************************************************/
		/**
		 * This is the default constructor that sets up the value of
		 * zero (0) in the variable. This is the same default that a
		 * traditional uint16_t would have.
		 */
		auint16_t();
		/**
		 * This constructor takes a traditional uint16_t and creates a new
		 * atomic uint16_t based on this value. It's not a copy constructor
		 * because we haven't really 'copied' anything - we're just taking
		 * the value.
		 */
		auint16_t( uint16_t aValue );
		/**
		 * This is the standard copy constructor and needs to be in every
		 * class to make sure that we don't have too many things running
		 * around.
		 */
		auint16_t( const auint16_t & anOther );
		/**
		 * This is the standard destructor and needs to be virtual to make
		 * sure that if we subclass off this the right destructor will be
		 * called.
		 */
		virtual ~auint16_t();

		/**
		 * When we want to process the result of an equality we need to
		 * make sure that we do this right by always having an equals
		 * operator on all classes.
		 */
		auint16_t & operator=( const auint16_t & anOther );
		/**
		 * Let's also allow this value to be taken from one of the other
		 * data types defined in stdint.h. Because of the size difference,
		 * we will simply TRUNCATE the values that are too large to hold
		 * in this instance to what can fit. This just makes it very easy
		 * to work in this atomic integer into existing code.
		 */
		auint16_t & operator=( bool aValue );
		auint16_t & operator=( uint8_t aValue );
		auint16_t & operator=( int8_t aValue );
		auint16_t & operator=( uint16_t aValue );
		auint16_t & operator=( int16_t aValue );
		auint16_t & operator=( uint32_t aValue );
		auint16_t & operator=( int32_t aValue );
		auint16_t & operator=( uint64_t aValue );
		auint16_t & operator=( int64_t aValue );
		/**
		 * Let's also allow this guy to take values from the classes in
		 * this package so that there is no unnecessary casting or method
		 * calling to get these values into the atomic integer.
		 */
		auint16_t & operator=( const abool & aValue );
		auint16_t & operator=( const auint8_t & aValue );
		auint16_t & operator=( const aint8_t & aValue );
		auint16_t & operator=( const aint16_t & aValue );
		auint16_t & operator=( const auint32_t & aValue );
		auint16_t & operator=( const aint32_t & aValue );
		auint16_t & operator=( const auint64_t & aValue );
		auint16_t & operator=( const aint64_t & aValue );

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
		uint16_t getValue() const;
		/**
		 * This method allows a more deliberate setting of the value by
		 * the caller. This is typically not used, but it can be in those
		 * times when the explicit method call is cleaner to use.
		 */
		void setValue( uint16_t aValue );

		/********************************************************
		 *
		 *             Useful Operator Methods
		 *
		 ********************************************************/
		/**
		 * This casting operator takes the atomic uint16_t and maps it's
		 * value into a simple uint16_t for all those times when you really
		 * need that value. This is again a snapshot as the value can change
		 * immediately upon return, but it's as good as you'll get.
		 */
		operator uint16_t() const;
		/**
		 * These map the auint16_t's value into the other basic data types in
		 * the stdint.h package.
		 */
		operator int() const;
		operator bool() const;
		operator uint8_t() const;
		operator int8_t() const;
		operator int16_t() const;
		operator uint32_t() const;
		operator uint64_t() const;
		operator int64_t() const;

		/**
		 * These are the prefix and postfix increment operators, in that
		 * order, and the first returns a reference to the same instance
		 * while the latter has to receive a copy prior to the increment.
		 */
		auint16_t & operator++();
		auint16_t operator++(int);
		/**
		 * These are the prefix and postfix decrement operators, in that
		 * order, and the first returns a reference to the same instance
		 * while the latter has to receive a copy prior to the decrement.
		 */
		auint16_t & operator--();
		auint16_t operator--(int);
		/**
		 * These are the standard RHS operators for this guy, and are all
		 * handled in a consistent, thread-safe way.
		 */
		auint16_t & operator+=( bool aValue );
		auint16_t & operator+=( uint8_t aValue );
		auint16_t & operator+=( int8_t aValue );
		auint16_t & operator+=( uint16_t aValue );
		auint16_t & operator+=( int16_t aValue );
		auint16_t & operator+=( uint32_t aValue );
		auint16_t & operator+=( int32_t aValue );
		auint16_t & operator+=( uint64_t aValue );
		auint16_t & operator+=( int64_t aValue );

		auint16_t & operator-=( bool aValue );
		auint16_t & operator-=( uint8_t aValue );
		auint16_t & operator-=( int8_t aValue );
		auint16_t & operator-=( uint16_t aValue );
		auint16_t & operator-=( int16_t aValue );
		auint16_t & operator-=( uint32_t aValue );
		auint16_t & operator-=( int32_t aValue );
		auint16_t & operator-=( uint64_t aValue );
		auint16_t & operator-=( int64_t aValue );

		/**
		 * These versions are the same operators, but using the components
		 * in this package as opposed to the basic data types they are modeled
		 * after.
		 */
		auint16_t & operator+=( const abool & aValue );
		auint16_t & operator+=( const auint8_t & aValue );
		auint16_t & operator+=( const aint8_t & aValue );
		auint16_t & operator+=( const auint16_t & aValue );
		auint16_t & operator+=( const aint16_t & aValue );
		auint16_t & operator+=( const auint32_t & aValue );
		auint16_t & operator+=( const aint32_t & aValue );
		auint16_t & operator+=( const auint64_t & aValue );
		auint16_t & operator+=( const aint64_t & aValue );

		auint16_t & operator-=( const abool & aValue );
		auint16_t & operator-=( const auint8_t & aValue );
		auint16_t & operator-=( const aint8_t & aValue );
		auint16_t & operator-=( const auint16_t & aValue );
		auint16_t & operator-=( const aint16_t & aValue );
		auint16_t & operator-=( const auint32_t & aValue );
		auint16_t & operator-=( const aint32_t & aValue );
		auint16_t & operator-=( const auint64_t & aValue );
		auint16_t & operator-=( const aint64_t & aValue );

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
		bool operator==( const auint16_t & anOther ) const;
		/**
		 * This method checks to see if the two values are not equal to
		 * one another based on the values they represent and *not* on the
		 * actual pointers themselves. If they are not equal, then this
		 * method returns true, otherwise it returns false.
		 */
		bool operator!=( const auint16_t & anOther ) const;

		/**
		 * These are the equals and not equals operators for the atomic
		 * integer w.r.t. the plain data types in the stdint.h package.
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
		 * integer w.r.t. the classes in this package. This is done to make
		 * the difference between an atomic and non-atomic version as
		 * small as possible.
		 */
		bool operator==( const abool & aValue ) const;
		bool operator==( const auint8_t & aValue ) const;
		bool operator==( const aint8_t & aValue ) const;
		bool operator==( const aint16_t & aValue ) const;
		bool operator==( const auint32_t & aValue ) const;
		bool operator==( const aint32_t & aValue ) const;
		bool operator==( const auint64_t & aValue ) const;
		bool operator==( const aint64_t & aValue ) const;

		bool operator!=( const abool & aValue ) const;
		bool operator!=( const auint8_t & aValue ) const;
		bool operator!=( const aint8_t & aValue ) const;
		bool operator!=( const aint16_t & aValue ) const;
		bool operator!=( const auint32_t & aValue ) const;
		bool operator!=( const aint32_t & aValue ) const;
		bool operator!=( const auint64_t & aValue ) const;
		bool operator!=( const aint64_t & aValue ) const;
		/**
		 * These are the rest of the inequalities for the elemental data
		 * types that would be in use in code.
		 */
		bool operator<( uint8_t aValue ) const;
		bool operator<( int8_t aValue ) const;
		bool operator<( uint16_t aValue ) const;
		bool operator<( int16_t aValue ) const;
		bool operator<( uint32_t aValue ) const;
		bool operator<( int32_t aValue ) const;
		bool operator<( uint64_t aValue ) const;
		bool operator<( int64_t aValue ) const;

		bool operator<=( uint8_t aValue ) const;
		bool operator<=( int8_t aValue ) const;
		bool operator<=( uint16_t aValue ) const;
		bool operator<=( int16_t aValue ) const;
		bool operator<=( uint32_t aValue ) const;
		bool operator<=( int32_t aValue ) const;
		bool operator<=( uint64_t aValue ) const;
		bool operator<=( int64_t aValue ) const;

		bool operator>( uint8_t aValue ) const;
		bool operator>( int8_t aValue ) const;
		bool operator>( uint16_t aValue ) const;
		bool operator>( int16_t aValue ) const;
		bool operator>( uint32_t aValue ) const;
		bool operator>( int32_t aValue ) const;
		bool operator>( uint64_t aValue ) const;
		bool operator>( int64_t aValue ) const;

		bool operator>=( uint8_t aValue ) const;
		bool operator>=( int8_t aValue ) const;
		bool operator>=( uint16_t aValue ) const;
		bool operator>=( int16_t aValue ) const;
		bool operator>=( uint32_t aValue ) const;
		bool operator>=( int32_t aValue ) const;
		bool operator>=( uint64_t aValue ) const;
		bool operator>=( int64_t aValue ) const;
		/**
		 * These are the rest of the inequalities for the other classes
		 * in this set that might be used in comparisons.
		 */
		bool operator<( const auint8_t & aValue ) const;
		bool operator<( const aint8_t & aValue ) const;
		bool operator<( const auint16_t & aValue ) const;
		bool operator<( const aint16_t & aValue ) const;
		bool operator<( const auint32_t & aValue ) const;
		bool operator<( const aint32_t & aValue ) const;
		bool operator<( const auint64_t & aValue ) const;
		bool operator<( const aint64_t & aValue ) const;

		bool operator<=( const auint8_t & aValue ) const;
		bool operator<=( const aint8_t & aValue ) const;
		bool operator<=( const auint16_t & aValue ) const;
		bool operator<=( const aint16_t & aValue ) const;
		bool operator<=( const auint32_t & aValue ) const;
		bool operator<=( const aint32_t & aValue ) const;
		bool operator<=( const auint64_t & aValue ) const;
		bool operator<=( const aint64_t & aValue ) const;

		bool operator>( const auint8_t & aValue ) const;
		bool operator>( const aint8_t & aValue ) const;
		bool operator>( const auint16_t & aValue ) const;
		bool operator>( const aint16_t & aValue ) const;
		bool operator>( const auint32_t & aValue ) const;
		bool operator>( const aint32_t & aValue ) const;
		bool operator>( const auint64_t & aValue ) const;
		bool operator>( const aint64_t & aValue ) const;

		bool operator>=( const auint8_t & aValue ) const;
		bool operator>=( const aint8_t & aValue ) const;
		bool operator>=( const auint16_t & aValue ) const;
		bool operator>=( const aint16_t & aValue ) const;
		bool operator>=( const auint32_t & aValue ) const;
		bool operator>=( const aint32_t & aValue ) const;
		bool operator>=( const auint64_t & aValue ) const;
		bool operator>=( const aint64_t & aValue ) const;

	private:
		/**
		 * This is the actual value - an unsigned two-byte integer that will
		 * be everything we need it to be.
		 */
		uint16_t				_value;
};


/**
 * Finish with a simple atomic signed 16-bit integer value.
 */
class aint16_t
{
	public:
		/********************************************************
		 *
		 *                Constructors/Destructor
		 *
		 ********************************************************/
		/**
		 * This is the default constructor that sets up the value of
		 * zero (0) in the variable. This is the same default that a
		 * traditional int16_t would have.
		 */
		aint16_t();
		/**
		 * This constructor takes a traditional int16_t and creates a new
		 * atomic int16_t based on this value. It's not a copy constructor
		 * because we haven't really 'copied' anything - we're just taking
		 * the value.
		 */
		aint16_t( int16_t aValue );
		/**
		 * This is the standard copy constructor and needs to be in every
		 * class to make sure that we don't have too many things running
		 * around.
		 */
		aint16_t( const aint16_t & anOther );
		/**
		 * This is the standard destructor and needs to be virtual to make
		 * sure that if we subclass off this the right destructor will be
		 * called.
		 */
		virtual ~aint16_t();

		/**
		 * When we want to process the result of an equality we need to
		 * make sure that we do this right by always having an equals
		 * operator on all classes.
		 */
		aint16_t & operator=( const aint16_t & anOther );
		/**
		 * Let's also allow this value to be taken from one of the other
		 * data types defined in stdint.h. Because of the size difference,
		 * we will simply TRUNCATE the values that are too large to hold
		 * in this instance to what can fit. This just makes it very easy
		 * to work in this atomic integer into existing code.
		 */
		aint16_t & operator=( bool aValue );
		aint16_t & operator=( uint8_t aValue );
		aint16_t & operator=( int8_t aValue );
		aint16_t & operator=( uint16_t aValue );
		aint16_t & operator=( int16_t aValue );
		aint16_t & operator=( uint32_t aValue );
		aint16_t & operator=( int32_t aValue );
		aint16_t & operator=( uint64_t aValue );
		aint16_t & operator=( int64_t aValue );
		/**
		 * Let's also allow this guy to take values from the classes in
		 * this package so that there is no unnecessary casting or method
		 * calling to get these values into the atomic integer.
		 */
		aint16_t & operator=( const abool & aValue );
		aint16_t & operator=( const auint8_t & aValue );
		aint16_t & operator=( const aint8_t & aValue );
		aint16_t & operator=( const auint16_t & aValue );
		aint16_t & operator=( const auint32_t & aValue );
		aint16_t & operator=( const aint32_t & aValue );
		aint16_t & operator=( const auint64_t & aValue );
		aint16_t & operator=( const aint64_t & aValue );

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
		int16_t getValue() const;
		/**
		 * This method allows a more deliberate setting of the value by
		 * the caller. This is typically not used, but it can be in those
		 * times when the explicit method call is cleaner to use.
		 */
		void setValue( int16_t aValue );

		/********************************************************
		 *
		 *             Useful Operator Methods
		 *
		 ********************************************************/
		/**
		 * This casting operator takes the atomic int16_t and maps it's
		 * value into a simple int16_t for all those times when you really
		 * need that value. This is again a snapshot as the value can change
		 * immediately upon return, but it's as good as you'll get.
		 */
		operator int16_t() const;
		/**
		 * These map the aint16_t's value into the other basic data types in
		 * the stdint.h package.
		 */
		operator int() const;
		operator bool() const;
		operator uint8_t() const;
		operator int8_t() const;
		operator uint16_t() const;
		operator uint32_t() const;
		operator uint64_t() const;
		operator int64_t() const;

		/**
		 * These are the prefix and postfix increment operators, in that
		 * order, and the first returns a reference to the same instance
		 * while the latter has to receive a copy prior to the increment.
		 */
		aint16_t & operator++();
		aint16_t operator++(int);
		/**
		 * These are the prefix and postfix decrement operators, in that
		 * order, and the first returns a reference to the same instance
		 * while the latter has to receive a copy prior to the decrement.
		 */
		aint16_t & operator--();
		aint16_t operator--(int);
		/**
		 * These are the standard RHS operators for this guy, and are all
		 * handled in a consistent, thread-safe way.
		 */
		aint16_t & operator+=( bool aValue );
		aint16_t & operator+=( uint8_t aValue );
		aint16_t & operator+=( int8_t aValue );
		aint16_t & operator+=( uint16_t aValue );
		aint16_t & operator+=( int16_t aValue );
		aint16_t & operator+=( uint32_t aValue );
		aint16_t & operator+=( int32_t aValue );
		aint16_t & operator+=( uint64_t aValue );
		aint16_t & operator+=( int64_t aValue );

		aint16_t & operator-=( bool aValue );
		aint16_t & operator-=( uint8_t aValue );
		aint16_t & operator-=( int8_t aValue );
		aint16_t & operator-=( uint16_t aValue );
		aint16_t & operator-=( int16_t aValue );
		aint16_t & operator-=( uint32_t aValue );
		aint16_t & operator-=( int32_t aValue );
		aint16_t & operator-=( uint64_t aValue );
		aint16_t & operator-=( int64_t aValue );

		/**
		 * These versions are the same operators, but using the components
		 * in this package as opposed to the basic data types they are modeled
		 * after.
		 */
		aint16_t & operator+=( const abool & aValue );
		aint16_t & operator+=( const auint8_t & aValue );
		aint16_t & operator+=( const aint8_t & aValue );
		aint16_t & operator+=( const auint16_t & aValue );
		aint16_t & operator+=( const aint16_t & aValue );
		aint16_t & operator+=( const auint32_t & aValue );
		aint16_t & operator+=( const aint32_t & aValue );
		aint16_t & operator+=( const auint64_t & aValue );
		aint16_t & operator+=( const aint64_t & aValue );

		aint16_t & operator-=( const abool & aValue );
		aint16_t & operator-=( const auint8_t & aValue );
		aint16_t & operator-=( const aint8_t & aValue );
		aint16_t & operator-=( const auint16_t & aValue );
		aint16_t & operator-=( const aint16_t & aValue );
		aint16_t & operator-=( const auint32_t & aValue );
		aint16_t & operator-=( const aint32_t & aValue );
		aint16_t & operator-=( const auint64_t & aValue );
		aint16_t & operator-=( const aint64_t & aValue );

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
		bool operator==( const aint16_t & anOther ) const;
		/**
		 * This method checks to see if the two values are not equal to
		 * one another based on the values they represent and *not* on the
		 * actual pointers themselves. If they are not equal, then this
		 * method returns true, otherwise it returns false.
		 */
		bool operator!=( const aint16_t & anOther ) const;

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
		 * integer w.r.t. the classes in this package. This is done to make
		 * the difference between an atomic and non-atomic version as
		 * small as possible.
		 */
		bool operator==( const abool & aValue ) const;
		bool operator==( const auint8_t & aValue ) const;
		bool operator==( const aint8_t & aValue ) const;
		bool operator==( const auint16_t & aValue ) const;
		bool operator==( const auint32_t & aValue ) const;
		bool operator==( const aint32_t & aValue ) const;
		bool operator==( const auint64_t & aValue ) const;
		bool operator==( const aint64_t & aValue ) const;

		bool operator!=( const abool & aValue ) const;
		bool operator!=( const auint8_t & aValue ) const;
		bool operator!=( const aint8_t & aValue ) const;
		bool operator!=( const auint16_t & aValue ) const;
		bool operator!=( const auint32_t & aValue ) const;
		bool operator!=( const aint32_t & aValue ) const;
		bool operator!=( const auint64_t & aValue ) const;
		bool operator!=( const aint64_t & aValue ) const;
		/**
		 * These are the rest of the inequalities for the elemental data
		 * types that would be in use in code.
		 */
		bool operator<( uint8_t aValue ) const;
		bool operator<( int8_t aValue ) const;
		bool operator<( uint16_t aValue ) const;
		bool operator<( int16_t aValue ) const;
		bool operator<( uint32_t aValue ) const;
		bool operator<( int32_t aValue ) const;
		bool operator<( uint64_t aValue ) const;
		bool operator<( int64_t aValue ) const;

		bool operator<=( uint8_t aValue ) const;
		bool operator<=( int8_t aValue ) const;
		bool operator<=( uint16_t aValue ) const;
		bool operator<=( int16_t aValue ) const;
		bool operator<=( uint32_t aValue ) const;
		bool operator<=( int32_t aValue ) const;
		bool operator<=( uint64_t aValue ) const;
		bool operator<=( int64_t aValue ) const;

		bool operator>( uint8_t aValue ) const;
		bool operator>( int8_t aValue ) const;
		bool operator>( uint16_t aValue ) const;
		bool operator>( int16_t aValue ) const;
		bool operator>( uint32_t aValue ) const;
		bool operator>( int32_t aValue ) const;
		bool operator>( uint64_t aValue ) const;
		bool operator>( int64_t aValue ) const;

		bool operator>=( uint8_t aValue ) const;
		bool operator>=( int8_t aValue ) const;
		bool operator>=( uint16_t aValue ) const;
		bool operator>=( int16_t aValue ) const;
		bool operator>=( uint32_t aValue ) const;
		bool operator>=( int32_t aValue ) const;
		bool operator>=( uint64_t aValue ) const;
		bool operator>=( int64_t aValue ) const;
		/**
		 * These are the rest of the inequalities for the other classes
		 * in this set that might be used in comparisons.
		 */
		bool operator<( const auint8_t & aValue ) const;
		bool operator<( const aint8_t & aValue ) const;
		bool operator<( const auint16_t & aValue ) const;
		bool operator<( const aint16_t & aValue ) const;
		bool operator<( const auint32_t & aValue ) const;
		bool operator<( const aint32_t & aValue ) const;
		bool operator<( const auint64_t & aValue ) const;
		bool operator<( const aint64_t & aValue ) const;

		bool operator<=( const auint8_t & aValue ) const;
		bool operator<=( const aint8_t & aValue ) const;
		bool operator<=( const auint16_t & aValue ) const;
		bool operator<=( const aint16_t & aValue ) const;
		bool operator<=( const auint32_t & aValue ) const;
		bool operator<=( const aint32_t & aValue ) const;
		bool operator<=( const auint64_t & aValue ) const;
		bool operator<=( const aint64_t & aValue ) const;

		bool operator>( const auint8_t & aValue ) const;
		bool operator>( const aint8_t & aValue ) const;
		bool operator>( const auint16_t & aValue ) const;
		bool operator>( const aint16_t & aValue ) const;
		bool operator>( const auint32_t & aValue ) const;
		bool operator>( const aint32_t & aValue ) const;
		bool operator>( const auint64_t & aValue ) const;
		bool operator>( const aint64_t & aValue ) const;

		bool operator>=( const auint8_t & aValue ) const;
		bool operator>=( const aint8_t & aValue ) const;
		bool operator>=( const auint16_t & aValue ) const;
		bool operator>=( const aint16_t & aValue ) const;
		bool operator>=( const auint32_t & aValue ) const;
		bool operator>=( const aint32_t & aValue ) const;
		bool operator>=( const auint64_t & aValue ) const;
		bool operator>=( const aint64_t & aValue ) const;

	private:
		/**
		 * This is the actual value - a single two-byte integer that will
		 * be everything we need it to be.
		 */
		volatile int16_t			_value;
};

/**
 * For debugging purposes, let's make it easy for the user to stream
 * out this value. It basically is just the value of the base data type
 * and puts it into the stream.
 */
std::ostream & operator<<( std::ostream & aStream, const auint16_t & aValue );
std::ostream & operator<<( std::ostream & aStream, const aint16_t & aValue );

#endif		// __DKIT_AINT16_H
