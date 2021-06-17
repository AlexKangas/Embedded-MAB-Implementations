#pragma once

#include <stdint.h>
#define FRACTIONAL_BITS 16 //Number of fractional bits used for scaling


typedef uint32_t fix16_t;




static const fix16_t fix16_maximum  = 0x7FFFFFFF; /*!< the maximum value of fix16_t */
static const fix16_t fix16_minimum  = 0x80000000; /*!< the minimum value of fix16_t */
static const fix16_t fix16_overflow = 0x80000000; /*!< the value used to indicate overflows when FIXMATH_NO_OVERFLOW is not specified */




fix16_t fix16_mul(fix16_t inArg0, fix16_t inArg1);

fix16_t fix16_div(fix16_t a, fix16_t b);


fix16_t fix16_sqrt(fix16_t inValue);
