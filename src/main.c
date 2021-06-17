#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#include "swucb-float.h"
#include "swucb-fixed.h"


#define ITERATIONS 1000

#define TIME_INTERVAL 1001

#define WINDOW_SIZE 1000

typedef struct {

  double arms[NUM_ARMS];

} arm_values_t;


static uint32_t draw_sample(double probability){

  double rndDouble = (double)rand() / RAND_MAX;
  if(rndDouble < probability){
    return 1;

  }
  else{
    return 0;

  }

}


void simulation_float(int window_size, int time_interval, arm_values_t *arm_values){

  
  //int successes = swucb_float(window_size, time_interval, ITERATIONS, arm_values);

  swucb_float_args_t *args = swucb_float_init(window_size);
  int successes = 0;
  int arm = 0;
  int sample = 0;

  int arms_index = 0;

  for(int t = 1; t <= ITERATIONS; ++t){

    if(t % time_interval == 0){

      arms_index++;

    }

    arm = swucb_float_get_arm(args);
    
    sample = draw_sample(arm_values[arms_index].arms[arm]);

    swucb_float_append_result(args, sample, arm);

    successes += sample;

  }
  swucb_float_destroy(args);
  
  
  double pdr = (double)successes / (double)ITERATIONS;
  printf("1,%d,%d,%lf|",window_size,time_interval,pdr);

  


}

//void simulation_fixed(uint32_t window_size, uint32_t time_interval, arm_values_t *arm_values, fix16_t *logs){
void simulation_fixed(uint32_t window_size, uint32_t time_interval, arm_values_t *arm_values){

  swucb_args_t *args = swucb_init(window_size);
  uint32_t successes = 0;
  uint32_t arm = 0;
  uint32_t sample = 0;

  uint32_t arms_index = 0;

  for(int t = 1; t <= ITERATIONS; ++t){

    if(t % time_interval == 0){

      arms_index++;

    }

    arm = swucb_get_arm(args);
    
    sample = draw_sample(arm_values[arms_index].arms[arm]);

    swucb_append_result(args, sample, arm);

    successes += sample;

  }
  swucb_destroy(args);
  
  double pdr = (double)successes / (double)ITERATIONS;
  printf("2,%d,%d,%lf|",window_size,time_interval,pdr);
}

int main(int argc, char *argv[]){

  srandom(time(NULL));

  FILE *file = fopen("pdrs.txt", "r");

  arm_values_t *arm_values = calloc(ITERATIONS, sizeof(arm_values_t));

  arm_values_t *iter = arm_values;

  int res;

  for(int i = 0; i < ITERATIONS; ++i){
    for(uint32_t arm = 0 ; arm < NUM_ARMS; ++arm){

      fscanf(file,"%lf\n",&(iter[i].arms[arm]));

    }
  }
  fclose(file);

  //fix16_t *logs = get_logs();

  
  for(int time_interval = 1; time_interval <= TIME_INTERVAL; ++time_interval){
    for(int window_size = 50; window_size <= 50; ++window_size){
      simulation_float(window_size, time_interval, arm_values);
      //simulation_fixed(window_size, time_interval, arm_values, logs);
      simulation_fixed(window_size, time_interval, arm_values);
    }
  }
  
  //free(logs);
  free(arm_values);
  return 0;

}
