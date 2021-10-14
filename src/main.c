#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#include "ducb-float.h"
#include "ducb-fixed.h"
#include "swucb-float.h"
#include "swucb-fixed.h"

#include "fixed-point.h"

#define ITERATIONS 1000 // Amount of time steps in total

#define TIME_INTERVAL 1000 // PDR values remain static for 500 time steps

#define BOUND (double)1
#define CONFIDENCE_LEVEL (double)0.6

// Stores the PDR values for every time interval
typedef struct {

  double arms[NUM_ARMS];

} arm_values_t;

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
// @param: arm_values an array of PDR values for each channel at different time intervals
//void swucb_float(int window_size, int time_interval, arm_values_t *arm_values, int *arms_index, double confidence_level,double bound){
void swucb_float(int window_size, int time_interval, double confidence_level,double bound){

  
  swucb_float_args_t *args = swucb_float_init(window_size, confidence_level, bound); //Initializes the parameters used by the algorithm
  
  int successes = 0;  // Number of successful draws from samples
  int arm = 0;        // Current selected arm
  int sample = 0;     // Sample which is either successful (1) of failed (0)

  //int arms_index = 0; // Current index value for arm_values which is used to iterate to a new set of PDRs in the array when a breakpoint has been reached

  double pdr = -1;

  double regret = 0;

  double pdr_optimal = 1.0;

  double arms[NUM_ARMS];

  gen_pdr(arms);

  for(int t = 1; t <= ITERATIONS; ++t){

    arm = swucb_float_get_arm(args); // Get an arm from the algorithm
    
    sample = draw_sample(arms[arm]); // Draw a sample based on the current PDR of the selected arm

    swucb_float_append_result(args, sample, arm); // Append the result to the sliding window in args

    successes += sample;

    //pdr = (double)successes / t;

    //printf("float %d %d %d %lf\n",window_size,time_interval,t,pdr);
    //printf("float %d %d %d %d\n",window_size,time_interval,t,successes);

    //regret += pdr_optimal - arm_values[*arms_index].arms[arm];

    //printf("swucb %d %d %d %lf\n",window_size,successes,t,regret);

    //if(t % time_interval == 0 && t < 2*TIME_INTERVAL){ // If a breakpoint has been reached, then select the next set of PDR values for the arms
    if(t % time_interval == 0){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

      //*arms_index = *arms_index + 1; 
      gen_pdr(arms);
    }

  }
  /*
  window_t window = *(args->window);
  printf("Size_current = %d\n",window.size_current);
  printf("Size_max = %d\n",window.size_max);

  for(int a = 0; a < 16; a++){
    printf("Selections arm %d = %d\n",a, window.selections[a]);
    printf("Sums arm %d = %d\n",a, window.sums[a]);
  }
  */
  
  
  swucb_float_destroy(args); // Deallocates the parameters for SWUCB
  
  
  pdr = (double)successes / (double)ITERATIONS;

  // Prints the result of the algorithm
  printf("float %d %d %lf %lf\n",window_size,time_interval,confidence_level,pdr);

  //*arms_index = *arms_index + 1; 


}

