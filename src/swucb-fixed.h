#pragma once

#include "fixed-point.h"

#include "common.h"

// Parameters used by swucb to calculate upper bounds
//#define CONFIDENCE_LEVEL ((uint32_t)CONFIDENCE_LEVEL_NUM) << FRACTIONAL_BITS
//#define CONFIDENCE_LEVEL ((int32_t)CONFIDENCE_LEVEL_NUM) << FRACTIONAL_BITS
//#define BOUND BOUND_NUM << FRACTIONAL_BITS
//#define BOUND ((int32_t)BOUND_NUM) << FRACTIONAL_BITS

typedef struct fix_link fix_link_t; //Datatype for an entry in the sliding window
typedef struct window window_t; // Datatype for a sliding window, which is basically a linked list

//Datatype for an entry in the sliding window
struct fix_link{

  uint32_t value;
  uint32_t arm;
  fix_link_t *next; // Next entry in the window

};

// Datatype for a sliding window, which is basically a linked list
struct window{

  fix_link_t *first; // First entry in the window
  fix_link_t *last;   // Last entry in the window
  uint32_t size_current; // Current size of the sliding window
  uint32_t size_max;     // Maximum size of the sliding window
  uint32_t selections[NUM_ARMS];  // Stores the number of samples of each arm currently in the sliding window
  uint32_t sums[NUM_ARMS]; //Stores the sum of successful samples for each arm that are currently in the sliding window

};


// Arguments for running the algorithm
typedef struct {

  //window_t *window; //Sliding window
  window_t *window; //Sliding window
  //window_t window; //Sliding window
  uint32_t t;       //Current time steps
  //fix16_t *logs;    //Array of fixed point log values
  fix16_t confidence_level;
  fix16_t bound;

} swucb_args_t;


// Initializes the algorithm with needed parameters
// @param window_size the max fixed sliding window size of swucb
// @return the parameters needed to run swucb
//swucb_args_t *swucb_init(uint32_t window_size);
void swucb_init(swucb_args_t *args,window_t *window,uint32_t window_size,fix16_t confidence_level,fix16_t bound);
//void swucb_init(swucb_args_t *args,uint32_t window_size,fix16_t confidence_level,fix16_t bound);


// Selects an arm based on the current time step and sliding window
// @param args stores the current time step, sliding window and fixed point log values
// @return the selected arm
uint32_t swucb_get_arm(swucb_args_t *args);

// Appends a sample to the sliding window
// @param args stores the current time step, sliding window and fixed point log values
// @param result the result of the last sample draw, which is either a success (1) or a failure (0)
// @param arm the arm which was selected by swucb
void swucb_append_result(swucb_args_t *args, uint32_t result, uint32_t arm, fix_link_t *link);

// Deallocates the sliding window
// @param args stores the current time step, sliding window and fixed point log values
void swucb_destroy(swucb_args_t *args);

// Gets fixed point log values
// @return array where each index maps to its corresponding log value in fixed point format
fix16_t *get_logs();

fix16_t fix16_log(fix16_t inValue);
