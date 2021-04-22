#include <stdio.h>
#include <stdlib.h>
#include "swucb.h"

#define NUM_ARMS 16

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

  printf("PDR = %lf \n", pdr);

  return 0;
  
}
