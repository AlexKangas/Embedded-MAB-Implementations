#include <stdio.h>
#include <stdlib.h>
#include "swucb.h"
#include <stdint.h>
#include "swucbfix.h"

#define NUM_ARMS 16
#define PERCISION 16
#define SHIFT_MASK ((1 << PERCISION) - 1)

int main(int argc, char *argv[]){

  double arms[NUM_ARMS] = {0.8800,
			  0.7824,
			  0.4793,
			  0.6592,
			  0.1754,
			  0.6005,
			  0.5338,
			  0.4202,
			  0.9367,
			  0.6502,
			  0.5077,
			  0.0515,
			  0.4817,
			  0.9411,
			  0.2394,
			  0.8561};

  double pdr = swucb(arms,NUM_ARMS);

  printf("Float PDR = %lf \n", pdr);

  uint32_t armsfix[NUM_ARMS];


  for(int i = 0; i < NUM_ARMS; ++i){

    armsfix[i] = (uint32_t)(arms[i] * (1 << PERCISION));

  }

  uint32_t pdrfix = swucbfix(arms, (uint32_t)NUM_ARMS);

  printf("Fixed PDR = %lf \n", ((double)pdrfix / (double)(1 << PERCISION)));

  return 0;
  
}
