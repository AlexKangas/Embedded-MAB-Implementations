#pragma once
#include <stdint.h>

// The simplest integer square root
uint32_t isqrt_simple(uint32_t a);

/* An improved version.  Using no multiplications
 * it's ideal for small microcontrollers. Feel
 * free to change the word length as desired.
 */

uint32_t isqrt_improved(uint32_t a);
