#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#define NUM_ARMS 16

typedef double arm_t;

typedef struct {

  double arms[NUM_ARMS];

} arm_values_t;


int main(int argc, char *argv[]){

  srandom(time(NULL));

  FILE *file = fopen("pdrs.txt", "r");

  arm_values_t *arm_values = calloc(1001, sizeof(arm_values_t));

  arm_values_t *iter = arm_values;

  for(int i = 0; i < 1001; ++i){
    for(uint32_t arm = 0 ; arm < NUM_ARMS; ++arm){

      if(fscanf(file,"%lf\n",&(iter[i].arms[arm])) == 1){

      }

    }
  }
  fclose(file);

  double avg = 0;
  double epsilon = 0.00001;
  double diff = 0;

  for(int b = 0; b < 10000; b++){
    for(int i = 0; i < 1001; ++i){
      avg = 0;
      for(uint32_t arm = 0 ; arm < NUM_ARMS; ++arm){
      
	avg += iter[i].arms[arm];

      }
      avg = avg / NUM_ARMS;
      diff = avg - 0.5;
      if(diff < 0){diff = -diff;}
      if(diff >= epsilon){
	printf("FAIL: %lf\n", diff);
      }
    }
  }
  
  return 0;

}
