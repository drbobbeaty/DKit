/**
 * aint8.cpp - this file implements the two atomic 8-bit integer classes and
 *             mimics the simple int8_t and uint8_t datatypes in the stdint.h
 *             package. The class is to be treated like the regular datatypes
 *             in C++ - just when they are read/written, the operations are
 *             atomic in nature.
 */

//	System Headers

//	Third-Party Headers

//	Other Headers
#include "abool.h"
#include "aint8.h"
#include "aint16.h"
#include "aint32.h"
#include "aint64.h"

//	Forward Declarations

//	Private Constants

//	Private Datatypes

//	Private Data Constants


/********************************************************
 *
 *                Constructors/Destructor
 *
 ********************************************************/
/**
 * This is the default constructor that sets up the value of
 * zero (0) in the variable. This is the same default that a
 * traditional uint8_t would have.
 */
auint8_t::auint8_t() :
	_value(0)
{
}


/**
 * This constructor takes a traditional uint8_t and creates a new
 * atomic uint8_t based on this value. It's not a copy constructor
 * because we haven't really 'copied' anything - we're just taking
 * the value.
 */
auint8_t::auint8_t( uint8_t aValue ) :
	_value(aValue)
{
}


/**
 * This is the standard copy constructor and needs to be in every
 * class to make sure that we don't have too many things running
 * around.
 */
auint8_t::auint8_t( const auint8_t & anOther ) :
	_value(anOther.getValue())
{
}


/**
 * This is the standard destructor and needs to be virtual to make
 * sure that if we subclass off this the right destructor will be
 * called.
 */
auint8_t::~auint8_t()
{
	// there's not a lot to do here - yet
}


/**
 * When we want to process the result of an equality we need to
 * make sure that we do this right by always having an equals
 * operator on all classes.
 */
auint8_t & auint8_t::operator=( const auint8_t & anOther )
{
	if (this != & anOther) {
		setValue(anOther.getValue());
	}
	return *this;
}


/**
 * Let's also allow this value to be taken from one of the other
 * data types defined in stdint.h. Because of the size difference,
 * we will simply TRUNCATE the values that are too large to hold
 * in this instance to what can fit. This just makes it very easy
 * to work in this atomic integer into existing code.
 */
auint8_t & auint8_t::operator=( bool aValue )
{
	setValue(aValue ? 1 : 0);
	return *this;
}


auint8_t & auint8_t::operator=( uint8_t aValue )
{
	setValue(aValue);
	return *this;
}


