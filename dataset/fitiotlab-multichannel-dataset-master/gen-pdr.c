#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "gen-pdr.h"

typedef struct {

  int successes;
  int size;

} pdr_tuple_t; 


double gen_pdr(){

  pdr_tuple_t dataset_tuple[16] = {.successes = 0, .size = 0};
  int channel;
  int success;

  FILE *file = fopen("dataset.txt", "r");

  while(fscanf(file, "%d %d", &channel, &success) == 2){

    dataset_tuple[channel-11].success += success;
    dataset_tuple[channel-11].size++;
    

  }

  double pdrs[16];

  for(int i = 0; i < 16; ++i){

    pdrs[i] = (double) dataset_tuple[i].success / (double) dataset_tuple[i].size;

  }

  return pdrs;

}
