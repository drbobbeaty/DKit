/**
 * aint64.cpp - this file implements the two atomic 64-bit integer classes and
 *              mimics the simple int64_t and uint64_t datatypes in the
 *              stdint.h package. The class is to be treated like the regular
 *              datatypes in C++ - just when they are read/written, the
 *              operations are atomic in nature.
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
 * traditional uint64_t would have.
 */
auint64_t::auint64_t() :
	_value(0)
{
}


/**
 * This constructor takes a traditional uint64_t and creates a new
 * atomic uint64_t based on this value. It's not a copy constructor
 * because we haven't really 'copied' anything - we're just taking
 * the value.
 */
auint64_t::auint64_t( uint64_t aValue ) :
	_value(aValue)
{
}


/**
 * This is the standard copy constructor and needs to be in every
 * class to make sure that we don't have too many things running
 * around.
 */
auint64_t::auint64_t( const auint64_t & anOther ) :
	_value(anOther.getValue())
{
}


/**
 * This is the standard destructor and needs to be virtual to make
 * sure that if we subclass off this the right destructor will be
 * called.
 */
auint64_t::~auint64_t()
{
	// there's nothing much to do here - yet
}


/**
 * When we want to process the result of an equality we need to
 * make sure that we do this right by always having an equals
 * operator on all classes.
 */
auint64_t & auint64_t::operator=( const auint64_t & anOther )
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
auint64_t & auint64_t::operator=( bool aValue )
{
	setValue(aValue ? 1 : 0);
	return *this;
}


