/**
 * abool.cpp - this file implemetns the atomic bool class and mimics the simple
 *             bool datatype in the C spec. The class is to be treated like
 *             the regular datatypes in C++ - just when they are read/written,
 *             the operations are atomic in nature.
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
 * false (0) in the variable. This is the same default that a
 * traditional boolean would have.
 */
abool::abool() :
	_value(0)
{
}


/**
 * This constructor takes a traditional bool and creates a new
 * atomic bool based on this value. It's not a copy constructor
 * because we haven't really 'copied' anything - we're just taking
 * the value.
 */
abool::abool( bool aValue ) :
	_value(aValue ? 1 : 0)
{
}


/**
 * This is the standard copy constructor and needs to be in every
 * class to make sure that we don't have too many things running
 * around.
 */
abool::abool( const abool & anOther ) :
	_value(anOther.getValue() ? 1 : 0)
{
}


/**
 * This is the standard destructor and needs to be virtual to make
 * sure that if we subclass off this the right destructor will be
 * called.
 */
abool::~abool()
{
	// nothing much to do here - yet
}


/**
 * When we want to process the result of an equality we need to
 * make sure that we do this right by always having an equals
 * operator on all classes.
 */
abool & abool::operator=( const abool & anOther )
{
	if (this != & anOther) {
		setValue(anOther.getValue());
	}
	return *this;
}


/**
 * Let's also allow this value to be taken from one of the other
 * data types defined in stdint.h - again, according to the zero/
 * non-zero rule for booleans. This just makes it very easy to work
 * in this atomic bool into existing code.
 */
abool & abool::operator=( bool aValue )
{
	setValue(aValue);
	return *this;
}


abool & abool::operator=( uint8_t aValue )
{
	setValue(aValue != 0);
	return *this;
}


abool & abool::operator=( int8_t aValue )
{
	setValue(aValue != 0);
	return *this;
}


abool & abool::operator=( uint16_t aValue )
{
	setValue(aValue != 0);
	return *this;
}


abool & abool::operator=( int16_t aValue )
{
	setValue(aValue != 0);
	return *this;
}


abool & abool::operator=( uint32_t aValue )
{
	setValue(aValue != 0);
	return *this;
}


abool & abool::operator=( int32_t aValue )
{
	setValue(aValue != 0);
	return *this;
}


abool & abool::operator=( uint64_t aValue )
{
	setValue(aValue != 0);
	return *this;
}


abool & abool::operator=( int64_t aValue )
{
	setValue(aValue != 0);
	return *this;
}


/**
 * Let's also allow this guy to take values from the classes in
 * this package so that there is no unnecessary casting or method
 * calling to get these values into the atomic boolean.
 */
abool & abool::operator=( const auint8_t & aValue )
{
	setValue((uint8_t)aValue != 0);
	return *this;
}


abool & abool::operator=( const aint8_t & aValue )
{
	setValue((int8_t)aValue != 0);
	return *this;
}


abool & abool::operator=( const auint16_t & aValue )
{
	setValue((uint16_t)aValue != 0);
	return *this;
}


abool & abool::operator=( const aint16_t & aValue )
{
	setValue((int16_t)aValue != 0);
	return *this;
}


abool & abool::operator=( const auint32_t & aValue )
{
	setValue((uint32_t)aValue != 0);
	return *this;
}


abool & abool::operator=( const aint32_t & aValue )
{
	setValue((int32_t)aValue != 0);
	return *this;
}


abool & abool::operator=( const auint64_t & aValue )
{
	setValue((uint64_t)aValue != 0);
	return *this;
}


