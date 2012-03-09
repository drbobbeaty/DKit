/**
 * atomic.h - this file defines the classes of atomic values that mimic the
 *            datatypes found in the GCC stdint.h header file: the signed
 *            and unsigned integers of different sizes. These are nice
 *            because in a mixed 32- and 64-bit environment, these datatypes
 *            retain their size and max/min values. The classes are to be
 *            treated like regular datatypes in stdint.h - just when they
 *            are read/written, the operations are atomic in nature.
 */
#ifndef __DKIT_ATOMIC_H
#define __DKIT_ATOMIC_H

//	System Headers

//	Third-Party Headers

//	Other Headers
#include "abool.h"
#include "aint8.h"
#include "aint16.h"
#include "aint32.h"
#include "aint64.h"

//	Forward Declarations

//	Public Constants

//	Public Datatypes

//	Public Data Constants

#endif		// __DKIT_ATOMIC_H
