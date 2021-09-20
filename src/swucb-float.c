#include "swucb-float.h"


#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

#include "common.h"

//Gives the minimum value of a and b
//#define min(a,b) ((a) < (b) ? (a) : (b))



typedef struct link link_t;
typedef struct window window_t;

//Datatype for an entry in the sliding window
struct link{

  int value;
  int arm;
  link_t *next; // Next entry in the window

};

// Datatype for a sliding window, which is basically a linked list
struct window{

  link_t *first;    // First entry in the window
  link_t *last;     // Last entry in the window
  int size_current; // Current size of the sliding window
  int size_max;     // Maximum size of the sliding window
  int selections[NUM_ARMS]; // Stores the number of samples of each arm currently in the sliding window
  int sums[NUM_ARMS]; //Stores the sum of successful samples for each arm that are currently in the sliding window

};

static double min(double a, double b){

  if(a < b){
    return a;
  }
  else{
    return b;
  }

}

// Creates an entry for the sliding window
// @param value the value of the sample, which is success (1) or failure (0)
// @param arm the arm that the sample belongs to
// @return the allocated entry
static link_t *link_init(int value, int arm){

  link_t *link = calloc(1, sizeof(link_t));
  link->value = value;
  link->arm = arm;
  link->next = NULL;

  return link;

}

// Creates a sliding window
// @param window_size the maximum possible size of the sliding window
// @return an allocated sliding window
static window_t *window_init(int window_size){

  window_t *window = calloc(1, sizeof(window_t));
  window->size_current = 0;
  window->size_max = window_size;
  for(int arm = 0; arm < NUM_ARMS; arm++){
    window->selections[arm] = 0;
    window->sums[arm] = 0;
  }
  window->first = NULL;
  window->last = NULL;

  return window;

}

// Appends a sample into the sliding window
// @param window the sliding window
// @param value the sample value to be appended
// @param arm the arm that the sample belongs to
static void window_append(window_t *window, int value, int arm){

  link_t *link = link_init(value, arm);

  if(window->last == NULL){ // If the sliding window is empty

    window->first = link;
    window->last = link;

  }
  else{ // If the sliding window already contains entries

    window->last->next = link;
    window->last = link;

  }

  window->selections[arm]++;
  window->sums[arm] += value;

  if(window->size_current < window->size_max){

    window->size_current++;

  }
  else{ // If the sliding window is larger than it's max size, then remove the first entry

    link_t *temp = window->first;
    window->first = window->first->next;
    window->selections[temp->arm]--;
    window->sums[temp->arm] -= temp->value;
    free(temp);

  }

}

// Deallocates the sliding window
// @param window the sliding window to be deallocated
static void window_destroy(window_t *window){

  while(window->first != NULL){

    link_t *temp = window->first;
    window->first = window->first->next;
    free(temp);

  }

  free(window);

}

// Gets the mean value of the samples in the sliding window of an arm
// @param window the sliding window
// @param arm the arm to calculate the mean from
static double get_mean(window_t *window, int arm){

  int sum = window->sums[arm];
  int selections = window->selections[arm];

  
  return (double)sum / (double)selections;
  

}

// Gets the upper bound (or "padding function") of an arm
// @param window the sliding window
// @param t the current time step
// @param arm the arm to calculate the upper bound from
//static double get_upper_bound(window_t *window, int t, int arm){
static double get_upper_bound(window_t *window, int t, int arm, double confidence_level, double bound){

  int selections = window->selections[arm];

  double log_res = log((double)min(window->size_max,t));
  //double dividend = FLOAT_CONFIDENCE_LEVEL * log_res;
  double dividend = confidence_level * log_res;
  double divisor = (double)selections;
  double quotient = dividend/divisor;
  double root = sqrt(quotient);
  //double result = FLOAT_BOUND * root;
  double result = bound * root;

  return result;

}

// Selects an arm based on the current sliding window and time step
// @param window the sliding window
// @param t the current time step
// @return the selected arm
//static int select_arm(window_t *window, int t){
static int select_arm(window_t *window, int t, double confidence_level, double bound){

  int max_arm = 0;        // arm with the current maximum result
  double max_result = 0;  // the current maximum result
  
  for(int arm = 0; arm < NUM_ARMS; ++arm){

    if(window->selections[arm] == 0){ // If an arm does not have any samples in the sliding window, then select it
      
      return arm;
    }

    double mean = get_mean(window, arm);
    //double upper_bound = get_upper_bound(window, t, arm);
    double upper_bound = get_upper_bound(window, t, arm, confidence_level, bound);
    double current_result = mean + upper_bound;

    //printf("Mean: %lf, exp: %lf, curr: %lf\n", mean, upper_bound, current_result);
    
    if(current_result > max_result){

      max_arm = arm;
      max_result = current_result;

    }
  }

  return max_arm;

}

// Initializes the algorithm with needed parameters
// @param window_size the max fixed sliding window size of swucb
// @return the parameters needed to run swucb
//swucb_float_args_t *swucb_float_init(int window_size){
swucb_float_args_t *swucb_float_init(int window_size, double confidence_level, double bound){

  swucb_float_args_t *args = calloc(1, sizeof(swucb_float_args_t));

  args->window = window_init(window_size);
  args->t = 1;
  args->confidence_level = confidence_level;
  args->bound = bound;

  return args;

}

// Selects an arm based on the current time step and sliding window
// @param args stores the current time step and sliding window
// @return the selected arm
int swucb_float_get_arm(swucb_float_args_t *args){

  window_t *window = args->window;
  int t = args->t;

  double confidence_level = args->confidence_level;
  double bound = args->bound;
  
  //if(t < NUM_ARMS){ // Select each arm once at first so that each arm has a sample in the sliding window
  if(t-1 < NUM_ARMS){ // Select each arm once at first so that each arm has a sample in the sliding window

    return t-1;

  }
  else{

    //return select_arm(window, t);
    return select_arm(window, t, confidence_level, bound);

  }

}

// Appends a sample to the sliding window
// @param args stores the current time step and sliding window
// @param result the result of the last sample draw, which is either a success (1) or a failure (0)
// @param arm the arm which was selected by swucb
void swucb_float_append_result(swucb_float_args_t *args, int result, int arm){

  window_append(args->window, result, arm);
  args->t++;

}

// Deallocates the sliding window
// @param args stores the current time step and sliding window
void swucb_float_destroy(swucb_float_args_t *args){

  window_destroy(args->window);
  free(args);

}






