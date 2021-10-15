#pragma once

#include "fixed-point.h"

#define NUM_ARMS 16 //Number of arms used

typedef struct history{
  
  fix16_t n;
  fix16_t rewards;

} history_t;

// Arguments for running the algorithm
typedef struct {

  history_t history[NUM_ARMS]; 
  uint32_t t;            // Current time step
  fix16_t discount;
  fix16_t confidence_level;
  fix16_t bound;
  
} ducb_fixed_args_t;

// Initializes the algorithm with needed parameters
// @param history_size the max fixed sliding history size of ducb
// @return the parameters needed to run ducb
ducb_fixed_args_t *ducb_fixed_init(ducb_fixed_args_t *args,fix16_t discount,fix16_t confidence_level,fix16_t bound);

// Selects an arm based on the current time step and sliding history
// @param args stores the current time step and sliding history
// @return the selected arm
uint32_t ducb_fixed_get_arm(ducb_fixed_args_t *args);

// Appends a sample to the sliding history
// @param args stores the current time step and sliding history
// @param result the result of the last sample draw, which is either a success (1) or a failure (0)
// @param arm the arm which was selected by ducb
void ducb_fixed_append_result(ducb_fixed_args_t *args, uint32_t result, uint32_t arm);