// Runs the fixed point SWUCB algorithm
// @param: window_size the static sliding window size
// @param: time_interval the static duration in which PDR values remain static
// @param: arm_values an array of PDR values for each channel at different time intervals
//void swucb_fixed(uint32_t window_size, uint32_t time_interval, arm_values_t *arm_values, uint32_t *arms_index, double confidence_level,double bound){
void swucb_fixed(uint32_t window_size, uint32_t time_interval, double confidence_level,double bound){
  window_t window;
  swucb_args_t args;
  fix_link_t links[10000];
  //int large[50000];
  swucb_init(&args, &window, window_size, fix16_from_dbl(confidence_level), fix16_from_dbl(bound)); //Initializes the parameters used by the algorithm
  //swucb_init(&args, window_size, fix16_from_dbl(confidence_level), fix16_from_dbl(bound)); //Initializes the parameters used by the algorithm
  uint32_t successes = 0; // Number of successful draws from samples
  uint32_t arm = 0;       // Current selected arm
  uint32_t sample = 0;    // Sample which is either successful (1) of failed (0)

  double pdr = 0;

  double arms[NUM_ARMS];

  gen_pdr(arms);
  //printf("SIZE: %ld", sizeof(uint32_t)*4*1000);

  //uint32_t arms_index = 0; // Current index value for arm_values which is used to iterate to a new set of PDRs in the array when a breakpoint has been reached

  for(int t = 1; t <= ITERATIONS; ++t){

    arm = swucb_get_arm(&args); // Get an arm from the algorithm
    
    sample = draw_sample(arms[arm]); // Draw a sample based on the current PDR of the selected arm

    //fix_link_t link;
    //printf("SIZE: %ld", sizeof(fix_link_t));

    swucb_append_result(&args, sample, arm, &links[t-1]); // Append the result to the sliding window in args

    successes += sample;

    //pdr = (double)successes / t;

    //printf("fixed %d %d %d %lf\n",window_size,time_interval,t,pdr);
    //printf("float %d %d %d %d\n",window_size,time_interval,t,successes);

    if(t % time_interval == 0){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

      //*arms_index = *arms_index + 1; 
      gen_pdr(arms);
    }

  }
  //swucb_destroy(&args);
  /*
  printf("Size_current = %d\n",window.size_current);
  printf("Size_max = %d\n",window.size_max);

  for(int a = 0; a < 16; a++){
    printf("Selections arm %d = %d\n",a, window.selections[a]);
    printf("Sums arm %d = %d\n",a, window.sums[a]);
  }
  */
  /*
  int count = 0;
  for(fix_link_t *link = window.first; link != NULL; link = link->next){
    if (link == NULL){
      printf("Yikes");
    }
    count++;
    printf("Value: %d\n", link->value);
    printf("Arm: %d\n", link->arm);
    printf("Count: %d\n", count);
    }
  */
  /*
  if(window.last->next == NULL){
    printf("HELLYEYEBEBE");
    }*/


  
  pdr = (double)successes / (double)ITERATIONS;
  //Prints the result of the algorithm
  printf("fixed %d %d %lf %lf\n",window_size,time_interval,confidence_level,pdr);

  

  //*arms_index = *arms_index + 1; 
}




// Runs the floating point SWUCB algorithm
// @param: window_size the static sliding window size
// @param: time_interval the static duration in which PDR values remain static
// @param: arm_values an array of PDR values for each channel at different time intervals
//void ducb_float(double discount, int time_interval, arm_values_t *arm_values, int *arms_index, double confidence_level,double bound){
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

    //pdr = (double)successes / t;

    //printf("float %lf %d %d %lf\n",discount,time_interval,t,pdr);
    //printf("float %d %d %d %d\n",window_size,time_interval,t,successes);

    //if(t % time_interval == 0 && t < 1000){ // If a breakpoint has been reached, then select the next set of PDR values for the arms
    if(t % time_interval == 0){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

      gen_pdr(arms);

    }

  }
  ducb_float_destroy(args); // Deallocates the parameters for SWUCB
  
  
  pdr = (double)successes / (double)ITERATIONS;

  // Prints the result of the algorithm
  printf("float %lf %d %lf %lf\n",discount,time_interval,confidence_level,pdr);

  //*arms_index = *arms_index + 1; 

}

// Runs the floating point SWUCB algorithm
// @param: window_size the static sliding window size
// @param: time_interval the static duration in which PDR values remain static
// @param: arm_values an array of PDR values for each channel at different time intervals
void ducb_fixed(double discount, uint32_t time_interval, double confidence_level,double bound){

  history_t history[16];
  ducb_fixed_args_t args;
  ducb_fixed_init(&args,fix16_from_dbl(discount),fix16_from_dbl(confidence_level),fix16_from_dbl(bound), history); //Initializes the parameters used by the algorithm
  
  uint32_t successes = 0;  // Number of successful draws from samples
  uint32_t arm = 0;        // Current selected arm
  uint32_t sample = 0;     // Sample which is either successful (1) of failed (0)

  //int arms_index = 0; // Current index value for arm_values which is used to iterate to a new set of PDRs in the array when a breakpoint has been reached

  double pdr = -1;

  double arms[NUM_ARMS];

  gen_pdr(arms);

  for(uint32_t t = 1; t <= ITERATIONS; ++t){

    arm = ducb_fixed_get_arm(&args); // Get an arm from the algorithm
    
    sample = draw_sample(arms[arm]); // Draw a sample based on the current PDR of the selected arm

    ducb_fixed_append_result(&args, sample, arm); // Append the result to the sliding window in args

    successes += sample;

    pdr = (double)successes / t;

    //printf("fixed %lf %d %d %lf\n",discount,time_interval,t,pdr);
    //printf("float %d %d %d %d\n",window_size,time_interval,t,successes);

    //if(t % time_interval == 0 && t < 1000){ // If a breakpoint has been reached, then select the next set of PDR values for the arms
    if(t % time_interval == 0){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

      //*arms_index = *arms_index + 1;

      gen_pdr(arms);

    }

  }
  ducb_fixed_destroy(&args); // Deallocates the parameters for SWUCB
  
  
  pdr = (double)successes / (double)ITERATIONS;

  // Prints the result of the algorithm
  //printf("fixed %lf %d %lf\n",discount,time_interval,pdr);
  printf("fixed %lf %d %lf %lf\n",discount,time_interval,confidence_level,pdr);
  //*arms_index = *arms_index + 1; 

}