auint8_t & auint8_t::operator=( int8_t aValue )
{
	setValue((uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator=( uint16_t aValue )
{
	setValue((uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator=( int16_t aValue )
{
	setValue((uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator=( uint32_t aValue )
{
	setValue((uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator=( int32_t aValue )
{
	setValue((uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator=( uint64_t aValue )
{
	setValue((uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator=( int64_t aValue )
{
	setValue((uint8_t)aValue);
	return *this;
}


/**
 * Let's also allow this guy to take values from the classes in
 * this package so that there is no unnecessary casting or method
 * calling to get these values into the atomic integer.
 */
auint8_t & auint8_t::operator=( const abool & aValue )
{
	setValue(aValue.getValue() ? 1 : 0);
	return *this;
}


auint8_t & auint8_t::operator=( const aint8_t & aValue )
{
	setValue(aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator=( const auint16_t & aValue )
{
	setValue((uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator=( const aint16_t & aValue )
{
	setValue((uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator=( const auint32_t & aValue )
{
	setValue((uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator=( const aint32_t & aValue )
{
	setValue((uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator=( const auint64_t & aValue )
{
	setValue((uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator=( const aint64_t & aValue )
{
	setValue((uint8_t)aValue.getValue());
	return *this;
}


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
uint8_t auint8_t::getValue() const
{
	return __sync_or_and_fetch(const_cast<volatile uint8_t *>(&_value), 0x00);
}


/**
 * This method allows a more deliberate setting of the value by
 * the caller. This is typically not used, but it can be in those
 * times when the explicit method call is cleaner to use.
 */
void auint8_t::setValue( uint8_t aValue )
{
	uint8_t		old = _value;
	while (!__sync_bool_compare_and_swap(&_value, old, aValue)) {
		old = _value;
	}
}


/********************************************************
 *
 *             Useful Operator Methods
 *
 ********************************************************/
/**
 * This casting operator takes the atomic uint8_t and maps it's
 * value into a simple uint8_t for all those times when you really
 * need that value. This is again a snapshot as the value can change
 * immediately upon return, but it's as good as you'll get.
 */
auint8_t::operator uint8_t() const
{
	return getValue();
}


/**
 * These map the auint8_t's value into the other basic data types in
 * the stdint.h package.
 */
auint8_t::operator int() const
{
	return getValue();
}


auint8_t::operator bool() const
{
	return (getValue() != 0);
}


auint8_t::operator int8_t() const
{
	return getValue();
}


auint8_t::operator uint16_t() const
{
	return getValue();
}


auint8_t::operator int16_t() const
{
	return getValue();
}


auint8_t::operator uint32_t() const
{
	return getValue();
}


auint8_t::operator uint64_t() const
{
	return getValue();
}


auint8_t::operator int64_t() const
{
	return getValue();
}


/**
 * These are the prefix and postfix increment operators, in that
 * order, and the first returns a reference to the same instance
 * while the latter has to receive a copy prior to the increment.
 */
auint8_t & auint8_t::operator++()
{
	__sync_add_and_fetch(&_value, 1);
	return *this;
}


auint8_t auint8_t::operator++(int)
{
	auint8_t	pre(*this);
	__sync_add_and_fetch(&_value, 1);
	return pre;
}


/**
 * These are the prefix and postfix decrement operators, in that
 * order, and the first returns a reference to the same instance
 * while the latter has to receive a copy prior to the decrement.
 */
auint8_t & auint8_t::operator--()
{
	__sync_sub_and_fetch(&_value, 1);
	return *this;
}


auint8_t auint8_t::operator--(int)
{
	auint8_t	pre(*this);
	__sync_sub_and_fetch(&_value, 1);
	return pre;
}


/**
 * These are the standard RHS operators for this guy, and are all
 * handled in a consistent, thread-safe way.
 */
auint8_t & auint8_t::operator+=( bool aValue )
{
	__sync_add_and_fetch(&_value, (aValue ? 1 : 0));
	return *this;
}


auint8_t & auint8_t::operator+=( uint8_t aValue )
{
	__sync_add_and_fetch(&_value, aValue);
	return *this;
}


auint8_t & auint8_t::operator+=( int8_t aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator+=( uint16_t aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator+=( int16_t aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator+=( uint32_t aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator+=( int32_t aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator+=( uint64_t aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator+=( int64_t aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator-=( bool aValue )
{
	__sync_sub_and_fetch(&_value, (aValue ? 1 : 0));
	return *this;
}


auint8_t & auint8_t::operator-=( uint8_t aValue )
{
	__sync_sub_and_fetch(&_value, aValue);
	return *this;
}


auint8_t & auint8_t::operator-=( int8_t aValue )
{
	__sync_sub_and_fetch(&_value, (uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator-=( uint16_t aValue )
{
	__sync_sub_and_fetch(&_value, (uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator-=( int16_t aValue )
{
	__sync_sub_and_fetch(&_value, (uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator-=( uint32_t aValue )
{
	__sync_sub_and_fetch(&_value, (uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator-=( int32_t aValue )
{
	__sync_sub_and_fetch(&_value, (uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator-=( uint64_t aValue )
{
	__sync_sub_and_fetch(&_value, (uint8_t)aValue);
	return *this;
}


auint8_t & auint8_t::operator-=( int64_t aValue )
{
	__sync_sub_and_fetch(&_value, (uint8_t)aValue);
	return *this;
}


/**
 * These versions are the same operators, but using the components
 * in this package as opposed to the basic data types they are modeled
 * after.
 */
auint8_t & auint8_t::operator+=( const abool & aValue )
{
	__sync_add_and_fetch(&_value, (aValue.getValue() ? 1 : 0));
	return *this;
}


auint8_t & auint8_t::operator+=( const auint8_t & aValue )
{
	__sync_add_and_fetch(&_value, aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator+=( const aint8_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator+=( const auint16_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator+=( const aint16_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator+=( const auint32_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator+=( const aint32_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator+=( const auint64_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator+=( const aint64_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator-=( const abool & aValue )
{
	__sync_sub_and_fetch(&_value, (aValue.getValue() ? 1 : 0));
	return *this;
}


auint8_t & auint8_t::operator-=( const auint8_t & aValue )
{
	__sync_add_and_fetch(&_value, aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator-=( const aint8_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator-=( const auint16_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator-=( const aint16_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator-=( const auint32_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator-=( const aint32_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator-=( const auint64_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue.getValue());
	return *this;
}


auint8_t & auint8_t::operator-=( const aint64_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint8_t)aValue.getValue());
	return *this;
}


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
bool auint8_t::operator==( const auint8_t & anOther ) const
{
	return (getValue() == anOther.getValue());
}


/**
 * This method checks to see if the two values are not equal to
 * one another based on the values they represent and *not* on the
 * actual pointers themselves. If they are not equal, then this
 * method returns true, otherwise it returns false.
 */
bool auint8_t::operator!=( const auint8_t & anOther ) const
{
	return !operator==(anOther);
}


/**
 * These are the equals and not equals operators for the atomic
 * integer w.r.t. the plain data types in the stdint.h package.
 */
bool auint8_t::operator==( bool aValue ) const
{
	return (getValue() == (aValue ? 1 : 0));
}


bool auint8_t::operator==( uint8_t aValue ) const
{
	return (getValue() == aValue);
}


bool auint8_t::operator==( int8_t aValue ) const
{
	return (getValue() == (uint8_t)aValue);
}


bool auint8_t::operator==( uint16_t aValue ) const
{
	return (getValue() == (uint8_t)aValue);
}


bool auint8_t::operator==( int16_t aValue ) const
{
	return (getValue() == (uint8_t)aValue);
}


bool auint8_t::operator==( uint32_t aValue ) const
{
	return (getValue() == (uint8_t)aValue);
}


bool auint8_t::operator==( int32_t aValue ) const
{
	return (getValue() == (uint8_t)aValue);
}


bool auint8_t::operator==( uint64_t aValue ) const
{
	return (getValue() == (uint8_t)aValue);
}


bool auint8_t::operator==( int64_t aValue ) const
{
	return (getValue() == (uint8_t)aValue);
}


bool auint8_t::operator!=( bool aValue ) const
{
	return !operator==(aValue);
}


bool auint8_t::operator!=( uint8_t aValue ) const
{
	return !operator==(aValue);
}


bool auint8_t::operator!=( int8_t aValue ) const
{
	return !operator==(aValue);
}


bool auint8_t::operator!=( uint16_t aValue ) const
{
	return !operator==(aValue);
}


bool auint8_t::operator!=( int16_t aValue ) const
{
	return !operator==(aValue);
}


bool auint8_t::operator!=( uint32_t aValue ) const
{
	return !operator==(aValue);
}


bool auint8_t::operator!=( int32_t aValue ) const
{
	return !operator==(aValue);
}


bool auint8_t::operator!=( uint64_t aValue ) const
{
	return !operator==(aValue);
}


bool auint8_t::operator!=( int64_t aValue ) const
{
	return !operator==(aValue);
}


/**
 * These are the equals and not equals operators for the atomic
 * integer w.r.t. the classes in this package. This is done to make
 * the difference between an atomic and non-atomic version as
 * small as possible.
 */
bool auint8_t::operator==( const abool & aValue ) const
{
	return (getValue() == (aValue.getValue() ? 1 : 0));
}


bool auint8_t::operator==( const aint8_t & aValue ) const
{
	return (getValue() == (uint8_t)aValue.getValue());
}


bool auint8_t::operator==( const auint16_t & aValue ) const
{
	return (getValue() == (uint8_t)aValue.getValue());
}


bool auint8_t::operator==( const aint16_t & aValue ) const
{
	return (getValue() == (uint8_t)aValue.getValue());
}


bool auint8_t::operator==( const auint32_t & aValue ) const
{
	return (getValue() == (uint8_t)aValue.getValue());
}


bool auint8_t::operator==( const aint32_t & aValue ) const
{
	return (getValue() == (uint8_t)aValue.getValue());
}


bool auint8_t::operator==( const auint64_t & aValue ) const
{
	return (getValue() == (uint8_t)aValue.getValue());
}


bool auint8_t::operator==( const aint64_t & aValue ) const
{
	return (getValue() == (uint8_t)aValue.getValue());
}


bool auint8_t::operator!=( const abool & aValue ) const
{
	return !operator==(aValue);
}


bool auint8_t::operator!=( const aint8_t & aValue ) const
{
	return !operator==(aValue);
}


bool auint8_t::operator!=( const auint16_t & aValue ) const
{
	return !operator==(aValue);
}


bool auint8_t::operator!=( const aint16_t & aValue ) const
{
	return !operator==(aValue);
}


bool auint8_t::operator!=( const auint32_t & aValue ) const
{
	return !operator==(aValue);
}


bool auint8_t::operator!=( const aint32_t & aValue ) const
{
	return !operator==(aValue);
}


bool auint8_t::operator!=( const auint64_t & aValue ) const
{
	return !operator==(aValue);
}


bool auint8_t::operator!=( const aint64_t & aValue ) const
{
	return !operator==(aValue);
}


/**
 * These are the rest of the inequalities for the elemental data
 * types that would be in use in code.
 */
bool auint8_t::operator<( uint8_t aValue ) const
{
	return (getValue() < aValue);
}


bool auint8_t::operator<( int8_t aValue ) const
{
	return (getValue() < (uint8_t)aValue);
}


bool auint8_t::operator<( uint16_t aValue ) const
{
	return (getValue() < aValue);
}


bool auint8_t::operator<( int16_t aValue ) const
{
	return (getValue() < (uint16_t)aValue);
}


bool auint8_t::operator<( uint32_t aValue ) const
{
	return (getValue() < aValue);
}


bool auint8_t::operator<( int32_t aValue ) const
{
	return (getValue() < (uint32_t)aValue);
}


bool auint8_t::operator<( uint64_t aValue ) const
{
	return (getValue() < aValue);
}


bool auint8_t::operator<( int64_t aValue ) const
{
	return (getValue() < (uint64_t)aValue);
}


bool auint8_t::operator<=( uint8_t aValue ) const
{
	return (getValue() <= aValue);
}


bool auint8_t::operator<=( int8_t aValue ) const
{
	return (getValue() <= (uint8_t)aValue);
}


bool auint8_t::operator<=( uint16_t aValue ) const
{
	return (getValue() <= aValue);
}


bool auint8_t::operator<=( int16_t aValue ) const
{
	return (getValue() <= (uint16_t)aValue);
}


bool auint8_t::operator<=( uint32_t aValue ) const
{
	return (getValue() <= aValue);
}


bool auint8_t::operator<=( int32_t aValue ) const
{
	return (getValue() <= (uint32_t)aValue);
}


bool auint8_t::operator<=( uint64_t aValue ) const
{
	return (getValue() <= aValue);
}


bool auint8_t::operator<=( int64_t aValue ) const
{
	return (getValue() <= (uint64_t)aValue);
}


bool auint8_t::operator>( uint8_t aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>( int8_t aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>( uint16_t aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>( int16_t aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>( uint32_t aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>( int32_t aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>( uint64_t aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>( int64_t aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>=( uint8_t aValue ) const
{
	return !operator<(aValue);
}


bool auint8_t::operator>=( int8_t aValue ) const
{
	return !operator<(aValue);
}


bool auint8_t::operator>=( uint16_t aValue ) const
{
	return !operator<(aValue);
}


bool auint8_t::operator>=( int16_t aValue ) const
{
	return !operator<(aValue);
}


bool auint8_t::operator>=( uint32_t aValue ) const
{
	return !operator<(aValue);
}


bool auint8_t::operator>=( int32_t aValue ) const
{
	return !operator<(aValue);
}


bool auint8_t::operator>=( uint64_t aValue ) const
{
	return !operator<(aValue);
}


bool auint8_t::operator>=( int64_t aValue ) const
{
	return !operator<(aValue);
}


/**
 * These are the rest of the inequalities for the other classes
 * in this set that might be used in comparisons.
 */
bool auint8_t::operator<( const auint8_t & aValue ) const
{
	return (getValue() < aValue.getValue());
}


bool auint8_t::operator<( const aint8_t & aValue ) const
{
	return (getValue() < (uint8_t)aValue.getValue());
}


bool auint8_t::operator<( const auint16_t & aValue ) const
{
	return (getValue() < aValue.getValue());
}


bool auint8_t::operator<( const aint16_t & aValue ) const
{
	return (getValue() < (uint16_t)aValue.getValue());
}


bool auint8_t::operator<( const auint32_t & aValue ) const
{
	return (getValue() < aValue.getValue());
}


bool auint8_t::operator<( const aint32_t & aValue ) const
{
	return (getValue() < (uint32_t)aValue.getValue());
}


bool auint8_t::operator<( const auint64_t & aValue ) const
{
	return (getValue() < aValue.getValue());
}


bool auint8_t::operator<( const aint64_t & aValue ) const
{
	return (getValue() < (uint64_t)aValue.getValue());
}


bool auint8_t::operator<=( const auint8_t & aValue ) const
{
	return (getValue() <= aValue.getValue());
}


bool auint8_t::operator<=( const aint8_t & aValue ) const
{
	return (getValue() <= (uint8_t)aValue.getValue());
}


bool auint8_t::operator<=( const auint16_t & aValue ) const
{
	return (getValue() <= aValue.getValue());
}


bool auint8_t::operator<=( const aint16_t & aValue ) const
{
	return (getValue() <= (uint16_t)aValue.getValue());
}


bool auint8_t::operator<=( const auint32_t & aValue ) const
{
	return (getValue() <= aValue.getValue());
}


bool auint8_t::operator<=( const aint32_t & aValue ) const
{
	return (getValue() <= (uint32_t)aValue.getValue());
}


bool auint8_t::operator<=( const auint64_t & aValue ) const
{
	return (getValue() <= aValue.getValue());
}


bool auint8_t::operator<=( const aint64_t & aValue ) const
{
	return (getValue() <= (uint64_t)aValue.getValue());
}


bool auint8_t::operator>( const auint8_t & aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>( const aint8_t & aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>( const auint16_t & aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>( const aint16_t & aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>( const auint32_t & aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>( const aint32_t & aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>( const auint64_t & aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>( const aint64_t & aValue ) const
{
	return !operator<=(aValue);
}


bool auint8_t::operator>=( const auint8_t & aValue ) const
{
	return !operator<(aValue);
}


bool auint8_t::operator>=( const aint8_t & aValue ) const
{
	return !operator<(aValue);
}


bool auint8_t::operator>=( const auint16_t & aValue ) const
{
	return !operator<(aValue);
}


bool auint8_t::operator>=( const aint16_t & aValue ) const
{
	return !operator<(aValue);
}


bool auint8_t::operator>=( const auint32_t & aValue ) const
{
	return !operator<(aValue);
}


bool auint8_t::operator>=( const aint32_t & aValue ) const
{
	return !operator<(aValue);
}


bool auint8_t::operator>=( const auint64_t & aValue ) const
{
	return !operator<(aValue);
}


bool auint8_t::operator>=( const aint64_t & aValue ) const
{
	return !operator<(aValue);
}






/********************************************************
 *
 *                Constructors/Destructor
 *
 ********************************************************/
/**
 * This is the default constructor that sets up the value of
 * zero (0) in the variable. This is the same default that a
 * traditional int8_t would have.
 */
aint8_t::aint8_t() :
	_value(0)
{
}


/**
 * This constructor takes a traditional int8_t and creates a new
 * atomic int8_t based on this value. It's not a copy constructor
 * because we haven't really 'copied' anything - we're just taking
 * the value.
 */
aint8_t::aint8_t( int8_t aValue ) :
	_value(aValue)
{
}


/**
 * This is the standard copy constructor and needs to be in every
 * class to make sure that we don't have too many things running
 * around.
 */
aint8_t::aint8_t( const aint8_t & anOther ) :
	_value(anOther.getValue())
{
}


/**
 * This is the standard destructor and needs to be virtual to make
 * sure that if we subclass off this the right destructor will be
 * called.
 */
aint8_t::~aint8_t()
{
	// there's nothing much to do here - yet
}


/**
 * When we want to process the result of an equality we need to
 * make sure that we do this right by always having an equals
 * operator on all classes.
 */
aint8_t & aint8_t::operator=( const aint8_t & anOther )
{
	if (this != & anOther) {
		setValue(anOther.getValue());
	}
	return *this;
}


/**
 * Let's also allow this value to be taken from one of the other
 * data types defined in stdint.h. Because of the size difference,
 * we will simply TRUNCATE the values that are too large to hold
 * in this instance to what can fit. This just makes it very easy
 * to work in this atomic integer into existing code.
 */
aint8_t & aint8_t::operator=( bool aValue )
{
	setValue(aValue ? 1 : 0);
	return *this;
}


aint8_t & aint8_t::operator=( uint8_t aValue )
{
	setValue((int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator=( int8_t aValue )
{
	setValue(aValue);
	return *this;
}


aint8_t & aint8_t::operator=( uint16_t aValue )
{
	setValue((int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator=( int16_t aValue )
{
	setValue((int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator=( uint32_t aValue )
{
	setValue((int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator=( int32_t aValue )
{
	setValue((int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator=( uint64_t aValue )
{
	setValue((int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator=( int64_t aValue )
{
	setValue((int8_t)aValue);
	return *this;
}


/**
 * Let's also allow this guy to take values from the classes in
 * this package so that there is no unnecessary casting or method
 * calling to get these values into the atomic integer.
 */
aint8_t & aint8_t::operator=( const abool & aValue )
{
	setValue(aValue.getValue() ? 1 : 0);
	return *this;
}


aint8_t & aint8_t::operator=( const auint8_t & aValue )
{
	setValue((int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator=( const auint16_t & aValue )
{
	setValue((int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator=( const aint16_t & aValue )
{
	setValue((int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator=( const auint32_t & aValue )
{
	setValue((int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator=( const aint32_t & aValue )
{
	setValue((int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator=( const auint64_t & aValue )
{
	setValue((int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator=( const aint64_t & aValue )
{
	setValue((int8_t)aValue.getValue());
	return *this;
}


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
int8_t aint8_t::getValue() const
{
	return __sync_or_and_fetch(const_cast<volatile int8_t *>(&_value), 0x00);
}


/**
 * This method allows a more deliberate setting of the value by
 * the caller. This is typically not used, but it can be in those
 * times when the explicit method call is cleaner to use.
 */
void aint8_t::setValue( int8_t aValue )
{
	int8_t		old = _value;
	while (!__sync_bool_compare_and_swap(&_value, old, aValue)) {
		old = _value;
	}
}


/********************************************************
 *
 *             Useful Operator Methods
 *
 ********************************************************/
/**
 * This casting operator takes the atomic int8_t and maps it's
 * value into a simple int8_t for all those times when you really
 * need that value. This is again a snapshot as the value can change
 * immediately upon return, but it's as good as you'll get.
 */
aint8_t::operator int8_t() const
{
	return getValue();
}


/**
 * These map the aint8_t's value into the other basic data types in
 * the stdint.h package.
 */
aint8_t::operator int() const
{
	return getValue();
}


aint8_t::operator bool() const
{
	return (getValue() != 0);
}


aint8_t::operator uint8_t() const
{
	return getValue();
}


aint8_t::operator uint16_t() const
{
	return getValue();
}


aint8_t::operator int16_t() const
{
	return getValue();
}


aint8_t::operator uint32_t() const
{
	return getValue();
}


aint8_t::operator uint64_t() const
{
	return getValue();
}


aint8_t::operator int64_t() const
{
	return getValue();
}


/**
 * These are the prefix and postfix increment operators, in that
 * order, and the first returns a reference to the same instance
 * while the latter has to receive a copy prior to the increment.
 */
aint8_t & aint8_t::operator++()
{
	__sync_add_and_fetch(&_value, 1);
	return *this;
}


aint8_t aint8_t::operator++(int)
{
	aint8_t		prev(*this);
	__sync_add_and_fetch(&_value, 1);
	return prev;
}


/**
 * These are the prefix and postfix decrement operators, in that
 * order, and the first returns a reference to the same instance
 * while the latter has to receive a copy prior to the decrement.
 */
aint8_t & aint8_t::operator--()
{
	__sync_sub_and_fetch(&_value, 1);
	return *this;
}


aint8_t aint8_t::operator--(int)
{
	aint8_t		prev(*this);
	__sync_sub_and_fetch(&_value, 1);
	return prev;
}


/**
 * These are the standard RHS operators for this guy, and are all
 * handled in a consistent, thread-safe way.
 */
aint8_t & aint8_t::operator+=( bool aValue )
{
	__sync_add_and_fetch(&_value, (aValue ? 1 : 0));
	return *this;
}


aint8_t & aint8_t::operator+=( uint8_t aValue )
{
	__sync_add_and_fetch(&_value, (int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator+=( int8_t aValue )
{
	__sync_add_and_fetch(&_value, aValue);
	return *this;
}


aint8_t & aint8_t::operator+=( uint16_t aValue )
{
	__sync_add_and_fetch(&_value, (int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator+=( int16_t aValue )
{
	__sync_add_and_fetch(&_value, (int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator+=( uint32_t aValue )
{
	__sync_add_and_fetch(&_value, (int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator+=( int32_t aValue )
{
	__sync_add_and_fetch(&_value, (int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator+=( uint64_t aValue )
{
	__sync_add_and_fetch(&_value, (int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator+=( int64_t aValue )
{
	__sync_add_and_fetch(&_value, (int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator-=( bool aValue )
{
	__sync_sub_and_fetch(&_value, (aValue ? 1 : 0));
	return *this;
}


aint8_t & aint8_t::operator-=( uint8_t aValue )
{
	__sync_sub_and_fetch(&_value, (int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator-=( int8_t aValue )
{
	__sync_sub_and_fetch(&_value, aValue);
	return *this;
}


aint8_t & aint8_t::operator-=( uint16_t aValue )
{
	__sync_sub_and_fetch(&_value, (int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator-=( int16_t aValue )
{
	__sync_sub_and_fetch(&_value, (int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator-=( uint32_t aValue )
{
	__sync_sub_and_fetch(&_value, (int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator-=( int32_t aValue )
{
	__sync_sub_and_fetch(&_value, (int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator-=( uint64_t aValue )
{
	__sync_sub_and_fetch(&_value, (int8_t)aValue);
	return *this;
}


aint8_t & aint8_t::operator-=( int64_t aValue )
{
	__sync_sub_and_fetch(&_value, (int8_t)aValue);
	return *this;
}


/**
 * These versions are the same operators, but using the components
 * in this package as opposed to the basic data types they are modeled
 * after.
 */
aint8_t & aint8_t::operator+=( const abool & aValue )
{
	__sync_add_and_fetch(&_value, (aValue.getValue() ? 1 : 0));
	return *this;
}


aint8_t & aint8_t::operator+=( const auint8_t & aValue )
{
	__sync_add_and_fetch(&_value, (int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator+=( const aint8_t & aValue )
{
	__sync_add_and_fetch(&_value, aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator+=( const auint16_t & aValue )
{
	__sync_add_and_fetch(&_value, (int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator+=( const aint16_t & aValue )
{
	__sync_add_and_fetch(&_value, (int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator+=( const auint32_t & aValue )
{
	__sync_add_and_fetch(&_value, (int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator+=( const aint32_t & aValue )
{
	__sync_add_and_fetch(&_value, (int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator+=( const auint64_t & aValue )
{
	__sync_add_and_fetch(&_value, (int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator+=( const aint64_t & aValue )
{
	__sync_add_and_fetch(&_value, (int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator-=( const abool & aValue )
{
	__sync_sub_and_fetch(&_value, (aValue.getValue() ? 1 : 0));
	return *this;
}


aint8_t & aint8_t::operator-=( const auint8_t & aValue )
{
	__sync_sub_and_fetch(&_value, (int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator-=( const aint8_t & aValue )
{
	__sync_sub_and_fetch(&_value, aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator-=( const auint16_t & aValue )
{
	__sync_sub_and_fetch(&_value, (int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator-=( const aint16_t & aValue )
{
	__sync_sub_and_fetch(&_value, (int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator-=( const auint32_t & aValue )
{
	__sync_sub_and_fetch(&_value, (int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator-=( const aint32_t & aValue )
{
	__sync_sub_and_fetch(&_value, (int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator-=( const auint64_t & aValue )
{
	__sync_sub_and_fetch(&_value, (int8_t)aValue.getValue());
	return *this;
}


aint8_t & aint8_t::operator-=( const aint64_t & aValue )
{
	__sync_sub_and_fetch(&_value, (int8_t)aValue.getValue());
	return *this;
}


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
bool aint8_t::operator==( const aint8_t & anOther ) const
{
	return (getValue() == anOther.getValue());
}


/**
 * This method checks to see if the two values are not equal to
 * one another based on the values they represent and *not* on the
 * actual pointers themselves. If they are not equal, then this
 * method returns true, otherwise it returns false.
 */
bool aint8_t::operator!=( const aint8_t & anOther ) const
{
	return !operator==(anOther);
}


/**
 * These are the equals and not equals operators for the atomic
 * integer w.r.t. the plain data types in the stdint.h package.
 */
bool aint8_t::operator==( bool aValue ) const
{
	return (getValue() == (aValue ? 1 : 0));
}


bool aint8_t::operator==( uint8_t aValue ) const
{
	return (getValue() == (int8_t)aValue);
}


bool aint8_t::operator==( int8_t aValue ) const
{
	return (getValue() == aValue);
}


bool aint8_t::operator==( uint16_t aValue ) const
{
	return (getValue() == (int8_t)aValue);
}


bool aint8_t::operator==( int16_t aValue ) const
{
	return (getValue() == (int8_t)aValue);
}


bool aint8_t::operator==( uint32_t aValue ) const
{
	return (getValue() == (int8_t)aValue);
}


bool aint8_t::operator==( int32_t aValue ) const
{
	return (getValue() == (int8_t)aValue);
}


bool aint8_t::operator==( uint64_t aValue ) const
{
	return (getValue() == (int8_t)aValue);
}


bool aint8_t::operator==( int64_t aValue ) const
{
	return (getValue() == (int8_t)aValue);
}


bool aint8_t::operator!=( bool aValue ) const
{
	return !operator==(aValue);
}


bool aint8_t::operator!=( uint8_t aValue ) const
{
	return !operator==(aValue);
}


bool aint8_t::operator!=( int8_t aValue ) const
{
	return !operator==(aValue);
}


bool aint8_t::operator!=( uint16_t aValue ) const
{
	return !operator==(aValue);
}


bool aint8_t::operator!=( int16_t aValue ) const
{
	return !operator==(aValue);
}


bool aint8_t::operator!=( uint32_t aValue ) const
{
	return !operator==(aValue);
}


bool aint8_t::operator!=( int32_t aValue ) const
{
	return !operator==(aValue);
}


bool aint8_t::operator!=( uint64_t aValue ) const
{
	return !operator==(aValue);
}


bool aint8_t::operator!=( int64_t aValue ) const
{
	return !operator==(aValue);
}


/**
 * These are the equals and not equals operators for the atomic
 * integer w.r.t. the classes in this package. This is done to make
 * the difference between an atomic and non-atomic version as
 * small as possible.
 */
bool aint8_t::operator==( const abool & aValue ) const
{
	return (getValue() == (aValue.getValue() ? 1 : 0));
}


bool aint8_t::operator==( const auint8_t & aValue ) const
{
	return (getValue() == (int8_t)aValue.getValue());
}


bool aint8_t::operator==( const auint16_t & aValue ) const
{
	return (getValue() == (int8_t)aValue.getValue());
}


bool aint8_t::operator==( const aint16_t & aValue ) const
{
	return (getValue() == (int8_t)aValue.getValue());
}


bool aint8_t::operator==( const auint32_t & aValue ) const
{
	return (getValue() == (int8_t)aValue.getValue());
}


bool aint8_t::operator==( const aint32_t & aValue ) const
{
	return (getValue() == (int8_t)aValue.getValue());
}


bool aint8_t::operator==( const auint64_t & aValue ) const
{
	return (getValue() == (int8_t)aValue.getValue());
}


bool aint8_t::operator==( const aint64_t & aValue ) const
{
	return (getValue() == (int8_t)aValue.getValue());
}


bool aint8_t::operator!=( const abool & aValue ) const
{
	return !operator==(aValue);
}


bool aint8_t::operator!=( const auint8_t & aValue ) const
{
	return !operator==(aValue);
}


bool aint8_t::operator!=( const auint16_t & aValue ) const
{
	return !operator==(aValue);
}


bool aint8_t::operator!=( const aint16_t & aValue ) const
{
	return !operator==(aValue);
}


bool aint8_t::operator!=( const auint32_t & aValue ) const
{
	return !operator==(aValue);
}


bool aint8_t::operator!=( const aint32_t & aValue ) const
{
	return !operator==(aValue);
}


bool aint8_t::operator!=( const auint64_t & aValue ) const
{
	return !operator==(aValue);
}


bool aint8_t::operator!=( const aint64_t & aValue ) const
{
	return !operator==(aValue);
}


/**
 * These are the rest of the inequalities for the elemental data
 * types that would be in use in code.
 */
bool aint8_t::operator<( uint8_t aValue ) const
{
	return (getValue() < (int8_t)aValue);
}


bool aint8_t::operator<( int8_t aValue ) const
{
	return (getValue() < aValue);
}


bool aint8_t::operator<( uint16_t aValue ) const
{
	return (getValue() < (int16_t)aValue);
}


bool aint8_t::operator<( int16_t aValue ) const
{
	return (getValue() < aValue);
}


bool aint8_t::operator<( uint32_t aValue ) const
{
	return (getValue() < (int32_t)aValue);
}


bool aint8_t::operator<( int32_t aValue ) const
{
	return (getValue() < aValue);
}


bool aint8_t::operator<( uint64_t aValue ) const
{
	return (getValue() < (int64_t)aValue);
}


bool aint8_t::operator<( int64_t aValue ) const
{
	return (getValue() < aValue);
}


bool aint8_t::operator<=( uint8_t aValue ) const
{
	return (getValue() <= (int8_t)aValue);
}


bool aint8_t::operator<=( int8_t aValue ) const
{
	return (getValue() <= aValue);
}


bool aint8_t::operator<=( uint16_t aValue ) const
{
	return (getValue() <= (int16_t)aValue);
}


bool aint8_t::operator<=( int16_t aValue ) const
{
	return (getValue() <= aValue);
}


bool aint8_t::operator<=( uint32_t aValue ) const
{
	return (getValue() <= (int32_t)aValue);
}


bool aint8_t::operator<=( int32_t aValue ) const
{
	return (getValue() <= aValue);
}


bool aint8_t::operator<=( uint64_t aValue ) const
{
	return (getValue() <= (int64_t)aValue);
}


bool aint8_t::operator<=( int64_t aValue ) const
{
	return (getValue() <= aValue);
}


bool aint8_t::operator>( uint8_t aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>( int8_t aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>( uint16_t aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>( int16_t aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>( uint32_t aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>( int32_t aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>( uint64_t aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>( int64_t aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>=( uint8_t aValue ) const
{
	return !operator<(aValue);
}


bool aint8_t::operator>=( int8_t aValue ) const
{
	return !operator<(aValue);
}


bool aint8_t::operator>=( uint16_t aValue ) const
{
	return !operator<(aValue);
}


bool aint8_t::operator>=( int16_t aValue ) const
{
	return !operator<(aValue);
}


bool aint8_t::operator>=( uint32_t aValue ) const
{
	return !operator<(aValue);
}


bool aint8_t::operator>=( int32_t aValue ) const
{
	return !operator<(aValue);
}


bool aint8_t::operator>=( uint64_t aValue ) const
{
	return !operator<(aValue);
}


bool aint8_t::operator>=( int64_t aValue ) const
{
	return !operator<(aValue);
}


/**
 * These are the rest of the inequalities for the other classes
 * in this set that might be used in comparisons.
 */
bool aint8_t::operator<( const auint8_t & aValue ) const
{
	return (getValue() < (int8_t)aValue.getValue());
}


bool aint8_t::operator<( const aint8_t & aValue ) const
{
	return (getValue() < aValue.getValue());
}


bool aint8_t::operator<( const auint16_t & aValue ) const
{
	return (getValue() < (int16_t)aValue.getValue());
}


bool aint8_t::operator<( const aint16_t & aValue ) const
{
	return (getValue() < aValue.getValue());
}


bool aint8_t::operator<( const auint32_t & aValue ) const
{
	return (getValue() < (int32_t)aValue.getValue());
}


bool aint8_t::operator<( const aint32_t & aValue ) const
{
	return (getValue() < aValue.getValue());
}


bool aint8_t::operator<( const auint64_t & aValue ) const
{
	return (getValue() < (int64_t)aValue.getValue());
}


bool aint8_t::operator<( const aint64_t & aValue ) const
{
	return (getValue() < aValue.getValue());
}


bool aint8_t::operator<=( const auint8_t & aValue ) const
{
	return (getValue() <= (int8_t)aValue.getValue());
}


bool aint8_t::operator<=( const aint8_t & aValue ) const
{
	return (getValue() <= aValue.getValue());
}


bool aint8_t::operator<=( const auint16_t & aValue ) const
{
	return (getValue() <= (int16_t)aValue.getValue());
}


bool aint8_t::operator<=( const aint16_t & aValue ) const
{
	return (getValue() <= aValue.getValue());
}


bool aint8_t::operator<=( const auint32_t & aValue ) const
{
	return (getValue() <= (int32_t)aValue.getValue());
}


bool aint8_t::operator<=( const aint32_t & aValue ) const
{
	return (getValue() <= aValue.getValue());
}


bool aint8_t::operator<=( const auint64_t & aValue ) const
{
	return (getValue() <= (int64_t)aValue.getValue());
}


bool aint8_t::operator<=( const aint64_t & aValue ) const
{
	return (getValue() <= aValue.getValue());
}


bool aint8_t::operator>( const auint8_t & aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>( const aint8_t & aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>( const auint16_t & aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>( const aint16_t & aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>( const auint32_t & aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>( const aint32_t & aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>( const auint64_t & aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>( const aint64_t & aValue ) const
{
	return !operator<=(aValue);
}


bool aint8_t::operator>=( const auint8_t & aValue ) const
{
	return !operator<(aValue);
}


bool aint8_t::operator>=( const aint8_t & aValue ) const
{
	return !operator<(aValue);
}


bool aint8_t::operator>=( const auint16_t & aValue ) const
{
	return !operator<(aValue);
}


bool aint8_t::operator>=( const aint16_t & aValue ) const
{
	return !operator<(aValue);
}


bool aint8_t::operator>=( const auint32_t & aValue ) const
{
	return !operator<(aValue);
}


bool aint8_t::operator>=( const aint32_t & aValue ) const
{
	return !operator<(aValue);
}


bool aint8_t::operator>=( const auint64_t & aValue ) const
{
	return !operator<(aValue);
}


bool aint8_t::operator>=( const aint64_t & aValue ) const
{
	return !operator<(aValue);
}


/**
 * For debugging purposes, let's make it easy for the user to stream
 * out this value. It basically is just the value of the base data type
 * and puts it into the stream.
 */
std::ostream & operator<<( std::ostream & aStream, const auint8_t & aValue )
{
	aStream << aValue.getValue();
	return aStream;
}


std::ostream & operator<<( std::ostream & aStream, const aint8_t & aValue )
{
	aStream << aValue.getValue();
	return aStream;
}
