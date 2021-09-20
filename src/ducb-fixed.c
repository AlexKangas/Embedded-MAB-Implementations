#include "ducb-fixed.h"


#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

//Gives the minimum value of a and b
//#define min(a,b) ((a) < (b) ? (a) : (b))



typedef struct link link_t;
typedef struct history history_t;

//Datatype for an entry in the sliding history
struct link{

  /*
  uint32_t value;
  uint32_t t;
  link_t *next; // Next entry in the history
  */

  fix16_t n;
  fix16_t rewards;
  

};

// Datatype for a sliding history, which is basically a linked list
struct history{

  link_t arms[16];

};

// Creates an entry for the sliding history
// @param value the value of the sample, which is success (1) or failure (0)
// @param arm the arm that the sample belongs to
// @return the allocated entry
static link_t *link_init(uint32_t value, uint32_t t){

  /*
  link_t *link = calloc(1, sizeof(link_t));
  link->value = value;
  link->t = t;
  link->next = NULL;

  return link;
  */

  return NULL;

}

// Creates a sliding history
// @param history_size the maximum possible size of the sliding history
// @return an allocated sliding history
static history_t *history_init(){

  history_t *history = calloc(1, sizeof(history_t));

  for(uint32_t i = 0; i < 16; ++i){

    history->arms[i].n = 0;
    history->arms[i].rewards = 0;

  }

  return history;

}

// Appends a sample uint32_to the sliding history
// @param history the sliding history
// @param value the sample value to be appended
// @param arm the arm that the sample belongs to
//static void history_append(history_t *history, uint32_t value, uint32_t arm, uint32_t t){
static void history_append(history_t *history, uint32_t value, uint32_t arm, fix16_t discount){
  /*
  link_t *link = &history->arms[arm];

  while(link->next != NULL){

    link = link->next;

  }

  link->next = link_init(value, t);
  */

  for(uint32_t i = 0; i < 16; ++i){

    history->arms[i].n = fix16_mul(history->arms[i].n, discount);
    history->arms[i].rewards = fix16_mul(history->arms[i].rewards, discount);

  }

  history->arms[arm].n = history->arms[arm].n + (1 << FRACTIONAL_BITS);
  history->arms[arm].rewards = history->arms[arm].rewards + (value << FRACTIONAL_BITS);

}

// Deallocates the sliding history
// @param history the sliding history to be deallocated
static void history_destroy(history_t *history){

  /*
  for(uint32_t arm = 0; arm < 16; ++arm){
    link_t *link = &history->arms[arm];
    link = link->next;
    
    while(link != NULL){

      link_t *temp = link->next;
      free(link);
      link = temp;
      
    }
  }
  */

  free(history);

}

/*
static fix16_t sum_discounts(history_t *history, uint32_t t, uint32_t arm, fix16_t discount){

  fix16_t sum = 0;
  
  link_t *link = &history->arms[arm];

  while(link != NULL){

    if(link->t > 0){
      sum += pow(discount,t - link->t);
      //sum += power(discount,t - link->t);
    }
    link = link->next;
  }
  
  return sum;

}
*/

//static fix16_t get_mean(history_t *history, uint32_t arm, uint32_t t, fix16_t discount){
static fix16_t get_mean(history_t *history, uint32_t arm){


  return fix16_div(history->arms[arm].rewards, history->arms[arm].n);

}



// Gets the upper bound (or "padding function") of an arm
// @param history the sliding history
// @param t the current time step
// @param arm the arm to calculate the upper bound from
//static fix16_t get_exploration(history_t *history, uint32_t t, uint32_t arm, fix16_t discount){
//static fix16_t get_exploration(history_t *history, uint32_t arm){
static fix16_t get_exploration(history_t *history, uint32_t arm,fix16_t confidence_level,fix16_t bound){


  fix16_t n_sum = 0;
  for(uint32_t a = 0; a < 16; ++a){
    n_sum += history->arms[a].n;
  }

  fix16_t dividend = fix16_log(n_sum);
  
  fix16_t divisior = history->arms[arm].n;

  //fix16_t result = fix16_mul(BOUND, fix16_mul(2 << FRACTIONAL_BITS, fix16_sqrt(fix16_div(fix16_mul(CONFIDENCE_LEVEL, dividend), divisior))));
  fix16_t result = fix16_mul(bound, fix16_mul(2 << FRACTIONAL_BITS, fix16_sqrt(fix16_div(fix16_mul(confidence_level, dividend), divisior))));

  return result;

}

// Selects an arm based on the current sliding history and time step
// @param history the sliding history
// @param t the current time step
// @return the selected arm
//static uint32_t select_arm(history_t *history, uint32_t t, fix16_t discount){
//static uint32_t select_arm(history_t *history){
static uint32_t select_arm(history_t *history,fix16_t confidence_level,fix16_t bound){

  uint32_t max_arm = 0;        // arm with the current maximum result
  fix16_t max_result = 0;  // the current maximum result
  
  for(uint32_t arm = 0; arm < 16; ++arm){

    /*
    fix16_t mean = get_mean(history, arm, t, discount);
    fix16_t exploration = get_exploration(history, t, arm, discount);
    fix16_t current_result = mean + exploration;
    */

    fix16_t mean = get_mean(history, arm);
    fix16_t exploration = get_exploration(history, arm, confidence_level,bound);
    fix16_t current_result = mean + exploration;

    //printf("Mean: %d, exp: %d, curr: %d\n", mean, exploration, current_result);

    //pruint32_tf("Arm: %d, discount: %lf, t: %d, mean: %lf, explo: %lf, res: %lf\n", arm,discount,t,mean,exploration,current_result);

    
    if(current_result > max_result){

      max_arm = arm;
      max_result = current_result;

    }
  }
  //pruint32_tf("max_arm: %d\n",max_arm);
  return max_arm;

}

// Initializes the algorithm with needed parameters
// @param history_size the max fixed sliding history size of ducb
// @return the parameters needed to run ducb
//ducb_fixed_args_t *ducb_fixed_init(fix16_t discount){
ducb_fixed_args_t *ducb_fixed_init(fix16_t discount,fix16_t confidence_level,fix16_t bound){

  ducb_fixed_args_t *args = calloc(1, sizeof(ducb_fixed_args_t));

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
uint32_t ducb_fixed_get_arm(ducb_fixed_args_t *args){

  history_t *history = args->history;
  uint32_t t = args->t;
  //fix16_t discount = args->discount;

  fix16_t confidence_level = args->confidence_level;
  fix16_t bound = args->bound;
  
  //if(t < 16){ // Select each arm once at first so that each arm has a sample in the sliding history
  if(t-1 < 16){ // Select each arm once at first so that each arm has a sample in the sliding history

    return t-1;

  }
  else{

    //return select_arm(history, t, discount);
    //return select_arm(history);
    return select_arm(history,confidence_level,bound);

  }

}

// Appends a sample to the sliding history
// @param args stores the current time step and sliding history
// @param result the result of the last sample draw, which is either a success (1) or a failure (0)
// @param arm the arm which was selected by ducb
void ducb_fixed_append_result(ducb_fixed_args_t *args, uint32_t result, uint32_t arm){

  //history_append(args->history, result, arm, args->t);
  history_append(args->history, result, arm, args->discount);
  args->t++;

}

// Deallocates the sliding history
// @param args stores the current time step and sliding history
void ducb_fixed_destroy(ducb_fixed_args_t *args){

  history_destroy(args->history);
  free(args);

}