void swucb_float_3_arms(int window_size, double confidence_level,double bound){

  
  swucb_float_args_t *args = swucb_float_init(window_size, confidence_level, bound); //Initializes the parameters used by the algorithm
  
  int successes = 0;  // Number of successful draws from samples
  int arm = 0;        // Current selected arm
  int sample = 0;     // Sample which is either successful (1) of failed (0)

  //int arms_index = 0; // Current index value for arm_values which is used to iterate to a new set of PDRs in the array when a breakpoint has been reached

  double arms[3];
  arms[0] = 0.5;
  arms[1] = 0.3;
  arms[2] = 0.4;

  int optimal = 0;

  double regret = 0;
  
  double pdr = -1;

  

  for(int t = 1; t <= ITERATIONS; ++t){

    arm = swucb_float_get_arm(args); // Get an arm from the algorithm
    
    sample = draw_sample(arms[arm]); // Draw a sample based on the current PDR of the selected arm

    swucb_float_append_result(args, sample, arm); // Append the result to the sliding window in args

    successes += sample;

    pdr = (double)successes / t;

    //printf("float %d %d %d %lf\n",window_size,time_interval,t,pdr);
    //printf("float %d %d %d %d\n",window_size,time_interval,t,successes);

    regret += arms[optimal] - arms[arm];

    printf("swucb %d %d %d %lf\n",window_size,successes,t,regret);

    if(t == 3000){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

      arms[2] = 0.9;

      optimal = 2;

    }

    if(t == 5000){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

      arms[2] = 0.4;

      optimal = 0;

    }

  }
  swucb_float_destroy(args); // Deallocates the parameters for SWUCB
  
  
  pdr = (double)successes / (double)ITERATIONS;

  // Prints the result of the algorithm
  //printf("float %d %d %lf\n",window_size,time_interval,pdr);


}


void ducb_float_3_arms(double discount, double confidence_level,double bound){

  
  ducb_float_args_t *args = ducb_float_init(discount,confidence_level,bound); //Initializes the parameters used by the algorithm
  
  int successes = 0;  // Number of successful draws from samples
  int arm = 0;        // Current selected arm
  int sample = 0;     // Sample which is either successful (1) of failed (0)

  //int arms_index = 0; // Current index value for arm_values which is used to iterate to a new set of PDRs in the array when a breakpoint has been reached

  double pdr = -1;


  double arms[3];
  arms[0] = 0.5;
  arms[1] = 0.3;
  arms[2] = 0.4;

  int optimal = 0;

  double regret = 0;

  for(int t = 1; t <= ITERATIONS; ++t){

    arm = ducb_float_get_arm(args); // Get an arm from the algorithm

    //printf("arm: %d\n",arm);
    
    sample = draw_sample(arms[arm]); // Draw a sample based on the current PDR of the selected arm

    ducb_float_append_result(args, sample, arm); // Append the result to the sliding window in args

    successes += sample;

    pdr = (double)successes / t;

    //printf("float %lf %d %d %lf\n",discount,time_interval,t,pdr);
    //printf("float %d %d %d %d\n",window_size,time_interval,t,successes);

    //if(t % time_interval == 0 && t < 1000){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

    regret += arms[optimal] - arms[arm];

    printf("ducb %lf %d %d %lf\n",discount,successes,t,regret);

    if(t == 3000){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

      arms[2] = 0.9;

      optimal = 2;

    }

    if(t == 5000){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

      arms[2] = 0.4;

      optimal = 0;

    }

  }
  ducb_float_destroy(args); // Deallocates the parameters for SWUCB
  
  
  pdr = (double)successes / (double)ITERATIONS;

  // Prints the result of the algorithm
  //printf("float %lf %d %lf\n",discount,time_interval,pdr);

}




