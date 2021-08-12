#include "swucb-fixed.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>

//Gives the minimum value of a and b
#define min(a,b) ((a) < (b) ? (a) : (b))

//Datatype for an entry in the sliding window
struct link{

  fix16_t value;
  uint32_t arm;
  link_t *next; // Next entry in the window

};

// Datatype for a sliding window, which is basically a linked list
struct window{

  link_t *first; // First entry in the window
  link_t *last;   // Last entry in the window
  uint32_t size_current; // Current size of the sliding window
  uint32_t size_max;     // Maximum size of the sliding window
  uint32_t selections[NUM_ARMS];  // Stores the number of samples of each arm currently in the sliding window
  uint32_t sums[NUM_ARMS]; //Stores the sum of successful samples for each arm that are currently in the sliding window

};


// Creates an entry for the sliding window
// @param value the value of the sample, which is success (1) or failure (0)
// @param arm the arm that the sample belongs to
// @return the allocated entry
static link_t *link_init(fix16_t value, uint32_t arm){

  link_t *link = calloc(1, sizeof(link_t));
  link->value = value;
  link->arm = arm;
  link->next = NULL;

  return link;

}

// Creates a sliding window
// @param window_size the maximum possible size of the sliding window
// @return an allocated sliding window
static window_t *window_init(uint32_t window_size){

  window_t *window = calloc(1, sizeof(window_t));
  window->size_current = 0;
  window->size_max = window_size;
  for(uint32_t arm = 0; arm < NUM_ARMS; arm++){
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
static void window_append(window_t *window, fix16_t value, uint32_t arm){

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
static fix16_t get_mean(window_t *window, uint32_t arm){

  fix16_t sum = window->sums[arm]  << FRACTIONAL_BITS;
  fix16_t selections = window->selections[arm]  << FRACTIONAL_BITS;

  
  return fix16_div(sum, selections);
  
}

// Gets the upper bound (or "padding function") of an arm
// @param window the sliding window
// @param t the current time step
// @param arm the arm to calculate the upper bound from
// @param logs the array mapping indexes to their corresponding fixed point log values
static fix16_t get_upper_bound(window_t *window, uint32_t t, uint32_t arm, fix16_t logs[]){

  uint32_t selections = window->selections[arm];

  fix16_t log_res = logs[min(window->size_max,t)];
  fix16_t dividend = fix16_mul(CONFIDENCE_LEVEL, log_res);
  fix16_t divisor = selections << FRACTIONAL_BITS; // Change selections to its corresponding fixed point value
  fix16_t quotient = fix16_div(dividend, divisor);
  fix16_t root = fix16_sqrt(quotient);
  fix16_t result = fix16_mul(BOUND, root);

  return result;

}

// Selects an arm based on the current sliding window and time step
// @param window the sliding window
// @param t the current time step
// @param logs the array mapping indexes to their corresponding fixed point log values
// @return the selected arm
static uint32_t select_arm(window_t *window, uint32_t t, fix16_t logs[]){

  uint32_t max_arm = 0;   // arm with the current maximum result
  fix16_t max_result = 0; // the current maximum result
  for(uint32_t arm = 0; arm < NUM_ARMS; ++arm){

    if(window->selections[arm] == 0){ // If an arm does not have any samples in the sliding window, then select it
      
      return arm;
    }

    fix16_t mean = get_mean(window, arm);

    fix16_t upper_bound = get_upper_bound(window, t, arm, logs);
    
    fix16_t current_result = mean + upper_bound;
    
    if(current_result > max_result){

      max_arm = arm;
      max_result = current_result;

    }
  }

  return max_arm;

}

// Gets fixed point log values
// @return array where each index maps to its corresponding log value in fixed point format
fix16_t *get_logs(){

  FILE *file = fopen("src/log10000.txt", "r");

  uint32_t *logs = calloc(1002, sizeof(fix16_t));

  for(uint32_t i = 1 ; fscanf(file,"%" PRIu32 "\n",&logs[i]) == 1  && i <= 1000; ++i);
  fclose(file);

  return logs;

}

// Initializes the algorithm with needed parameters
// @param window_size the max fixed sliding window size of swucb
// @return the parameters needed to run swucb
swucb_args_t *swucb_init(uint32_t window_size){

  swucb_args_t *args = calloc(1, sizeof(swucb_args_t));

  args->window = window_init(window_size);
  args->t = 1;
  args->logs = get_logs();

  return args;

}

// Selects an arm based on the current time step and sliding window
// @param args stores the current time step, sliding window and fixed point log values
// @return the selected arm
uint32_t swucb_get_arm(swucb_args_t *args){

  window_t *window = args->window;
  uint32_t t = args->t;
  fix16_t *logs = args->logs;
  
  if(t < NUM_ARMS){ // Select each arm once at first so that each arm has a sample in the sliding window

    return t-1;

  }
  else{

    uint32_t arm = select_arm(window, t, logs);

    return arm;

  }

}

// Appends a sample to the sliding window
// @param args stores the current time step, sliding window and fixed point log values
// @param result the result of the last sample draw, which is either a success (1) or a failure (0)
// @param arm the arm which was selected by swucb
void swucb_append_result(swucb_args_t *args, uint32_t result, uint32_t arm){

  window_append(args->window, result, arm);
  args->t++;

}

// Deallocates the sliding window
// @param args stores the current time step, sliding window and fixed point log values
void swucb_destroy(swucb_args_t *args){

  window_destroy(args->window);
  free(args->logs);
  free(args);

}












