#include "ducb-fixed.h"

// Creates a sliding history
// @param history_size the maximum possible size of the sliding history
// @return an allocated sliding history
static void history_init(history_t *history){

  for(uint32_t i = 0; i < 16; ++i){

    history[i].n = 0;
    history[i].rewards = 0;
  }

}

// Appends a sample uint32_to the sliding history
// @param history the sliding history
// @param value the sample value to be appended
// @param arm the arm that the sample belongs to
static void history_append(history_t *history, uint32_t value, uint32_t arm, fix16_t discount){

  for(uint32_t i = 0; i < 16; ++i){
    
    history[i].n = fix16_mul(history[i].n, discount);
    history[i].rewards = fix16_mul(history[i].rewards, discount);
    
  }
  
  history[arm].n = history[arm].n + (1 << FRACTIONAL_BITS);
  history[arm].rewards = history[arm].rewards + (value << FRACTIONAL_BITS);

}

static fix16_t get_mean(history_t *history, uint32_t arm){

  return fix16_div(history[arm].rewards, history[arm].n);

}



// Gets the exploration (or "padding function") of an arm
// @param history the sliding history
// @param t the current time step
// @param arm the arm to calculate the upper bound from
static fix16_t get_exploration(history_t *history, uint32_t arm,fix16_t confidence_level,fix16_t bound){
  
  fix16_t n_sum = 0;
  for(uint32_t a = 0; a < 16; ++a){
    n_sum += history[a].n;
  }

  fix16_t dividend = fix16_log(n_sum);
  
  fix16_t divisior = history[arm].n;

  fix16_t result = fix16_mul(bound, fix16_mul(2 << FRACTIONAL_BITS, fix16_sqrt(fix16_div(fix16_mul(confidence_level, dividend), divisior))));

  return result;

}

// Selects an arm based on the current history and time step
// @param history the sliding history
// @param t the current time step
// @return the selected arm
static uint32_t select_arm(history_t *history,fix16_t confidence_level,fix16_t bound){

  uint32_t max_arm = 0;        // arm with the current maximum result
  fix16_t max_result = 0;  // the current maximum result
  
  for(uint32_t arm = 0; arm < 16; ++arm){

    fix16_t mean = get_mean(history, arm);
    fix16_t exploration = get_exploration(history, arm, confidence_level,bound);
    fix16_t current_result = mean + exploration;

    
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
ducb_fixed_args_t *ducb_fixed_init(ducb_fixed_args_t *args,fix16_t discount,fix16_t confidence_level,fix16_t bound){

  history_init(args->history);
  args->t = 1;
  args->discount = discount;
  args->confidence_level = confidence_level;
  args->bound = bound;

  return args;

}

// Selects an arm based on the current time step and sliding history
// @param args stores the current time step and sliding history
// @return the selected arm
uint32_t ducb_fixed_get_arm(ducb_fixed_args_t *args){

  history_t *history = args->history;
  uint32_t t = args->t;

  fix16_t confidence_level = args->confidence_level;
  fix16_t bound = args->bound;
  
  if(t-1 < 16){ // Select each arm once at first so that each arm has a sample in the sliding history

    return t-1;

  }
  else{
    
    return select_arm(history,confidence_level,bound);

  }

}

// Appends a sample to the sliding history
// @param args stores the current time step and sliding history
// @param result the result of the last sample draw, which is either a success (1) or a failure (0)
// @param arm the arm which was selected by ducb
void ducb_fixed_append_result(ducb_fixed_args_t *args, uint32_t result, uint32_t arm){

  history_append(args->history, result, arm, args->discount);
  args->t++;

}


