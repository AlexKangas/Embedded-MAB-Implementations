#include "ducb-float.h"


#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

typedef struct link link_t;
typedef struct history history_t;

//Datatype for an entry in the sliding history
struct link{

  double n;
  double rewards;
  

};

// Datatype for a sliding history, which is basically a linked list
struct history{

  link_t arms[NUM_ARMS];

};

// Creates a sliding history
// @param history_size the maximum possible size of the sliding history
// @return an allocated sliding history
static history_t *history_init(){

  history_t *history = calloc(1, sizeof(history_t));

  for(int i = 0; i < NUM_ARMS; ++i){

    history->arms[i].n = 0;
    history->arms[i].rewards = 0;

  }

  return history;

}

// Appends a sample into the sliding history
// @param history the sliding history
// @param value the sample value to be appended
// @param arm the arm that the sample belongs to
static void history_append(history_t *history, int value, int arm, double discount){

  for(int i = 0; i < NUM_ARMS; ++i){

    history->arms[i].n *= discount;
    history->arms[i].rewards *= discount;

  }

  history->arms[arm].n += 1;
  history->arms[arm].rewards += value;

}

// Deallocates the sliding history
// @param history the sliding history to be deallocated
static void history_destroy(history_t *history){

  free(history);

}

static double get_mean(history_t *history, int arm){

  return history->arms[arm].rewards/history->arms[arm].n;

}



// Gets the upper bound (or "padding function") of an arm
// @param history the sliding history
// @param t the current time step
// @param arm the arm to calculate the upper bound from
static double get_exploration(history_t *history, int arm,double confidence_level,double bound){


  double n_sum = 0;
  for(int a = 0; a < NUM_ARMS; ++a){
    n_sum += history->arms[a].n;
  }

  double dividend = log(n_sum);
  
  double divisior = history->arms[arm].n;

  double result = 2*bound*sqrt(confidence_level*dividend/divisior);

  return result;

}

// Selects an arm based on the current sliding history and time step
// @param history the sliding history
// @param t the current time step
// @return the selected arm
static int select_arm(history_t *history,double confidence_level,double bound){

  int max_arm = 0;        // arm with the current maximum result
  double max_result = 0;  // the current maximum result
  
  for(int arm = 0; arm < NUM_ARMS; ++arm){

    double mean = get_mean(history, arm);
    double exploration = get_exploration(history, arm, confidence_level, bound);
    double current_result = mean + exploration;

    
    if(current_result > max_result){

      max_arm = arm;
      max_result = current_result;

    }
  }
  return max_arm;

}

// Initializes the algorithm with needed parameters
// @param history_size the max fixed sliding history size of ducb
// @return the parameters needed to run ducb
ducb_float_args_t *ducb_float_init(double discount,double confidence_level,double bound){

  ducb_float_args_t *args = calloc(1, sizeof(ducb_float_args_t));

  args->history = history_init();
  args->t = 1;
  args->discount = discount;
  args->confidence_level = confidence_level;
  args->bound = bound;

  return args;

}

// Selects an arm based on the current time step and sliding history
// @param args stores the current time step and sliding history
// @return the selected arm
int ducb_float_get_arm(ducb_float_args_t *args){

  history_t *history = args->history;
  int t = args->t;
  double confidence_level = args->confidence_level;
  double bound = args->bound;
  
  if(t-1 < NUM_ARMS){ // Select each arm once at first so that each arm has a sample in the sliding history

    return t-1;

  }
  else{

    return select_arm(history, confidence_level,bound);

  }

}

// Appends a sample to the sliding history
// @param args stores the current time step and sliding history
// @param result the result of the last sample draw, which is either a success (1) or a failure (0)
// @param arm the arm which was selected by ducb
void ducb_float_append_result(ducb_float_args_t *args, int result, int arm){

  history_append(args->history, result, arm, args->discount);
  args->t++;

}

// Deallocates the sliding history
// @param args stores the current time step and sliding history
void ducb_float_destroy(ducb_float_args_t *args){

  history_destroy(args->history);
  free(args);

}

