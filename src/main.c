#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#include "swucb-float.h"
#include "swucb-fixed.h"


#define ITERATIONS 1000 // Amount of time steps in total

#define TIME_INTERVAL 500 // PDR values remain static for 500 time steps


// Stores the PDR values for every time interval
typedef struct {

  double arms[NUM_ARMS];

} arm_values_t;


// Draws a sample
// @param probability the probability of a success
// @return 1 if success, else 0
static uint32_t draw_sample(double probability){

  double rndDouble = (double)rand() / RAND_MAX;
  if(rndDouble <= probability){
    return 1;

  }
  else{
    return 0;

  }

}

// Runs the floating point SWUCB algorithm
// @param: window_size the static sliding window size
// @param: time_interval the static duration in which PDR values remain static
// @param: arm_values an array of PDR values for each channel at different time intervals
void swucb_float(int window_size, int time_interval, arm_values_t *arm_values, int *arms_index){

  
  swucb_float_args_t *args = swucb_float_init(window_size); //Initializes the parameters used by the algorithm
  
  int successes = 0;  // Number of successful draws from samples
  int arm = 0;        // Current selected arm
  int sample = 0;     // Sample which is either successful (1) of failed (0)

  //int arms_index = 0; // Current index value for arm_values which is used to iterate to a new set of PDRs in the array when a breakpoint has been reached

  double pdr = -1;

  for(int t = 1; t <= ITERATIONS; ++t){

    arm = swucb_float_get_arm(args); // Get an arm from the algorithm
    
    sample = draw_sample(arm_values[*arms_index].arms[arm]); // Draw a sample based on the current PDR of the selected arm

    swucb_float_append_result(args, sample, arm); // Append the result to the sliding window in args

    successes += sample;

    pdr = (double)successes / t;

    printf("float %d %d %d %lf\n",window_size,time_interval,t,pdr);
    //printf("float %d %d %d %d\n",window_size,time_interval,t,successes);

    if(t % time_interval == 0 && t < 1000){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

      *arms_index = *arms_index + 1; 

    }

  }
  swucb_float_destroy(args); // Deallocates the parameters for SWUCB
  
  
  pdr = (double)successes / (double)ITERATIONS;

  // Prints the result of the algorithm
  //printf("float %d %d %lf\n",window_size,time_interval,pdr);

  


}

// Runs the fixed point SWUCB algorithm
// @param: window_size the static sliding window size
// @param: time_interval the static duration in which PDR values remain static
// @param: arm_values an array of PDR values for each channel at different time intervals
void swucb_fixed(uint32_t window_size, uint32_t time_interval, arm_values_t *arm_values){

  swucb_args_t *args = swucb_init(window_size); //Initializes the parameters used by the algorithm
  uint32_t successes = 0; // Number of successful draws from samples
  uint32_t arm = 0;       // Current selected arm
  uint32_t sample = 0;    // Sample which is either successful (1) of failed (0)

  uint32_t arms_index = 0; // Current index value for arm_values which is used to iterate to a new set of PDRs in the array when a breakpoint has been reached

  for(int t = 1; t <= ITERATIONS; ++t){

    arm = swucb_get_arm(args); // Get an arm from the algorithm
    
    sample = draw_sample(arm_values[arms_index].arms[arm]); // Draw a sample based on the current PDR of the selected arm

    swucb_append_result(args, sample, arm); // Append the result to the sliding window in args

    successes += sample;

    if(t % time_interval == 0){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

      arms_index++;

    }

  }
  swucb_destroy(args);
  
  double pdr = (double)successes / (double)ITERATIONS;
  //Prints the result of the algorithm
  printf("fixed %d %d %lf\n",window_size,time_interval,pdr);
}

int main(int argc, char *argv[]){

  srandom(time(NULL));

  
  FILE *file = fopen("src/simulation.txt", "r");
  //FILE *file = fopen("src/dataset.txt", "r");
  //FILE *file = fopen("src/pdrtest.txt", "r");
  //FILE *file = fopen("src/pdrtest2.txt", "r");

  arm_values_t *arm_values = calloc(ITERATIONS, sizeof(arm_values_t));

  arm_values_t *iter = arm_values;

  int arms_index = 0;

  //Initializes the PDR values from the file into the arm_values array
  for(int i = 0; i < ITERATIONS; ++i){
    for(uint32_t arm = 0 ; arm < NUM_ARMS; ++arm){

      fscanf(file,"%lf\n",&(iter[i].arms[arm]));

    }
  }
  fclose(file);
  for(int window_size = 100; window_size <= 1000; window_size+=100){
    for(int i = 0; i < 40; ++i){
      swucb_float(window_size, TIME_INTERVAL, arm_values, &arms_index);
      //arms_index = 0; //For pdrtest2.txt
      
      //swucb_float(1000, TIME_INTERVAL, arm_values, &arms_index);
      //swucb_fixed(window_size, TIME_INTERVAL, arm_values);
    }
  }
  //swucb_float(10000, TIME_INTERVAL, arm_values, &arms_index);
 
  
  free(arm_values);
  return 0;

}
