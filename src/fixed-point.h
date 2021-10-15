#pragma once

#include <stdint.h>
#define FRACTIONAL_BITS 16 //Number of fractional bits of the fixed point number

// Datatype for the fixed point number
typedef int32_t fix16_t;

static const fix16_t fix16_e   = 178145;     /*!< fix16_t value of e */
static const fix16_t fix16_one = 0x00010000; /*!< fix16_t value of 1 */

static const fix16_t fix16_maximum  = 0x7FFFFFFF; /*!< the maximum value of fix16_t */
//static const fix16_t fix16_maximum  = 0xFFFFFFFF; /*!< the maximum value of fix16_t */
static const fix16_t fix16_minimum  = 0x80000000; /*!< the minimum value of fix16_t */
//static const fix16_t fix16_minimum  = 0x00000000; /*!< the minimum value of fix16_t */
static const fix16_t fix16_overflow = 0x80000000; /*!< the value used to indicate overflows when FIXMATH_NO_OVERFLOW is not specified */

static inline fix16_t fix16_from_int(int a)     { return a * fix16_one; }
//static inline fix16_t fix16_from_int(int a)     { return (uint32_t)(a * fix16_one); }

// Multiplies two fixed point numbers
fix16_t fix16_mul(fix16_t a, fix16_t b);

// Divides two fixed point numbers
fix16_t fix16_div(fix16_t a, fix16_t b);

// Calculates the square root of a value
fix16_t fix16_sqrt(fix16_t inValue);

// Calculates the natural logarithm of a value
fix16_t fix16_log(fix16_t inValue);

// Calculates the exponential function of a value
fix16_t fix16_exp(fix16_t inValue);
