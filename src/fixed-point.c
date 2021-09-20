#include "fixed-point.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* 64-bit implementation for fix16_mul. Fastest version for e.g. ARM Cortex M3.
 * Performs a 32*32 -> 64bit multiplication. The middle 32 bits are the result,
 * bottom 16 bits are used for rounding, and upper 16 bits are used for overflow
 * detection.
 */
 
#if !defined(FIXMATH_NO_64BIT) && !defined(FIXMATH_OPTIMIZE_8BIT)
fix16_t fix16_mul(fix16_t inArg0, fix16_t inArg1)
{
	int64_t product = (int64_t)inArg0 * inArg1;
	
	#ifndef FIXMATH_NO_OVERFLOW
	// The upper 17 bits should all be the same (the sign).
	uint32_t upper = (product >> 47);
	#endif
	
	if (product < 0)
	{
		#ifndef FIXMATH_NO_OVERFLOW
		if (~upper)
				return fix16_overflow;
		#endif
		
		#ifndef FIXMATH_NO_ROUNDING
		// This adjustment is required in order to round -1/2 correctly
		product--;
		#endif
	}
	else
	{
		#ifndef FIXMATH_NO_OVERFLOW
		if (upper)
				return fix16_overflow;
		#endif
	}
	
	#ifdef FIXMATH_NO_ROUNDING
	return product >> 16;
	#else
	fix16_t result = product >> 16;
	result += (product & 0x8000) >> 15;
	
	return result;
	#endif
}
#endif



/* 32-bit implementation of fix16_div. Fastest version for e.g. ARM Cortex M3.
 * Performs 32-bit divisions repeatedly to reduce the remainder. For this to
 * be efficient, the processor has to have 32-bit hardware division.
 */
#if !defined(FIXMATH_OPTIMIZE_8BIT)
#ifdef __GNUC__
// Count leading zeros, using processor-specific instruction if available.
#define clz(x) (__builtin_clzl(x) - (8 * sizeof(long) - 32))
#else
static uint8_t clz(uint32_t x)
{
	uint8_t result = 0;
	if (x == 0) return 32;
	while (!(x & 0xF0000000)) { result += 4; x <<= 4; }
	while (!(x & 0x80000000)) { result += 1; x <<= 1; }
	return result;
}
#endif

fix16_t fix16_div(fix16_t a, fix16_t b)
{
	// This uses a hardware 32/32 bit division multiple times, until we have
	// computed all the bits in (a<<17)/b. Usually this takes 1-3 iterations.
	
	if (b == 0)
			return fix16_minimum;
	
	uint32_t remainder = (a >= 0) ? a : (-a);
	uint32_t divider = (b >= 0) ? b : (-b);
	uint32_t quotient = 0;
	int bit_pos = 17;
	
	// Kick-start the division a bit.
	// This improves speed in the worst-case scenarios where N and D are large
	// It gets a lower estimate for the result by N/(D >> 17 + 1).
	if (divider & 0xFFF00000)
	{
		uint32_t shifted_div = ((divider >> 17) + 1);
		quotient = remainder / shifted_div;
		remainder -= ((uint64_t)quotient * divider) >> 17;
	}
	
	// If the divider is divisible by 2^n, take advantage of it.
	while (!(divider & 0xF) && bit_pos >= 4)
	{
		divider >>= 4;
		bit_pos -= 4;
	}
	
	while (remainder && bit_pos >= 0)
	{
		// Shift remainder as much as we can without overflowing
		int shift = clz(remainder);
		if (shift > bit_pos) shift = bit_pos;
		remainder <<= shift;
		bit_pos -= shift;
		
		uint32_t div = remainder / divider;
		remainder = remainder % divider;
		quotient += div << bit_pos;

		#ifndef FIXMATH_NO_OVERFLOW
		if (div & ~(0xFFFFFFFF >> bit_pos))
				return fix16_overflow;
		#endif
		
		remainder <<= 1;
		bit_pos--;
	}
	
	#ifndef FIXMATH_NO_ROUNDING
	// Quotient is always positive so rounding is easy
	quotient++;
	#endif
	
	fix16_t result = quotient >> 1;
	
	// Figure out the sign of the result
	if ((a ^ b) & 0x80000000)
	{
		#ifndef FIXMATH_NO_OVERFLOW
		if (result == fix16_minimum)
				return fix16_overflow;
		#endif
		
		result = -result;
	}
	
	return result;
}
#endif


/* The square root algorithm is quite directly from
 * http://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Binary_numeral_system_.28base_2.29
 * An important difference is that it is split to two parts
 * in order to use only 32-bit operations.
 *
 * Note that for negative numbers we return -sqrt(-inValue).
 * Not sure if someone relies on this behaviour, but not going
 * to break it for now. It doesn't slow the code much overall.
 */
