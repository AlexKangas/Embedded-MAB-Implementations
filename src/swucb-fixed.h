#pragma once

#include "fixed-point.h"

#include "common.h"

// Parameters used by swucb to calculate upper bounds
//#define CONFIDENCE_LEVEL ((uint32_t)CONFIDENCE_LEVEL_NUM) << FRACTIONAL_BITS
//#define CONFIDENCE_LEVEL ((int32_t)CONFIDENCE_LEVEL_NUM) << FRACTIONAL_BITS
//#define BOUND BOUND_NUM << FRACTIONAL_BITS
//#define BOUND ((int32_t)BOUND_NUM) << FRACTIONAL_BITS

typedef struct link link_t; //Datatype for an entry in the sliding window
typedef struct window window_t; // Datatype for a sliding window, which is basically a linked list

// Arguments for running the algorithm
typedef struct {

  window_t *window; //Sliding window
  uint32_t t;       //Current time steps
  fix16_t *logs;    //Array of fixed point log values
  fix16_t confidence_level;
  fix16_t bound;

} swucb_args_t;


// Initializes the algorithm with needed parameters
// @param window_size the max fixed sliding window size of swucb
// @return the parameters needed to run swucb
//swucb_args_t *swucb_init(uint32_t window_size);
swucb_args_t *swucb_init(uint32_t window_size,fix16_t confidence_level,fix16_t bound);


// Selects an arm based on the current time step and sliding window
// @param args stores the current time step, sliding window and fixed point log values
// @return the selected arm
uint32_t swucb_get_arm(swucb_args_t *args);

// Appends a sample to the sliding window
// @param args stores the current time step, sliding window and fixed point log values
// @param result the result of the last sample draw, which is either a success (1) or a failure (0)
// @param arm the arm which was selected by swucb
void swucb_append_result(swucb_args_t *args, uint32_t result, uint32_t arm);

// Deallocates the sliding window
// @param args stores the current time step, sliding window and fixed point log values
void swucb_destroy(swucb_args_t *args);

// Gets fixed point log values
// @return array where each index maps to its corresponding log value in fixed point format
fix16_t *get_logs();

fix16_t fix16_log(fix16_t inValue);
