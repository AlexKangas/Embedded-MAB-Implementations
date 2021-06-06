/* File integer.cpp
 *
 * Programmed by Jack W. Crenshaw, Crenshaw Technologies
 *
 * This file contains integer versions of many useful functions,
 * including square root and log/exp functions.  Copy them into
 * your programs where they might be useful.  Some functions may
 * be multiply defined, so do not expect the file to compile as is.
 */

#include "sqrtfix.h"
#include <stdint.h>



#include <stdio.h>

//#define PERCISION 27

#define FIX_ADD(a,b) (a + b)
#define FIX_SUB(a,b) (a - b)
#define FIX_MUL(a,b,percision) ((uint32_t)(((uint64_t)a * (uint64_t)b) >> percision))
#define FIX_DIV(a,b,percision) ((uint32_t)(((uint64_t)a << percision)/b))

// The simplest integer square root
uint32_t isqrt_simple(uint32_t a, uint32_t percision){
        uint32_t x = 1 << percision;
	while(FIX_MUL(x,x,percision) <= a){
	  x += 1 << percision;
	  //printf("%d\n", FIX_MUL(x,x) >> 16);
	}
	return x - (1 << percision);
}

/* An improved version.  Using no multiplications
 * it's ideal for small microcontrollers. Feel
 * free to change the word length as desired.
 */

uint32_t isqrt_improved(uint32_t a, uint32_t percision){
        uint32_t x = 1 << percision;
        uint32_t xsqr = 1 << percision;
        uint32_t delta = 3 << percision;
	while(xsqr <=a){
		x += 1 << percision;
		xsqr += delta;
		delta += 2 << percision;
	}
	return x - (1 << percision);
}


// The simplest integer square root
/*
uint32_t isqrt_simple(uint32_t a){
        uint32_t x = 1;
	while(x*x <= a)
		++x;
return --x;
}
*/

/* An improved version.  Using no multiplications
 * it's ideal for small microcontrollers. Feel
 * free to change the word length as desired.
 */
/*
uint32_t isqrt_improved(uint32_t a){
        uint32_t x = 1;
        uint32_t xsqr = 1;
        uint32_t delta = 3;
	while(xsqr <=a){
		++x;
		xsqr += delta;
		delta += 2;
	}
	return --x;
}
*/


/* A non-iterative square root
 * This function is the binary version of your
 * familiar high-school algorithm.  It works in
 * fashion similar to the shift-and-subtract
 * integer division.
 */

unsigned short isqrt_non_iter(unsigned long a){
	unsigned long rem = 0;
	unsigned long root = 0;
	for(int i=0; i<16; i++){
		root <<= 1;
		rem = ((rem << 2) + (a >> 30));
		a <<= 2;
		root ++;
		if(root <= rem){
			rem -= root;
			root++;
		}
		else
			root--;
	}
	return (unsigned short)(root >> 1);
}

/* An integer version of the iterative method
 * with optimal initial guess.  See pp. 85-87
 * of text.
 */
/*
unsigned short isqrt_iter(unsigned long a){
	unsigned long temp;
	long e;
	unsigned long x = 0;
	if((a & 0xffff0000) != 0)
		x = 444 + a / 26743;
	else if((a & 0xff00) != 0)
		x = 21 + a / 200;
	else
		x = 1 + a / 12;
	do{
		temp = a / x;
		e = (x - temp) / 2;
		x = (x + temp) / 2;
	}
	while(e != 0);
	return (unsigned short)x;
}
*/

uint32_t isqrt_iter(uint32_t a){
        uint32_t temp;
        int32_t e;
        uint32_t x = 0;
	if((a & (0xffff0000 << 16)) != 0)
	  x = (444 << 16) + FIX_DIV(a, 26743 << 16, 16);
	else if((a & (0xff00 << 16)) != 0)
	  x = (21 << 16) + FIX_DIV(a, 200 << 16, 16);
	else
	  x = (1 << 16) + FIX_DIV(a, 12 << 16, 16);
	do{
	  temp = FIX_DIV(a, x, 16);
		e = (x - temp) >> 1;
		x = (x + temp) >> 1;
	}
	while(e != 0);
	return (uint32_t)x;
}