auint64_t & auint64_t::operator=( uint8_t aValue )
{
	setValue((uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator=( int8_t aValue )
{
	setValue((uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator=( uint16_t aValue )
{
	setValue((uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator=( int16_t aValue )
{
	setValue((uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator=( uint32_t aValue )
{
	setValue((uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator=( int32_t aValue )
{
	setValue((uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator=( uint64_t aValue )
{
	setValue(aValue);
	return *this;
}


auint64_t & auint64_t::operator=( int64_t aValue )
{
	setValue((uint64_t)aValue);
	return *this;
}


/**
 * Let's also allow this guy to take values from the classes in
 * this package so that there is no unnecessary casting or method
 * calling to get these values into the atomic integer.
 */
auint64_t & auint64_t::operator=( const abool & aValue )
{
	setValue(aValue.getValue() ? 1 : 0);
	return *this;
}


auint64_t & auint64_t::operator=( const auint8_t & aValue )
{
	setValue((uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator=( const aint8_t & aValue )
{
	setValue((uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator=( const auint16_t & aValue )
{
	setValue((uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator=( const aint16_t & aValue )
{
	setValue((uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator=( const auint32_t & aValue )
{
	setValue((uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator=( const aint32_t & aValue )
{
	setValue((uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator=( const aint64_t & aValue )
{
	setValue((uint64_t)aValue.getValue());
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
uint64_t auint64_t::getValue() const
{
	return __sync_or_and_fetch(const_cast<volatile uint64_t *>(&_value), 0x00);
}


/**
 * This method allows a more deliberate setting of the value by
 * the caller. This is typically not used, but it can be in those
 * times when the explicit method call is cleaner to use.
 */
void auint64_t::setValue( uint64_t aValue )
{
	uint64_t		old = _value;
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
 * This casting operator takes the atomic uint64_t and maps it's
 * value into a simple uint64_t for all those times when you really
 * need that value. This is again a snapshot as the value can change
 * immediately upon return, but it's as good as you'll get.
 */
auint64_t::operator uint64_t() const
{
	return getValue();
}


/**
 * These map the auint32_t's value into the other basic data types in
 * the stdint.h package.
 */
auint64_t::operator int() const
{
	return getValue();
}


auint64_t::operator bool() const
{
	return (getValue() != 0);
}


auint64_t::operator uint8_t() const
{
	return getValue();
}


auint64_t::operator int8_t() const
{
	return getValue();
}


auint64_t::operator uint16_t() const
{
	return getValue();
}


auint64_t::operator int16_t() const
{
	return getValue();
}


auint64_t::operator uint32_t() const
{
	return getValue();
}


auint64_t::operator int64_t() const
{
	return getValue();
}


/**
 * These are the prefix and postfix increment operators, in that
 * order, and the first returns a reference to the same instance
 * while the latter has to receive a copy prior to the increment.
 */
auint64_t & auint64_t::operator++()
{
	__sync_add_and_fetch(&_value, 1);
	return *this;
}


auint64_t auint64_t::operator++(int)
{
	auint64_t	pre(*this);
	__sync_add_and_fetch(&_value, 1);
	return pre;
}


/**
 * These are the prefix and postfix decrement operators, in that
 * order, and the first returns a reference to the same instance
 * while the latter has to receive a copy prior to the decrement.
 */
auint64_t & auint64_t::operator--()
{
	__sync_sub_and_fetch(&_value, 1);
	return *this;
}


auint64_t auint64_t::operator--(int)
{
	auint64_t	pre(*this);
	__sync_sub_and_fetch(&_value, 1);
	return pre;
}


/**
 * These are the standard RHS operators for this guy, and are all
 * handled in a consistent, thread-safe way.
 */
auint64_t & auint64_t::operator+=( bool aValue )
{
	__sync_add_and_fetch(&_value, (aValue ? 1 : 0));
	return *this;
}


auint64_t & auint64_t::operator+=( uint8_t aValue )
{
	__sync_add_and_fetch(&_value, (uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator+=( int8_t aValue )
{
	__sync_add_and_fetch(&_value, (uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator+=( uint16_t aValue )
{
	__sync_add_and_fetch(&_value, (uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator+=( int16_t aValue )
{
	__sync_add_and_fetch(&_value, (uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator+=( uint32_t aValue )
{
	__sync_add_and_fetch(&_value, (uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator+=( int32_t aValue )
{
	__sync_add_and_fetch(&_value, (uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator+=( uint64_t aValue )
{
	__sync_add_and_fetch(&_value, aValue);
	return *this;
}


auint64_t & auint64_t::operator+=( int64_t aValue )
{
	__sync_add_and_fetch(&_value, (uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator-=( bool aValue )
{
	__sync_sub_and_fetch(&_value, (aValue ? 1 : 0));
	return *this;
}


auint64_t & auint64_t::operator-=( uint8_t aValue )
{
	__sync_sub_and_fetch(&_value, (uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator-=( int8_t aValue )
{
	__sync_sub_and_fetch(&_value, (uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator-=( uint16_t aValue )
{
	__sync_sub_and_fetch(&_value, (uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator-=( int16_t aValue )
{
	__sync_sub_and_fetch(&_value, (uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator-=( uint32_t aValue )
{
	__sync_sub_and_fetch(&_value, (uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator-=( int32_t aValue )
{
	__sync_sub_and_fetch(&_value, (uint64_t)aValue);
	return *this;
}


auint64_t & auint64_t::operator-=( uint64_t aValue )
{
	__sync_sub_and_fetch(&_value, aValue);
	return *this;
}


auint64_t & auint64_t::operator-=( int64_t aValue )
{
	__sync_sub_and_fetch(&_value, (uint64_t)aValue);
	return *this;
}


/**
 * These versions are the same operators, but using the components
 * in this package as opposed to the basic data types they are modeled
 * after.
 */
auint64_t & auint64_t::operator+=( const abool & aValue )
{
	__sync_add_and_fetch(&_value, (aValue.getValue() ? 1 : 0));
	return *this;
}


auint64_t & auint64_t::operator+=( const auint8_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator+=( const aint8_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator+=( const auint16_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator+=( const aint16_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator+=( const auint32_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator+=( const aint32_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator+=( const auint64_t & aValue )
{
	__sync_add_and_fetch(&_value, aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator+=( const aint64_t & aValue )
{
	__sync_add_and_fetch(&_value, (uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator-=( const abool & aValue )
{
	__sync_sub_and_fetch(&_value, (aValue.getValue() ? 1 : 0));
	return *this;
}


auint64_t & auint64_t::operator-=( const auint8_t & aValue )
{
	__sync_sub_and_fetch(&_value, (uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator-=( const aint8_t & aValue )
{
	__sync_sub_and_fetch(&_value, (uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator-=( const auint16_t & aValue )
{
	__sync_sub_and_fetch(&_value, (uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator-=( const aint16_t & aValue )
{
	__sync_sub_and_fetch(&_value, (uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator-=( const auint32_t & aValue )
{
	__sync_sub_and_fetch(&_value, (uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator-=( const aint32_t & aValue )
{
	__sync_sub_and_fetch(&_value, (uint64_t)aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator-=( const auint64_t & aValue )
{
	__sync_sub_and_fetch(&_value, aValue.getValue());
	return *this;
}


auint64_t & auint64_t::operator-=( const aint64_t & aValue )
{
	__sync_sub_and_fetch(&_value, (uint64_t)aValue.getValue());
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
bool auint64_t::operator==( const auint64_t & anOther ) const
{
	return (getValue() == anOther.getValue());
}


/**
 * This method checks to see if the two values are not equal to
 * one another based on the values they represent and *not* on the
 * actual pointers themselves. If they are not equal, then this
 * method returns true, otherwise it returns false.
 */
bool auint64_t::operator!=( const auint64_t & anOther ) const
{
	return !operator==(anOther);
}


/**
 * These are the equals and not equals operators for the atomic
 * integer w.r.t. the plain data types in the stdint.h package.
 */
bool auint64_t::operator==( bool aValue ) const
{
	return (getValue() == (aValue ? 1 : 0));
}


bool auint64_t::operator==( uint8_t aValue ) const
{
	return (getValue() == (uint64_t)aValue);
}


bool auint64_t::operator==( int8_t aValue ) const
{
	return (getValue() == (uint64_t)aValue);
}


bool auint64_t::operator==( uint16_t aValue ) const
{
	return (getValue() == (uint64_t)aValue);
}


bool auint64_t::operator==( int16_t aValue ) const
{
	return (getValue() == (uint64_t)aValue);
}


bool auint64_t::operator==( uint32_t aValue ) const
{
	return (getValue() == (uint64_t)aValue);
}


bool auint64_t::operator==( int32_t aValue ) const
{
	return (getValue() == (uint64_t)aValue);
}


bool auint64_t::operator==( uint64_t aValue ) const
{
	return (getValue() == aValue);
}


bool auint64_t::operator==( int64_t aValue ) const
{
	return (getValue() == (uint64_t)aValue);
}


bool auint64_t::operator!=( bool aValue ) const
{
	return !operator==(aValue);
}


bool auint64_t::operator!=( uint8_t aValue ) const
{
	return !operator==(aValue);
}


bool auint64_t::operator!=( int8_t aValue ) const
{
	return !operator==(aValue);
}


bool auint64_t::operator!=( uint16_t aValue ) const
{
	return !operator==(aValue);
}


bool auint64_t::operator!=( int16_t aValue ) const
{
	return !operator==(aValue);
}


bool auint64_t::operator!=( uint32_t aValue ) const
{
	return !operator==(aValue);
}


bool auint64_t::operator!=( int32_t aValue ) const
{
	return !operator==(aValue);
}


bool auint64_t::operator!=( uint64_t aValue ) const
{
	return !operator==(aValue);
}


bool auint64_t::operator!=( int64_t aValue ) const
{
	return !operator==(aValue);
}


/**
 * These are the equals and not equals operators for the atomic
 * integer w.r.t. the classes in this package. This is done to make
 * the difference between an atomic and non-atomic version as
 * small as possible.
 */
bool auint64_t::operator==( const abool & aValue ) const
{
	return (getValue() == (aValue.getValue() ? 1 : 0));
}


bool auint64_t::operator==( const auint8_t & aValue ) const
{
	return (getValue() == (uint64_t)aValue.getValue());
}


bool auint64_t::operator==( const aint8_t & aValue ) const
{
	return (getValue() == (uint64_t)aValue.getValue());
}


bool auint64_t::operator==( const auint16_t & aValue ) const
{
	return (getValue() == (uint64_t)aValue.getValue());
}


bool auint64_t::operator==( const aint16_t & aValue ) const
{
	return (getValue() == (uint64_t)aValue.getValue());
}


bool auint64_t::operator==( const auint32_t & aValue ) const
{
	return (getValue() == (uint64_t)aValue.getValue());
}


bool auint64_t::operator==( const aint32_t & aValue ) const
{
	return (getValue() == (uint64_t)aValue.getValue());
}


bool auint64_t::operator==( const aint64_t & aValue ) const
{
	return (getValue() == (uint64_t)aValue.getValue());
}


bool auint64_t::operator!=( const abool & aValue ) const
{
	return !operator==(aValue);
}


bool auint64_t::operator!=( const auint8_t & aValue ) const
{
	return !operator==(aValue);
}


bool auint64_t::operator!=( const aint8_t & aValue ) const
{
	return !operator==(aValue);
}


bool auint64_t::operator!=( const auint16_t & aValue ) const
{
	return !operator==(aValue);
}


bool auint64_t::operator!=( const aint16_t & aValue ) const
{
	return !operator==(aValue);
}


bool auint64_t::operator!=( const auint32_t & aValue ) const
{
	return !operator==(aValue);
}


bool auint64_t::operator!=( const aint32_t & aValue ) const
{
	return !operator==(aValue);
}


bool auint64_t::operator!=( const aint64_t & aValue ) const
{
	return !operator==(aValue);
}






/********************************************************
 *
 *                Constructors/Destructor
 *
 ********************************************************/
/**
 * This is the default constructor that sets up the value of
 * zero (0) in the variable. This is the same default that a
 * traditional int64_t would have.
 */
aint64_t::aint64_t() :
	_value(0)
{
}


/**
 * This constructor takes a traditional int64_t and creates a new
 * atomic int64_t based on this value. It's not a copy constructor
 * because we haven't really 'copied' anything - we're just taking
 * the value.
 */
aint64_t::aint64_t( int64_t aValue ) :
	_value(aValue)
{
}


/**
 * This is the standard copy constructor and needs to be in every
 * class to make sure that we don't have too many things running
 * around.
 */
aint64_t::aint64_t( const aint64_t & anOther ) :
	_value(anOther.getValue())
{
}


/**
 * This is the standard destructor and needs to be virtual to make
 * sure that if we subclass off this the right destructor will be
 * called.
 */
aint64_t::~aint64_t()
{
	// there's nothing much to do here - yet
}


/**
 * When we want to process the result of an equality we need to
 * make sure that we do this right by always having an equals
 * operator on all classes.
 */
aint64_t & aint64_t::operator=( const aint64_t & anOther )
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
aint64_t & aint64_t::operator=( bool aValue )
{
	setValue(aValue ? 1 : 0);
	return *this;
}


aint64_t & aint64_t::operator=( uint8_t aValue )
{
	setValue((int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator=( int8_t aValue )
{
	setValue((int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator=( uint16_t aValue )
{
	setValue((int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator=( int16_t aValue )
{
	setValue((int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator=( uint32_t aValue )
{
	setValue((int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator=( int32_t aValue )
{
	setValue((int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator=( uint64_t aValue )
{
	setValue((int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator=( int64_t aValue )
{
	setValue(aValue);
	return *this;
}


/**
 * Let's also allow this guy to take values from the classes in
 * this package so that there is no unnecessary casting or method
 * calling to get these values into the atomic integer.
 */
aint64_t & aint64_t::operator=( const abool & aValue )
{
	setValue(aValue.getValue() ? 1 : 0);
	return *this;
}


aint64_t & aint64_t::operator=( const auint8_t & aValue )
{
	setValue((int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator=( const aint8_t & aValue )
{
	setValue((int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator=( const auint16_t & aValue )
{
	setValue((int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator=( const aint16_t & aValue )
{
	setValue((int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator=( const auint32_t & aValue )
{
	setValue((int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator=( const aint32_t & aValue )
{
	setValue((int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator=( const auint64_t & aValue )
{
	setValue((int64_t)aValue.getValue());
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
int64_t aint64_t::getValue() const
{
	return __sync_or_and_fetch(const_cast<volatile int64_t *>(&_value), 0x00);
}


/**
 * This method allows a more deliberate setting of the value by
 * the caller. This is typically not used, but it can be in those
 * times when the explicit method call is cleaner to use.
 */
void aint64_t::setValue( int64_t aValue )
{
	int64_t			old = _value;
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
 * This casting operator takes the atomic int64_t and maps it's
 * value into a simple int64_t for all those times when you really
 * need that value. This is again a snapshot as the value can change
 * immediately upon return, but it's as good as you'll get.
 */
aint64_t::operator int64_t() const
{
	return getValue();
}


/**
 * These map the aint64_t's value into the other basic data types in
 * the stdint.h package.
 */
aint64_t::operator int() const
{
	return getValue();
}


aint64_t::operator bool() const
{
	return (getValue() != 0);
}


aint64_t::operator uint8_t() const
{
	return getValue();
}


aint64_t::operator int8_t() const
{
	return getValue();
}


aint64_t::operator uint16_t() const
{
	return getValue();
}


aint64_t::operator int16_t() const
{
	return getValue();
}


aint64_t::operator uint32_t() const
{
	return getValue();
}


aint64_t::operator uint64_t() const
{
	return getValue();
}


/**
 * These are the prefix and postfix increment operators, in that
 * order, and the first returns a reference to the same instance
 * while the latter has to receive a copy prior to the increment.
 */
aint64_t & aint64_t::operator++()
{
	__sync_add_and_fetch(&_value, 1);
	return *this;
}


aint64_t aint64_t::operator++(int)
{
	aint64_t	pre(*this);
	__sync_add_and_fetch(&_value, 1);
	return pre;
}


/**
 * These are the prefix and postfix decrement operators, in that
 * order, and the first returns a reference to the same instance
 * while the latter has to receive a copy prior to the decrement.
 */
aint64_t & aint64_t::operator--()
{
	__sync_sub_and_fetch(&_value, 1);
	return *this;
}


aint64_t aint64_t::operator--(int)
{
	aint64_t	pre(*this);
	__sync_sub_and_fetch(&_value, 1);
	return pre;
}


/**
 * These are the standard RHS operators for this guy, and are all
 * handled in a consistent, thread-safe way.
 */
aint64_t & aint64_t::operator+=( bool aValue )
{
	__sync_add_and_fetch(&_value, (aValue ? 1 : 0));
	return *this;
}


aint64_t & aint64_t::operator+=( uint8_t aValue )
{
	__sync_add_and_fetch(&_value, (int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator+=( int8_t aValue )
{
	__sync_add_and_fetch(&_value, (int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator+=( uint16_t aValue )
{
	__sync_add_and_fetch(&_value, (int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator+=( int16_t aValue )
{
	__sync_add_and_fetch(&_value, (int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator+=( uint32_t aValue )
{
	__sync_add_and_fetch(&_value, (int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator+=( int32_t aValue )
{
	__sync_add_and_fetch(&_value, (int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator+=( uint64_t aValue )
{
	__sync_add_and_fetch(&_value, (int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator+=( int64_t aValue )
{
	__sync_add_and_fetch(&_value, aValue);
	return *this;
}


aint64_t & aint64_t::operator-=( bool aValue )
{
	__sync_sub_and_fetch(&_value, (aValue ? 1 : 0));
	return *this;
}


aint64_t & aint64_t::operator-=( uint8_t aValue )
{
	__sync_sub_and_fetch(&_value, (int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator-=( int8_t aValue )
{
	__sync_sub_and_fetch(&_value, (int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator-=( uint16_t aValue )
{
	__sync_sub_and_fetch(&_value, (int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator-=( int16_t aValue )
{
	__sync_sub_and_fetch(&_value, (int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator-=( uint32_t aValue )
{
	__sync_sub_and_fetch(&_value, (int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator-=( int32_t aValue )
{
	__sync_sub_and_fetch(&_value, (int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator-=( uint64_t aValue )
{
	__sync_sub_and_fetch(&_value, (int64_t)aValue);
	return *this;
}


aint64_t & aint64_t::operator-=( int64_t aValue )
{
	__sync_sub_and_fetch(&_value, aValue);
	return *this;
}


/**
 * These versions are the same operators, but using the components
 * in this package as opposed to the basic data types they are modeled
 * after.
 */
aint64_t & aint64_t::operator+=( const abool & aValue )
{
	__sync_add_and_fetch(&_value, (aValue.getValue() ? 1 : 0));
	return *this;
}


aint64_t & aint64_t::operator+=( const auint8_t & aValue )
{
	__sync_add_and_fetch(&_value, (int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator+=( const aint8_t & aValue )
{
	__sync_add_and_fetch(&_value, (int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator+=( const auint16_t & aValue )
{
	__sync_add_and_fetch(&_value, (int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator+=( const aint16_t & aValue )
{
	__sync_add_and_fetch(&_value, (int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator+=( const auint32_t & aValue )
{
	__sync_add_and_fetch(&_value, (int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator+=( const aint32_t & aValue )
{
	__sync_add_and_fetch(&_value, (int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator+=( const auint64_t & aValue )
{
	__sync_add_and_fetch(&_value, (int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator+=( const aint64_t & aValue )
{
	__sync_add_and_fetch(&_value, aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator-=( const abool & aValue )
{
	__sync_sub_and_fetch(&_value, (aValue.getValue() ? 1 : 0));
	return *this;
}


aint64_t & aint64_t::operator-=( const auint8_t & aValue )
{
	__sync_sub_and_fetch(&_value, (int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator-=( const aint8_t & aValue )
{
	__sync_sub_and_fetch(&_value, (int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator-=( const auint16_t & aValue )
{
	__sync_sub_and_fetch(&_value, (int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator-=( const aint16_t & aValue )
{
	__sync_sub_and_fetch(&_value, (int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator-=( const auint32_t & aValue )
{
	__sync_sub_and_fetch(&_value, (int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator-=( const aint32_t & aValue )
{
	__sync_sub_and_fetch(&_value, (int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator-=( const auint64_t & aValue )
{
	__sync_sub_and_fetch(&_value, (int64_t)aValue.getValue());
	return *this;
}


aint64_t & aint64_t::operator-=( const aint64_t & aValue )
{
	__sync_sub_and_fetch(&_value, aValue.getValue());
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
bool aint64_t::operator==( const aint64_t & anOther ) const
{
	return (getValue() == anOther.getValue());
}


/**
 * This method checks to see if the two values are not equal to
 * one another based on the values they represent and *not* on the
 * actual pointers themselves. If they are not equal, then this
 * method returns true, otherwise it returns false.
 */
bool aint64_t::operator!=( const aint64_t & anOther ) const
{
	return !operator==(anOther);
}


/**
 * These are the equals and not equals operators for the atomic
 * bool w.r.t. the plain data types in the stdint.h package.
 */
bool aint64_t::operator==( bool aValue ) const
{
	return (getValue() == (aValue ? 1 : 0));
}


bool aint64_t::operator==( uint8_t aValue ) const
{
	return (getValue() == (int64_t)aValue);
}


bool aint64_t::operator==( int8_t aValue ) const
{
	return (getValue() == (int64_t)aValue);
}


bool aint64_t::operator==( uint16_t aValue ) const
{
	return (getValue() == (int64_t)aValue);
}


bool aint64_t::operator==( int16_t aValue ) const
{
	return (getValue() == (int64_t)aValue);
}


bool aint64_t::operator==( uint32_t aValue ) const
{
	return (getValue() == (int64_t)aValue);
}


bool aint64_t::operator==( int32_t aValue ) const
{
	return (getValue() == (int64_t)aValue);
}


bool aint64_t::operator==( uint64_t aValue ) const
{
	return (getValue() == (int64_t)aValue);
}


bool aint64_t::operator==( int64_t aValue ) const
{
	return (getValue() == aValue);
}


bool aint64_t::operator!=( bool aValue ) const
{
	return !operator==(aValue);
}


bool aint64_t::operator!=( uint8_t aValue ) const
{
	return !operator==(aValue);
}


bool aint64_t::operator!=( int8_t aValue ) const
{
	return !operator==(aValue);
}


bool aint64_t::operator!=( uint16_t aValue ) const
{
	return !operator==(aValue);
}


bool aint64_t::operator!=( int16_t aValue ) const
{
	return !operator==(aValue);
}


bool aint64_t::operator!=( uint32_t aValue ) const
{
	return !operator==(aValue);
}


bool aint64_t::operator!=( int32_t aValue ) const
{
	return !operator==(aValue);
}


bool aint64_t::operator!=( uint64_t aValue ) const
{
	return !operator==(aValue);
}


bool aint64_t::operator!=( int64_t aValue ) const
{
	return !operator==(aValue);
}


/**
 * These are the equals and not equals operators for the atomic
 * integer w.r.t. the classes in this package. This is done to make
 * the difference between an atomic and non-atomic version as
 * small as possible.
 */
bool aint64_t::operator==( const abool & aValue ) const
{
	return (getValue() == (aValue.getValue() ? 1 : 0));
}


bool aint64_t::operator==( const auint8_t & aValue ) const
{
	return (getValue() == (int64_t)aValue.getValue());
}


bool aint64_t::operator==( const aint8_t & aValue ) const
{
	return (getValue() == (int64_t)aValue.getValue());
}


bool aint64_t::operator==( const auint16_t & aValue ) const
{
	return (getValue() == (int64_t)aValue.getValue());
}


bool aint64_t::operator==( const aint16_t & aValue ) const
{
	return (getValue() == (int64_t)aValue.getValue());
}


bool aint64_t::operator==( const auint32_t & aValue ) const
{
	return (getValue() == (int64_t)aValue.getValue());
}


bool aint64_t::operator==( const aint32_t & aValue ) const
{
	return (getValue() == (int64_t)aValue.getValue());
}


bool aint64_t::operator==( const auint64_t & aValue ) const
{
	return (getValue() == (int64_t)aValue.getValue());
}


bool aint64_t::operator!=( const abool & aValue ) const
{
	return !operator==(aValue);
}


bool aint64_t::operator!=( const auint8_t & aValue ) const
{
	return !operator==(aValue);
}


bool aint64_t::operator!=( const aint8_t & aValue ) const
{
	return !operator==(aValue);
}


bool aint64_t::operator!=( const auint16_t & aValue ) const
{
	return !operator==(aValue);
}


bool aint64_t::operator!=( const aint16_t & aValue ) const
{
	return !operator==(aValue);
}


bool aint64_t::operator!=( const auint32_t & aValue ) const
{
	return !operator==(aValue);
}


bool aint64_t::operator!=( const aint32_t & aValue ) const
{
	return !operator==(aValue);
}


bool aint64_t::operator!=( const auint64_t & aValue ) const
{
	return !operator==(aValue);
}


/**
 * For debugging purposes, let's make it easy for the user to stream
 * out this value. It basically is just the value of the base data type
 * and puts it into the stream.
 */
std::ostream & operator<<( std::ostream & aStream, const auint64_t & aValue )
{
	aStream << aValue.getValue();
	return aStream;
}


std::ostream & operator<<( std::ostream & aStream, const aint64_t & aValue )
{
	aStream << aValue.getValue();
	return aStream;
}
