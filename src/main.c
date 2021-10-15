#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ducb-float.h"
#include "ducb-fixed.h"
#include "swucb-float.h"
#include "swucb-fixed.h"

#include "fixed-point.h"

#define ITERATIONS 1000 // Amount of time steps in total

#define STATIONARY_ENV 1000
#define ONE_BREAKPOINT_ENV 500
#define FOUR_BREAKPOINT_ENV 200

//Converts a doubles to its fixed-point representation
fix16_t fix16_from_dbl(double a)
{
	double temp = a * fix16_one;
#ifndef FIXMATH_NO_ROUNDING
	temp += (temp >= 0) ? 0.5f : -0.5f;
#endif
	return (fix16_t)temp;
}

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

//Generates random PDRs for channels
static void gen_pdr(double *arms){

  double epsilon = 0.000001;

  double avg = 0;
  double diff = 0;
  
  do{
    avg = 0;
    int random_index = rand() % NUM_ARMS;
    for(int i = 0; i < NUM_ARMS; ++i){
      if(i == random_index){
	arms[i] = 1.0;
      }
      else{
	arms[i] = (double)rand() / RAND_MAX;
      }
      avg += arms[i];
    }
    avg = avg / NUM_ARMS;
    diff = avg - 0.5;
    if(diff < 0){diff = -diff;}
  }while(diff >= epsilon);

  

}

// Runs the floating point SWUCB algorithm
// @param: window_size the static sliding window size
// @param: time_interval the static duration in which PDR values remain static
void swucb_float(int window_size, int time_interval, double confidence_level,double bound){

  
  swucb_float_args_t *args = swucb_float_init(window_size, confidence_level, bound); //Initializes the parameters used by the algorithm
  
  int successes = 0;  // Number of successful draws from samples
  int arm = 0;        // Current selected arm
  int sample = 0;     // Sample which is either successful (1) of failed (0)

  double pdr = -1;

  double arms[NUM_ARMS];

  gen_pdr(arms);

  for(int t = 1; t <= ITERATIONS; ++t){

    arm = swucb_float_get_arm(args); // Get an arm from the algorithm
    
    sample = draw_sample(arms[arm]); // Draw a sample based on the current PDR of the selected arm

    swucb_float_append_result(args, sample, arm); // Append the result to the sliding window in args

    successes += sample;
    if(t % time_interval == 0){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

      gen_pdr(arms);
    }

  }
  
  
  swucb_float_destroy(args); // Deallocates the parameters for SWUCB
  
  
  pdr = (double)successes / (double)ITERATIONS;

  // Prints the result of the algorithm
  printf("float %d %d %lf %lf\n",window_size,time_interval,confidence_level,pdr);


}

// Runs the fixed point SWUCB algorithm
// @param: window_size the static sliding window size
// @param: time_interval the static duration in which PDR values remain static
void swucb_fixed(uint32_t window_size, uint32_t time_interval, double confidence_level,double bound){
  //window_t window;
  swucb_args_t args;
  fix_link_t links[1000];
  
  //swucb_init(&args, &window, window_size, fix16_from_dbl(confidence_level), fix16_from_dbl(bound)); //Initializes the parameters used by the algorithm
  swucb_init(&args, window_size, fix16_from_dbl(confidence_level), fix16_from_dbl(bound)); //Initializes the parameters used by the algorithm
  uint32_t successes = 0; // Number of successful draws from samples
  uint32_t arm = 0;       // Current selected arm
  uint32_t sample = 0;    // Sample which is either successful (1) of failed (0)

  double pdr = 0;

  double arms[NUM_ARMS];

  gen_pdr(arms);
  
  for(int t = 1; t <= ITERATIONS; ++t){

    arm = swucb_get_arm(&args); // Get an arm from the algorithm
    
    sample = draw_sample(arms[arm]); // Draw a sample based on the current PDR of the selected arm

    swucb_append_result(&args, sample, arm, &links[t-1]); // Append the result to the sliding window in args

    successes += sample;

    if(t % time_interval == 0){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

      gen_pdr(arms);
    }

  }

  
  pdr = (double)successes / (double)ITERATIONS;
  //Prints the result of the algorithm
  printf("fixed %d %d %lf %lf\n",window_size,time_interval,confidence_level,pdr);
}