void swucb_float_16_arms(int window_size, double confidence_level,double bound){

  
  swucb_float_args_t *args = swucb_float_init(window_size, confidence_level, bound); //Initializes the parameters used by the algorithm
  
  int successes = 0;  // Number of successful draws from samples
  int arm = 0;        // Current selected arm
  int sample = 0;     // Sample which is either successful (1) of failed (0)

  //int arms_index = 0; // Current index value for arm_values which is used to iterate to a new set of PDRs in the array when a breakpoint has been reached

  double arms[16];

  for(int i = 0; i < 16; ++i){
    arms[i] = 0.3;
  }
  arms[0] = 0.5;
  arms[6] = 0.4;

  int optimal = 0;

  double regret = 0;
  
  double pdr = -1;

  

  for(int t = 1; t <= ITERATIONS; ++t){

    arm = swucb_float_get_arm(args); // Get an arm from the algorithm
    
    sample = draw_sample(arms[arm]); // Draw a sample based on the current PDR of the selected arm

    swucb_float_append_result(args, sample, arm); // Append the result to the sliding window in args

    successes += sample;

    pdr = (double)successes / t;

    //printf("float %d %d %d %lf\n",window_size,time_interval,t,pdr);
    //printf("float %d %d %d %d\n",window_size,time_interval,t,successes);

    regret += arms[optimal] - arms[arm];

    printf("swucb %d %d %d %lf\n",window_size,successes,t,regret);

    if(t == 3000){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

      arms[6] = 0.9;

      optimal = 6;

    }

    if(t == 5000){ // If a breakpoint has been reached, then select the next set of PDR values for the arms

      arms[6] = 0.4;

      optimal = 0;

    }

  }
  swucb_float_destroy(args); // Deallocates the parameters for SWUCB
  
  
  pdr = (double)successes / (double)ITERATIONS;

  // Prints the result of the algorithm
  //printf("float %d %d %lf\n",window_size,time_interval,pdr);


}

