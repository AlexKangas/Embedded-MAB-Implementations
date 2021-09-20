#pragma once

#include "fixed-point.h"

#include "common.h"

//#define CONFIDENCE_LEVEL ((uint32_t)CONFIDENCE_LEVEL_NUM) << FRACTIONAL_BITS
//#define BOUND ((int32_t)BOUND_NUM) << FRACTIONAL_BITS

typedef struct link link_t; //Datatype for an entry in the sliding history

typedef struct history history_t; // Datatype for a sliding history, which is basically a linked list

// Arguments for running the algorithm
typedef struct {

  history_t *history; // Sliding history
  uint32_t t;            // Current time step
  fix16_t discount;
  fix16_t *logs;    //Array of fixed point log values
  fix16_t confidence_level;
  fix16_t bound;
  
} ducb_fixed_args_t;

// Initializes the algorithm with needed parameters
// @param history_size the max fixed sliding history size of ducb
// @return the parameters needed to run ducb
ducb_fixed_args_t *ducb_fixed_init(fix16_t discount,fix16_t confidence_level,fix16_t bound);

// Selects an arm based on the current time step and sliding history
// @param args stores the current time step and sliding history
// @return the selected arm
uint32_t ducb_fixed_get_arm(ducb_fixed_args_t *args);

// Appends a sample to the sliding history
// @param args stores the current time step and sliding history
// @param result the result of the last sample draw, which is either a success (1) or a failure (0)
// @param arm the arm which was selected by ducb
void ducb_fixed_append_result(ducb_fixed_args_t *args, uint32_t result, uint32_t arm);

// Deallocates the sliding history
// @param args stores the current time step and sliding history
void ducb_fixed_destroy(ducb_fixed_args_t *args);