// Runs the floating point SWUCB algorithm
// @param: window_size the static sliding window size
// @param: time_interval the static duration in which PDR values remain static
void ducb_float(double discount, int time_interval, double confidence_level,double bound){

  
  ducb_float_args_t *args = ducb_float_init(discount,confidence_level,bound); //Initializes the parameters used by the algorithm
  
  int successes = 0;  // Number of successful draws from samples
  int arm = 0;        // Current selected arm
  int sample = 0;     // Sample which is either successful (1) of failed (0)

  //int arms_index = 0; // Current index value for arm_values which is used to iterate to a new set of PDRs in the array when a breakpoint has been reached

  double pdr = -1;

  double arms[NUM_ARMS];

  gen_pdr(arms);

  for(int t = 1; t <= ITERATIONS; ++t){

    arm = ducb_float_get_arm(args); // Get an arm from the algorithm
    
    sample = draw_sample(arms[arm]); // Draw a sample based on the current PDR of the selected arm

    ducb_float_append_result(args, sample, arm); // Append the result to the sliding window in args

    successes += sample;
    
    if(t % time_interval == 0){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

      gen_pdr(arms);

    }

  }
  ducb_float_destroy(args); // Deallocates the parameters for SWUCB
  
  
  pdr = (double)successes / (double)ITERATIONS;

  // Prints the result of the algorithm
  printf("float %lf %d %lf %lf\n",discount,time_interval,confidence_level,pdr);

}

// Runs the floating point SWUCB algorithm
// @param: window_size the static sliding window size
// @param: time_interval the static duration in which PDR values remain static
void ducb_fixed(double discount, uint32_t time_interval, double confidence_level,double bound){

  ducb_fixed_args_t args;
  ducb_fixed_init(&args,fix16_from_dbl(discount),fix16_from_dbl(confidence_level),fix16_from_dbl(bound)); //Initializes the parameters used by the algorithm
  
  uint32_t successes = 0;  // Number of successful draws from samples
  uint32_t arm = 0;        // Current selected arm
  uint32_t sample = 0;     // Sample which is either successful (1) of failed (0)

  double pdr = -1;

  double arms[NUM_ARMS];

  gen_pdr(arms);

  for(uint32_t t = 1; t <= ITERATIONS; ++t){

    arm = ducb_fixed_get_arm(&args); // Get an arm from the algorithm
    
    sample = draw_sample(arms[arm]); // Draw a sample based on the current PDR of the selected arm

    ducb_fixed_append_result(&args, sample, arm); // Append the result to the sliding window in args

    successes += sample;

    pdr = (double)successes / t;

    
    if(t % time_interval == 0){ // If a breakpoint has been reached, then select the next set of PDR values for the arms


      gen_pdr(arms);

    }

  }
  
  
  pdr = (double)successes / (double)ITERATIONS;

  // Prints the result of the algorithm
  printf("fixed %lf %d %lf %lf\n",discount,time_interval,confidence_level,pdr);

}

void run_swucb_simulations(){
  for(int window_size = 100; window_size <= 1000; window_size+=100){
    for(int i = 0; i < 100; ++i){
      
      swucb_float(window_size, STATIONARY_ENV, 1, 1);
      swucb_float(window_size, STATIONARY_ENV, 0.1, 1);
      swucb_float(window_size, STATIONARY_ENV, 0.01, 1);
      swucb_float(window_size, STATIONARY_ENV, 0.001, 1);
      swucb_float(window_size, STATIONARY_ENV, 0.0001, 1);
      
      swucb_fixed(window_size, STATIONARY_ENV, 1, 1);
      swucb_fixed(window_size, STATIONARY_ENV, 0.1, 1);
      swucb_fixed(window_size, STATIONARY_ENV, 0.01, 1);
      swucb_fixed(window_size, STATIONARY_ENV, 0.001, 1);
      swucb_fixed(window_size, STATIONARY_ENV, 0.0001, 1);

      swucb_float(window_size, ONE_BREAKPOINT_ENV, 1, 1);
      swucb_float(window_size, ONE_BREAKPOINT_ENV, 0.1, 1);
      swucb_float(window_size, ONE_BREAKPOINT_ENV, 0.01, 1);
      swucb_float(window_size, ONE_BREAKPOINT_ENV, 0.001, 1);
      swucb_float(window_size, ONE_BREAKPOINT_ENV, 0.0001, 1);

      swucb_fixed(window_size, ONE_BREAKPOINT_ENV, 1, 1);
      swucb_fixed(window_size, ONE_BREAKPOINT_ENV, 0.1, 1);
      swucb_fixed(window_size, ONE_BREAKPOINT_ENV, 0.01, 1);
      swucb_fixed(window_size, ONE_BREAKPOINT_ENV, 0.001, 1);
      swucb_fixed(window_size, ONE_BREAKPOINT_ENV, 0.0001, 1);

      swucb_float(window_size, FOUR_BREAKPOINT_ENV, 1, 1);
      swucb_float(window_size, FOUR_BREAKPOINT_ENV, 0.1, 1);
      swucb_float(window_size, FOUR_BREAKPOINT_ENV, 0.01, 1);
      swucb_float(window_size, FOUR_BREAKPOINT_ENV, 0.001, 1);
      swucb_float(window_size, FOUR_BREAKPOINT_ENV, 0.0001, 1);

      swucb_fixed(window_size, FOUR_BREAKPOINT_ENV, 1, 1);
      swucb_fixed(window_size, FOUR_BREAKPOINT_ENV, 0.1, 1);
      swucb_fixed(window_size, FOUR_BREAKPOINT_ENV, 0.01, 1);
      swucb_fixed(window_size, FOUR_BREAKPOINT_ENV, 0.001, 1);
      swucb_fixed(window_size, FOUR_BREAKPOINT_ENV, 0.0001, 1);
    }
  }
}

