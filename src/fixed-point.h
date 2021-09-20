#pragma once

#include <stdint.h>
#define FRACTIONAL_BITS 16 //Number of fractional bits of the fixed point number

// Datatype for the fixed point number
typedef int32_t fix16_t;
//typedef uint32_t fix16_t;


static const fix16_t FOUR_DIV_PI  = 0x145F3;            /*!< Fix16 value of 4/PI */
static const fix16_t _FOUR_DIV_PI2 = 0xFFFF9840;        /*!< Fix16 value of -4/PI² */
static const fix16_t X4_CORRECTION_COMPONENT = 0x399A; 	/*!< Fix16 value of 0.225 */
static const fix16_t PI_DIV_4 = 0x0000C90F;             /*!< Fix16 value of PI/4 */
static const fix16_t THREE_PI_DIV_4 = 0x00025B2F;       /*!< Fix16 value of 3PI/4 */

static const fix16_t fix16_pi  = 205887;     /*!< fix16_t value of pi */
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
fix16_t fix16_mul(fix16_t inArg0, fix16_t inArg1);

// Divides two fixed point numbers
fix16_t fix16_div(fix16_t a, fix16_t b);

// Calculates the square root of a value
fix16_t fix16_sqrt(fix16_t inValue);

fix16_t fix16_log(fix16_t inValue);

fix16_t fix16_exp(fix16_t inValue);
