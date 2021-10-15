#pragma once

#define NUM_ARMS 16 //Number of arms used

typedef struct link link_t; //Datatype for an entry in the sliding history

typedef struct history history_t; // Datatype for a sliding history, which is basically a linked list

// Arguments for running the algorithm
typedef struct {

  history_t *history; 
  int t;            // Current time step
  double discount;
  double confidence_level;
  double bound;
  

} ducb_float_args_t;

// Initializes the algorithm with needed parameters
// @param history_size the max fixed sliding history size of ducb
// @return the parameters needed to run ducb
//ducb_float_args_t *ducb_float_init(double discount);
ducb_float_args_t *ducb_float_init(double discount,double confidence_level,double bound);

// Selects an arm based on the current time step and sliding history
// @param args stores the current time step and sliding history
// @return the selected arm
int ducb_float_get_arm(ducb_float_args_t *args);

// Appends a sample to the sliding history
// @param args stores the current time step and sliding history
// @param result the result of the last sample draw, which is either a success (1) or a failure (0)
// @param arm the arm which was selected by ducb
void ducb_float_append_result(ducb_float_args_t *args, int result, int arm);

// Deallocates the sliding history
// @param args stores the current time step and sliding history
void ducb_float_destroy(ducb_float_args_t *args);


