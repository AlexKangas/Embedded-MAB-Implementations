#pragma once

#include <stdint.h>
#define FRACTIONAL_BITS 16 //Number of fractional bits of the fixed point number

// Datatype for the fixed point number
typedef uint32_t fix16_t;




static const fix16_t fix16_maximum  = 0x7FFFFFFF; /*!< the maximum value of fix16_t */
static const fix16_t fix16_minimum  = 0x80000000; /*!< the minimum value of fix16_t */
static const fix16_t fix16_overflow = 0x80000000; /*!< the value used to indicate overflows when FIXMATH_NO_OVERFLOW is not specified */



// Multiplies two fixed point numbers
fix16_t fix16_mul(fix16_t inArg0, fix16_t inArg1);

// Divides two fixed point numbers
fix16_t fix16_div(fix16_t a, fix16_t b);

// Calculates the square root of a value
fix16_t fix16_sqrt(fix16_t inValue);
