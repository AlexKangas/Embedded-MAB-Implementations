#include "fixed-point.h"
#include <stdbool.h>

fix16_t fix16_mul(fix16_t a, fix16_t b)
{
  
  int64_t product = (int64_t)a * b;
  return product >> 16;
  
}

fix16_t fix16_div(fix16_t a, fix16_t b)
{
  
  int64_t dividend = (int64_t) a << 16;
  return (fix16_t) (dividend/b);
	
}


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
  if(inValue == 0        ){
    return fix16_one;
  }
  if(inValue == fix16_one){
    return fix16_e;
  }
  if(inValue >= 681391   ){
    return fix16_maximum;
  }
  
  if(inValue <= -772243  ){
    return 0;
  }
  

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

	uint_fast8_t i;        
	for (i = 2; i < 30; i++)
	{
		term = fix16_mul(term, fix16_div(inValue, fix16_from_int(i)));
		result += term;
		
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
	fix16_t guess = fix16_from_int(2);
	fix16_t delta;
	int scaling = 0;
	int count = 0;
	
	if (inValue <= 0)
	  {
		return fix16_minimum;
	  }
	
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
	
	return guess + fix16_from_int(scaling);
}