fix16_t fix16_sqrt(fix16_t inValue)
{
	uint8_t  neg = (inValue < 0);
	uint32_t num = (neg ? -inValue : inValue);
	uint32_t result = 0;
	uint32_t bit;
	uint8_t  n;
	
	// Many numbers will be less than 15, so
	// this gives a good balance between time spent
	// in if vs. time spent in the while loop
	// when searching for the starting value.
	if (num & 0xFFF00000)
		bit = (uint32_t)1 << 30;
	else
		bit = (uint32_t)1 << 18;
	
	while (bit > num) bit >>= 2;
	
	// The main part is executed twice, in order to avoid
	// using 64 bit values in computations.
	for (n = 0; n < 2; n++)
	{
		// First we get the top 24 bits of the answer.
		while (bit)
		{
			if (num >= result + bit)
			{
				num -= result + bit;
				result = (result >> 1) + bit;
			}
			else
			{
				result = (result >> 1);
			}
			bit >>= 2;
		}
		
		if (n == 0)
		{
			// Then process it again to get the lowest 8 bits.
			if (num > 65535)
			{
				// The remainder 'num' is too large to be shifted left
				// by 16, so we have to add 1 to result manually and
				// adjust 'num' accordingly.
				// num = a - (result + 0.5)^2
				//	 = num + result^2 - (result + 0.5)^2
				//	 = num - result - 0.5
				num -= result;
				num = (num << 16) - 0x8000;
				result = (result << 16) + 0x8000;
			}
			else
			{
				num <<= 16;
				result <<= 16;
			}
			
			bit = 1 << 14;
		}
	}


	
#ifndef FIXMATH_NO_ROUNDING
	// Finally, if next bit would have been 1, round the result upwards.
	if (num > result)
	{
		result++;
	}
#endif
	
	return (neg ? -(fix16_t)result : (fix16_t)result);
}



#ifndef FIXMATH_NO_CACHE
static fix16_t _fix16_exp_cache_index[4096]  = { 0 };
static fix16_t _fix16_exp_cache_value[4096]  = { 0 };
#endif



fix16_t fix16_exp(fix16_t inValue) {
  //printf("hello: %d\n", inValue);
  if(inValue == 0        ){
    //printf("shit1\n");
    return fix16_one;
  }
  if(inValue == fix16_one){
    //printf("shit2\n");
    return fix16_e;
  }
  if(inValue >= 681391   ){
    //printf("shit3\n");
    return fix16_maximum;
  }
  
  if(inValue <= -772243  ){
    //printf("shit4\n");
    return 0;
  }
  

  //printf("there!\n");

	#ifndef FIXMATH_NO_CACHE
	fix16_t tempIndex = (inValue ^ (inValue >> 16));
	tempIndex = (inValue ^ (inValue >> 4)) & 0x0FFF;
	if(_fix16_exp_cache_index[tempIndex] == inValue)
		return _fix16_exp_cache_value[tempIndex];
	#endif
                        
	/* The algorithm is based on the power series for exp(x):
	 * http://en.wikipedia.org/wiki/Exponential_function#Formal_definition
	 * 
	 * From term n, we get term n+1 by multiplying with x/n.
	 * When the sum term drops to zero, we can stop summing.
	 */
            
	// The power-series converges much faster on positive values
	// and exp(-x) = 1/exp(x).
	bool neg = (inValue < 0);
	if (neg) inValue = -inValue;
            
	fix16_t result = inValue + fix16_one;
	fix16_t term = inValue;

	//printf("term: %d\n",term);
	//printf("result: %d\n",result);

	uint_fast8_t i;        
	for (i = 2; i < 30; i++)
	{
		term = fix16_mul(term, fix16_div(inValue, fix16_from_int(i)));
		result += term;
		
		//printf("term: %d\n",term);
		//printf("result: %d\n",result);
		
		if ((term < 500) && ((i > 15) || (term < 20)))
			break;
	}
            
	if (neg) result = fix16_div(fix16_one, result);
            
	#ifndef FIXMATH_NO_CACHE
	_fix16_exp_cache_index[tempIndex] = inValue;
	_fix16_exp_cache_value[tempIndex] = result;
	#endif

	return result;
}



fix16_t fix16_log(fix16_t inValue)
{
  //printf("arg: %d\n", inValue);
	fix16_t guess = fix16_from_int(2);
	fix16_t delta;
	int scaling = 0;
	int count = 0;
	
	if (inValue <= 0)
		return fix16_minimum;
	
	// Bring the value to the most accurate range (1 < x < 100)
	const fix16_t e_to_fourth = 3578144;
	while (inValue > fix16_from_int(100))
	{
		inValue = fix16_div(inValue, e_to_fourth);
		scaling += 4;
	}
	
	while (inValue < fix16_one)
	{
		inValue = fix16_mul(inValue, e_to_fourth);
		scaling -= 4;
	}
	
	do
	{
		// Solving e(x) = y using Newton's method
		// f(x) = e(x) - y
		// f'(x) = e(x)
		fix16_t e = fix16_exp(guess);
		delta = fix16_div(inValue - e, e);
		
		// It's unlikely that logarithm is very large, so avoid overshooting.
		if (delta > fix16_from_int(3))
			delta = fix16_from_int(3);
		
		guess += delta;
		
	} while ((count++ < 10)
		&& ((delta > 1) || (delta < -1)));

	//printf("guess: %d\n", guess);
	
	return guess + fix16_from_int(scaling);
}
