#pragma once

#include "common.h"

//Parameters for the SWUCB algorithm for callculating the upper bound
#define FLOAT_CONFIDENCE_LEVEL (double)CONFIDENCE_LEVEL_NUM
#define FLOAT_BOUND (double)BOUND_NUM

typedef struct link link_t; //Datatype for an entry in the sliding window

typedef struct window window_t; // Datatype for a sliding window, which is basically a linked list

// Arguments for running the algorithm
typedef struct {

  window_t *window; // Sliding window
  int t;            // Current time step

} swucb_float_args_t;

// Initializes the algorithm with needed parameters
// @param window_size the max fixed sliding window size of swucb
// @return the parameters needed to run swucb
swucb_float_args_t *swucb_float_init(int window_size);

// Selects an arm based on the current time step and sliding window
// @param args stores the current time step and sliding window
// @return the selected arm
int swucb_float_get_arm(swucb_float_args_t *args);

// Appends a sample to the sliding window
// @param args stores the current time step and sliding window
// @param result the result of the last sample draw, which is either a success (1) or a failure (0)
// @param arm the arm which was selected by swucb
void swucb_float_append_result(swucb_float_args_t *args, int result, int arm);

// Deallocates the sliding window
// @param args stores the current time step and sliding window
void swucb_float_destroy(swucb_float_args_t *args);