void run_ducb_simulations(){

  for(double discount = 0.950; discount <= 1.0; discount += 0.005){
    for(int i = 0; i < 100; ++i){
      ducb_float(discount, STATIONARY_ENV, 1, 1);
      ducb_float(discount, STATIONARY_ENV, 0.1, 1);
      ducb_float(discount, STATIONARY_ENV, 0.01, 1);
      ducb_float(discount, STATIONARY_ENV, 0.001, 1);
      ducb_float(discount, STATIONARY_ENV, 0.0001, 1);

      ducb_fixed(discount, STATIONARY_ENV, 1, 1);
      ducb_fixed(discount, STATIONARY_ENV, 0.1, 1);
      ducb_fixed(discount, STATIONARY_ENV, 0.01, 1);
      ducb_fixed(discount, STATIONARY_ENV, 0.001, 1);
      ducb_fixed(discount, STATIONARY_ENV, 0.0001, 1);


      ducb_float(discount, ONE_BREAKPOINT_ENV, 1, 1);
      ducb_float(discount, ONE_BREAKPOINT_ENV, 0.1, 1);
      ducb_float(discount, ONE_BREAKPOINT_ENV, 0.01, 1);
      ducb_float(discount, ONE_BREAKPOINT_ENV, 0.001, 1);
      ducb_float(discount, ONE_BREAKPOINT_ENV, 0.0001, 1);

      ducb_fixed(discount, ONE_BREAKPOINT_ENV, 1, 1);
      ducb_fixed(discount, ONE_BREAKPOINT_ENV, 0.1, 1);
      ducb_fixed(discount, ONE_BREAKPOINT_ENV, 0.01, 1);
      ducb_fixed(discount, ONE_BREAKPOINT_ENV, 0.001, 1);
      ducb_fixed(discount, ONE_BREAKPOINT_ENV, 0.0001, 1);


      ducb_float(discount, FOUR_BREAKPOINT_ENV, 1, 1);
      ducb_float(discount, FOUR_BREAKPOINT_ENV, 0.1, 1);
      ducb_float(discount, FOUR_BREAKPOINT_ENV, 0.01, 1);
      ducb_float(discount, FOUR_BREAKPOINT_ENV, 0.001, 1);
      ducb_float(discount, FOUR_BREAKPOINT_ENV, 0.0001, 1);

      ducb_fixed(discount, FOUR_BREAKPOINT_ENV, 1, 1);
      ducb_fixed(discount, FOUR_BREAKPOINT_ENV, 0.1, 1);
      ducb_fixed(discount, FOUR_BREAKPOINT_ENV, 0.01, 1);
      ducb_fixed(discount, FOUR_BREAKPOINT_ENV, 0.001, 1);
      ducb_fixed(discount, FOUR_BREAKPOINT_ENV, 0.0001, 1);
    }
  }

}

int main(int argc, char *argv[]){

  srandom(time(NULL));
  
  //run_swucb_simulations();
  run_ducb_simulations();
  
  return 0;

}