abool & abool::operator=( const aint64_t & aValue )
{
	setValue((int64_t)aValue != 0);
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
bool abool::getValue() const
{
	return (__sync_or_and_fetch(const_cast<volatile uint8_t *>(&_value), 0x00) == 1);
}


/**
 * This method allows a more deliberate setting of the value by
 * the caller. This is typically not used, but it can be in those
 * times when the explicit method call is cleaner to use.
 */
void abool::setValue( bool aValue )
{
	if (aValue) {
		__sync_or_and_fetch(&_value, 0x01);
	} else {
		__sync_and_and_fetch(&_value, 0x00);
	}
}


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
abool::operator bool() const
{
	return (__sync_or_and_fetch(const_cast<volatile uint8_t *>(&_value), 0x00) == 1);
}


/**
 * These map the abool's value into the other basic data types in
 * the stdint.h package.
 */
abool::operator int() const
{
	return (getValue() ? 1 : 0);
}


abool::operator uint8_t() const
{
	return (getValue() ? 1 : 0);
}


abool::operator int8_t() const
{
	return (getValue() ? 1 : 0);
}


abool::operator uint16_t() const
{
	return (getValue() ? 1 : 0);
}


abool::operator int16_t() const
{
	return (getValue() ? 1 : 0);
}


abool::operator uint32_t() const
{
	return (getValue() ? 1 : 0);
}


abool::operator uint64_t() const
{
	return (getValue() ? 1 : 0);
}


abool::operator int64_t() const
{
	return (getValue() ? 1 : 0);
}


/**
 * These are the prefix and postfix increment operators, in that
 * order, and the first returns a reference to the same instance
 * while the latter has to receive a copy prior to the increment.
 * In the case of the boolean, these simply "flip" the state of the
 * bool from true->false->true, etc.
 */
abool & abool::operator++()
{
	__sync_xor_and_fetch(&_value, 0x01);
	return *this;
}


abool abool::operator++(int)
{
	abool	pre(*this);
	__sync_xor_and_fetch(&_value, 0x01);
	return pre;
}


/**
 * These are the prefix and postfix decrement operators, in that
 * order, and the first returns a reference to the same instance
 * while the latter has to receive a copy prior to the decrement.
 * In the case of the boolean, these simply "flip" the state of the
 * bool from true->false->true, etc.
 */
abool & abool::operator--()
{
	__sync_xor_and_fetch(&_value, 0x01);
	return *this;
}


abool abool::operator--(int)
{
	abool	pre(*this);
	__sync_xor_and_fetch(&_value, 0x01);
	return pre;
}


/**
 * These are the standard RHS operators for this guy, and are all
 * handled in a consistent, thread-safe way. The way this works for
 * a boolean is that each move "flips" the state, so a simple
 * modulo 2 gives us whether or not to flip this value at all.
 */
abool & abool::operator+=( bool aValue )
{
	if (aValue) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( uint8_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( int8_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( uint16_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( int16_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( uint32_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( int32_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( uint64_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( int64_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( bool aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( uint8_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( int8_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( uint16_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( int16_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( uint32_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( int32_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( uint64_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( int64_t aValue )
{
	if ((aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


/**
 * These versions are the same operators, but using the components
 * in this package as opposed to the basic data types they are modeled
 * after.
 */
abool & abool::operator+=( const abool & aValue )
{
	if ((bool)aValue) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( const auint8_t & aValue )
{
	if (((uint8_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( const aint8_t & aValue )
{
	if (((int8_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( const auint16_t & aValue )
{
	if (((uint16_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( const aint16_t & aValue )
{
	if (((int16_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( const auint32_t & aValue )
{
	if (((uint32_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( const aint32_t & aValue )
{
	if (((int32_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( const auint64_t & aValue )
{
	if (((uint64_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator+=( const aint64_t & aValue )
{
	if (((int64_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( const abool & aValue )
{
	if ((bool)aValue) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( const auint8_t & aValue )
{
	if (((uint8_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( const aint8_t & aValue )
{
	if (((int8_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( const auint16_t & aValue )
{
	if (((uint16_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( const aint16_t & aValue )
{
	if (((int16_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( const auint32_t & aValue )
{
	if (((uint32_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( const aint32_t & aValue )
{
	if (((int32_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( const auint64_t & aValue )
{
	if (((uint64_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
	return *this;
}


abool & abool::operator-=( const aint64_t & aValue )
{
	if (((int64_t)aValue % 2) > 0) {
		__sync_xor_and_fetch(&_value, 0x01);
	}
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
bool abool::operator==( const abool & anOther ) const
{
	return (getValue() == anOther.getValue());
}


/**
 * This method checks to see if the two values are not equal to
 * one another based on the values they represent and *not* on the
 * actual pointers themselves. If they are not equal, then this
 * method returns true, otherwise it returns false.
 */
bool abool::operator!=( const abool & anOther ) const
{
	return !operator==(anOther);
}


/**
 * These are the equals and not equals operators for the atomic
 * bool w.r.t. the plain data types in the stdint.h package.
 */
bool abool::operator==( bool aValue ) const
{
	return (getValue() == aValue);
}


bool abool::operator==( uint8_t aValue ) const
{
	return operator==(aValue != 0);
}


bool abool::operator==( int8_t aValue ) const
{
	return operator==(aValue != 0);
}


bool abool::operator==( uint16_t aValue ) const
{
	return operator==(aValue != 0);
}


bool abool::operator==( int16_t aValue ) const
{
	return operator==(aValue != 0);
}


bool abool::operator==( uint32_t aValue ) const
{
	return operator==(aValue != 0);
}


bool abool::operator==( int32_t aValue ) const
{
	return operator==(aValue != 0);
}


bool abool::operator==( uint64_t aValue ) const
{
	return operator==(aValue != 0);
}


bool abool::operator==( int64_t aValue ) const
{
	return operator==(aValue != 0);
}


bool abool::operator!=( bool aValue ) const
{
	return !operator==(aValue);
}


bool abool::operator!=( uint8_t aValue ) const
{
	return !operator==(aValue);
}


bool abool::operator!=( int8_t aValue ) const
{
	return !operator==(aValue);
}


bool abool::operator!=( uint16_t aValue ) const
{
	return !operator==(aValue);
}


bool abool::operator!=( int16_t aValue ) const
{
	return !operator==(aValue);
}


bool abool::operator!=( uint32_t aValue ) const
{
	return !operator==(aValue);
}


bool abool::operator!=( int32_t aValue ) const
{
	return !operator==(aValue);
}


bool abool::operator!=( uint64_t aValue ) const
{
	return !operator==(aValue);
}


bool abool::operator!=( int64_t aValue ) const
{
	return !operator==(aValue);
}


/**
 * These are the equals and not equals operators for the atomic
 * bool w.r.t. the classes in this package. This is done to make
 * the difference between an atomic and non-atomic version as
 * small as possible.
 */
bool abool::operator==( const auint8_t & aValue ) const
{
	return operator==((uint8_t)aValue != 0);
}


bool abool::operator==( const aint8_t & aValue ) const
{
	return operator==((int8_t)aValue != 0);
}


bool abool::operator==( const auint16_t & aValue ) const
{
	return operator==((uint16_t)aValue != 0);
}


bool abool::operator==( const aint16_t & aValue ) const
{
	return operator==((int16_t)aValue != 0);
}


bool abool::operator==( const auint32_t & aValue ) const
{
	return operator==((uint32_t)aValue != 0);
}


bool abool::operator==( const aint32_t & aValue ) const
{
	return operator==((int32_t)aValue != 0);
}


bool abool::operator==( const auint64_t & aValue ) const
{
	return operator==((uint64_t)aValue != 0);
}


bool abool::operator==( const aint64_t & aValue ) const
{
	return operator==((int64_t)aValue != 0);
}


bool abool::operator!=( const auint8_t & aValue ) const
{
	return !operator==(aValue);
}


bool abool::operator!=( const aint8_t & aValue ) const
{
	return !operator==(aValue);
}


bool abool::operator!=( const auint16_t & aValue ) const
{
	return !operator==(aValue);
}


bool abool::operator!=( const aint16_t & aValue ) const
{
	return !operator==(aValue);
}


bool abool::operator!=( const auint32_t & aValue ) const
{
	return !operator==(aValue);
}


bool abool::operator!=( const aint32_t & aValue ) const
{
	return !operator==(aValue);
}


bool abool::operator!=( const auint64_t & aValue ) const
{
	return !operator==(aValue);
}


bool abool::operator!=( const aint64_t & aValue ) const
{
	return !operator==(aValue);
}


/**
 * For debugging purposes, let's make it easy for the user to stream
 * out this value. It basically is just the value of the base data type
 * and puts it into the stream.
 */
std::ostream & operator<<( std::ostream & aStream, const abool & aValue )
{
	aStream << (aValue.getValue() ? "true" : "false");
	return aStream;
}