int main(int argc, char *argv[]){

  srandom(time(NULL));

  //printf("Hello?\n");
  //FILE *file = fopen("src/simulation.txt", "r");
  //FILE *file = fopen("src/simulation2.txt", "r");
  //FILE *file = fopen("src/simulation_3_arms.txt", "r");
  //FILE *file = fopen("src/dataset.txt", "r");
  //FILE *file = fopen("src/pdrtest.txt", "r");
  //FILE *file = fopen("src/pdrtest2.txt", "r");

  /*
  arm_values_t *arm_values = calloc(100000, sizeof(arm_values_t));

  arm_values_t *iter = arm_values;

  int arms_index = 0;

  //Initializes the PDR values from the file into the arm_values array
  for(int i = 0; i < 100000; ++i){
    for(uint32_t arm = 0 ; arm < NUM_ARMS; ++arm){

      fscanf(file,"%lf\n",&(iter[i].arms[arm]));

    }
  }
  fclose(file);
  */
  
  for(int window_size = 100; window_size <= 1000; window_size+=100){
    for(int i = 0; i < 100; ++i){
      /*
      swucb_float(window_size, 1000, 1, 1);
      swucb_float(window_size, 1000, 0.1, 1);
      swucb_float(window_size, 1000, 0.01, 1);
      swucb_float(window_size, 1000, 0.001, 1);
      */
      swucb_float(window_size, 1000, 0.0001, 1);
      
      /*
      swucb_fixed(window_size, 1000, 1, 1);
      swucb_fixed(window_size, 1000, 0.1, 1);
      swucb_fixed(window_size, 1000, 0.01, 1);
      swucb_fixed(window_size, 1000, 0.001, 1);
      */
      swucb_fixed(window_size, 1000, 0.0001, 1);


      /*
      swucb_float(window_size, 500, 1, 1);
      swucb_float(window_size, 500, 0.1, 1);
      swucb_float(window_size, 500, 0.01, 1);
      swucb_float(window_size, 500, 0.001, 1);
      */
      swucb_float(window_size, 500, 0.0001, 1);

      /*
      swucb_fixed(window_size, 500, 1, 1);
      swucb_fixed(window_size, 500, 0.1, 1);
      swucb_fixed(window_size, 500, 0.01, 1);
      swucb_fixed(window_size, 500, 0.001, 1);
      */
      swucb_fixed(window_size, 500, 0.0001, 1);

      /*
      swucb_float(window_size, 200, 1, 1);
      swucb_float(window_size, 200, 0.1, 1);
      swucb_float(window_size, 200, 0.01, 1);
      swucb_float(window_size, 200, 0.001, 1);
      */
      swucb_float(window_size, 200, 0.0001, 1);

      /*
      swucb_fixed(window_size, 200, 1, 1);
      swucb_fixed(window_size, 200, 0.1, 1);
      swucb_fixed(window_size, 200, 0.01, 1);
      swucb_fixed(window_size, 200, 0.001, 1);
      */
      swucb_fixed(window_size, 200, 0.0001, 1);
    }
  }
  

  /*
  for(int window_size = 100; window_size <= 1000; window_size+=100){
    for(int i = 0; i < 100; ++i){
      swucb_float(window_size, 1000, 0, 1);

      swucb_fixed(window_size, 1000, 0, 1);


      swucb_float(window_size, 500, 0, 1);

      swucb_fixed(window_size, 500, 0, 1);


      swucb_float(window_size, 200, 0, 1);

      swucb_fixed(window_size, 200, 0, 1);
    }
  }
  */

  /*
  for(int window_size = 1000; window_size <= 10000; window_size+=1000){
    for(int i = 0; i < 100; ++i){
      swucb_float(window_size, 10000, 1, 1);
      swucb_float(window_size, 10000, 0.01, 1);
      swucb_float(window_size, 10000, 0.0001, 1);

      swucb_fixed(window_size, 10000, 1, 1);
      swucb_fixed(window_size, 10000, 0.01, 1);
      swucb_fixed(window_size, 10000, 0.0001, 1);


      swucb_float(window_size, 5000, 1, 1);
      swucb_float(window_size, 5000, 0.01, 1);
      swucb_float(window_size, 5000, 0.0001, 1);

      swucb_fixed(window_size, 5000, 1, 1);
      swucb_fixed(window_size, 5000, 0.1, 1);
      swucb_fixed(window_size, 5000, 0.01, 1);
      swucb_fixed(window_size, 5000, 0.001, 1);
      swucb_fixed(window_size, 5000, 0.0001, 1);
      swucb_fixed(window_size, 5000, 0.005, 1);


      swucb_float(window_size, 2000, 1, 1);
      swucb_float(window_size, 2000, 0.01, 1);
      swucb_float(window_size, 2000, 0.0001, 1);

      swucb_fixed(window_size, 2000, 1, 1);
      swucb_fixed(window_size, 2000, 0.01, 1);
      swucb_fixed(window_size, 2000, 0.0001, 1);
    }
  }
  */
  /*
  for(int window_size = 100; window_size <= 1000; window_size+=100){
    for(double param = 0.1; param <= 1.0; param += 0.1){
      for(int i = 0; i < 40; ++i){
	
	swucb_float(window_size, 1000, param, 1);
	swucb_fixed(window_size, 1000, param, 1);

	swucb_float(window_size, 500, param, 1);
	swucb_fixed(window_size, 500, param, 1);


	swucb_float(window_size, 200, param, 1);
	swucb_fixed(window_size, 200, param, 1);
	

	swucb_float(window_size, 50, param, 1);
	swucb_fixed(window_size, 50, param, 1);
      }
    }
  }
*/

  /*
  for(int window_size = 50; window_size <= 300; window_size+=50){
    for(double param = 0.1; param <= 1.0; param += 0.1){
      for(int i = 0; i < 40; ++i){
	

	swucb_float(window_size, 50, param, 1);
	swucb_fixed(window_size, 50, param, 1);
      }
    }
  }
  */
  
  /*
  for(double discount = 0.990; discount <= 1.00; discount += 0.001){
    for(double param = 0.1; param <= 1.0; param += 0.1){
      for(int i = 0; i < 100; ++i){
      
        ducb_float(discount, 1000, param, 1);
        ducb_fixed(discount, 1000, param, 1);

	ducb_float(discount, 500, param, 1);
        ducb_fixed(discount, 500, param, 1);


        ducb_float(discount, 200, param, 1);
        ducb_fixed(discount, 200, param, 1);
      }
    }
  }
  */
  /*
  for(double discount = 0.950; discount <= 1.0; discount += 0.005){
    for(int i = 0; i < 100; ++i){
      ducb_float(discount, 1000, 1, 1);
      ducb_float(discount, 1000, 0.1, 1);
      ducb_float(discount, 1000, 0.01, 1);
      ducb_float(discount, 1000, 0.001, 1);
      ducb_float(discount, 1000, 0.0001, 1);

      ducb_fixed(discount, 1000, 1, 1);
      ducb_fixed(discount, 1000, 0.1, 1);
      ducb_fixed(discount, 1000, 0.01, 1);
      ducb_fixed(discount, 1000, 0.001, 1);
      ducb_fixed(discount, 1000, 0.0001, 1);


      ducb_float(discount, 500, 1, 1);
      ducb_float(discount, 500, 0.1, 1);
      ducb_float(discount, 500, 0.01, 1);
      ducb_float(discount, 500, 0.001, 1);
      ducb_float(discount, 500, 0.0001, 1);

      ducb_fixed(discount, 500, 1, 1);
      ducb_fixed(discount, 500, 0.1, 1);
      ducb_fixed(discount, 500, 0.01, 1);
      ducb_fixed(discount, 500, 0.001, 1);
      ducb_fixed(discount, 500, 0.0001, 1);


      ducb_float(discount, 200, 1, 1);
      ducb_float(discount, 200, 0.1, 1);
      ducb_float(discount, 200, 0.01, 1);
      ducb_float(discount, 200, 0.001, 1);
      ducb_float(discount, 200, 0.0001, 1);

      ducb_fixed(discount, 200, 1, 1);
      ducb_fixed(discount, 200, 0.1, 1);
      ducb_fixed(discount, 200, 0.01, 1);
      ducb_fixed(discount, 200, 0.001, 1);
      ducb_fixed(discount, 200, 0.0001, 1);
    }
  }
  */

  /*
  for(int i = 0; i < 100; ++i){
      swucb_float(83, 200, 1, 1);
      swucb_float(83, 200, 0.1, 1);
      swucb_float(83, 200, 0.01, 1);
      swucb_float(83, 200, 0.001, 1);
      swucb_float(83, 200, 0.0001, 1);
      swucb_float(83, 200, 0.005, 1);

      swucb_fixed(83, 200, 1, 1);
      swucb_fixed(83, 200, 0.1, 1);
      swucb_fixed(83, 200, 0.01, 1);
      swucb_fixed(83, 200, 0.001, 1);
      swucb_fixed(83, 200, 0.0001, 1);
      swucb_fixed(83, 200, 0.005, 1);


    }
  */
  /*
  for(int i = 0; i < 40; ++i){
    //swucb_float_3_arms(1214, 0.6,1);
    //swucb_float_3_arms(10000, 0.5,1);
    //ducb_float_3_arms(0.9975, 0.6,1);
    //ducb_float_3_arms(1.0, 0.5,1);


    swucb_float_3_arms(1214, 3,1);
    swucb_float_3_arms(10000, 3,1);

    //swucb_float_16_arms(1214, 2,1);
    //swucb_float_16_arms(10000, 2,1);
  }
  */
  
  
  
  
  /*
  for(int i = 0; i < 40; ++i){
      swucb_float(10000, TIME_INTERVAL, arm_values, &arms_index);
      //arms_index = 0; //For pdrtest2.txt
      
      //swucb_float(1000, TIME_INTERVAL, arm_values, &arms_index);
      swucb_fixed(10000, TIME_INTERVAL, arm_values, (uint32_t *) &arms_index);
    }
  */
  
  //swucb_float(1000, TIME_INTERVAL, arm_values, &arms_index);

  /*
  for(double discount = 0.990; discount <= 1.00; discount += 0.001){

    for(int i = 0; i < 40; ++i){
      ducb_float(discount, TIME_INTERVAL, arm_values, &arms_index, 0.6,1);
      //ducb_fixed(discount, TIME_INTERVAL, arm_values, (uint32_t *)&arms_index, CONFIDENCE_LEVEL,BOUND);
    }
  }
  */
  
  

  /*
  for(int i = 0; i < 40; ++i){
      ducb_float(0.5, TIME_INTERVAL, arm_values, &arms_index);
    }
  */

  /*
  for(int i = 0; i < 20; ++i){
    ducb_float(0.5, TIME_INTERVAL, arm_values, &arms_index);
    ducb_float(1.0, TIME_INTERVAL, arm_values, &arms_index);
  }
  */

  /*
  for(int i = 0; i < 40; ++i){
    ducb_float(1.0, TIME_INTERVAL, arm_values, &arms_index);
    ducb_fixed(1.0, TIME_INTERVAL, arm_values, (uint32_t *)&arms_index);
  }
  */
  
  //free(arm_values);
  return 0;

}
