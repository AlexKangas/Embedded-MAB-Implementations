#include "ducb-float.h"


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
  int value;
  int t;
  link_t *next; // Next entry in the history
  */

  double n;
  double rewards;
  

};

// Datatype for a sliding history, which is basically a linked list
struct history{

  link_t arms[NUM_ARMS];

};

// Creates an entry for the sliding history
// @param value the value of the sample, which is success (1) or failure (0)
// @param arm the arm that the sample belongs to
// @return the allocated entry
static link_t *link_init(int value, int t){

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
//static void history_append(history_t *history, int value, int arm, int t){
static void history_append(history_t *history, int value, int arm, double discount){
  /*
  link_t *link = &history->arms[arm];

  while(link->next != NULL){

    link = link->next;

  }

  link->next = link_init(value, t);
  */

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

  /*
  for(int arm = 0; arm < 16; ++arm){
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

static double power(double base, int exp){

  if(exp == 0){

    return 1.0;
    
  }

  double result = base;

  for(int i = 1; i < exp; ++i){

    result = result * base;
    
  }

  return result;

}
/*
static double sum_discounts(history_t *history, int t, int arm, double discount){

  double sum = 0;
  
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

//static double get_mean(history_t *history, int arm, int t, double discount){
static double get_mean(history_t *history, int arm){

  /*
  double sum = 0;
  double divisior = 0;
  link_t *link = &history->arms[arm];

  

  while(link != NULL){

    if(link->t > 0){
      sum = sum + (link->value * pow(discount,t - link->t));
      //sum = sum + (link->value * power(discount,t - link->t));
      divisior = divisior + pow(discount,t - link->t);
      //divisior = divisior + power(discount,t - link->t);
    }
    link = link->next;
    
    if(link != NULL){
      //printf("Sum: %lf, div: %lf, t: %d, arm: %d, discount: %lf,link->t: %d, link->value: %d\n", sum,divisior,t,arm,discount,link->t,link->value);
    }
    else{
      //printf("Sum: %lf, div: %lf, t: %d, arm: %d, discount: %lf\n", sum,divisior,t,arm,discount);
    }
    
  }

  //printf("Sum: %lf, div: %lf, t: %d, arm: %d, discount: %lf\n", sum,divisior,t,arm,discount);
  if(divisior <=0){
    return 0;
    }
  return sum/divisior;
  */

  return history->arms[arm].rewards/history->arms[arm].n;

}



// Gets the upper bound (or "padding function") of an arm
// @param history the sliding history
// @param t the current time step
// @param arm the arm to calculate the upper bound from
//static double get_exploration(history_t *history, int t, int arm, double discount){
//static double get_exploration(history_t *history, int arm){
static double get_exploration(history_t *history, int arm,double confidence_level,double bound){

  /*
  double discount_sum = 0;
  for(int a = 0; a < 16; ++a){
    discount_sum += sum_discounts(history, t, a, discount);
  }
  
  double dividend = log(discount_sum);
  
  double divisior = sum_discounts(history, t, arm, discount);

  if(divisior <=0){
    return 0;
    }
  
  double result = 2*sqrt(dividend/divisior);
  */

  double n_sum = 0;
  for(int a = 0; a < NUM_ARMS; ++a){
    n_sum += history->arms[a].n;
  }

  double dividend = log(n_sum);
  
  double divisior = history->arms[arm].n;

  //double result = 2*FLOAT_BOUND*sqrt(FLOAT_CONFIDENCE_LEVEL*dividend/divisior);
  double result = 2*bound*sqrt(confidence_level*dividend/divisior);

  return result;

}

// Selects an arm based on the current sliding history and time step
// @param history the sliding history
// @param t the current time step
// @return the selected arm
//static int select_arm(history_t *history, int t, double discount){
//static int select_arm(history_t *history){
static int select_arm(history_t *history,double confidence_level,double bound){

  int max_arm = 0;        // arm with the current maximum result
  double max_result = 0;  // the current maximum result
  
  for(int arm = 0; arm < NUM_ARMS; ++arm){

    /*
    double mean = get_mean(history, arm, t, discount);
    double exploration = get_exploration(history, t, arm, discount);
    double current_result = mean + exploration;
    */

    double mean = get_mean(history, arm);
    //double exploration = get_exploration(history, arm);
    double exploration = get_exploration(history, arm, confidence_level, bound);
    double current_result = mean + exploration;

    //if(arm == 2){
    //printf("Arm: %d,  mean: %lf, explo: %lf, res: %lf\n", arm,mean,exploration,current_result);
    //}
    
    if(current_result > max_result){

      max_arm = arm;
      max_result = current_result;

    }
  }
  //printf("Max_Arm: %d, Max_res: %lf\n", max_arm,max_result);
  //printf("max_arm: %d\n",max_arm);
  return max_arm;

}

// Initializes the algorithm with needed parameters
// @param history_size the max fixed sliding history size of ducb
// @return the parameters needed to run ducb
//ducb_float_args_t *ducb_float_init(double discount){
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
  //double discount = args->discount;
  double confidence_level = args->confidence_level;
  double bound = args->bound;
  
  if(t-1 < NUM_ARMS){ // Select each arm once at first so that each arm has a sample in the sliding history

    return t-1;

  }
  else{

    //return select_arm(history, t, discount);
    return select_arm(history, confidence_level,bound);

  }

}

// Appends a sample to the sliding history
// @param args stores the current time step and sliding history
// @param result the result of the last sample draw, which is either a success (1) or a failure (0)
// @param arm the arm which was selected by ducb
void ducb_float_append_result(ducb_float_args_t *args, int result, int arm){

  //history_append(args->history, result, arm, args->t);
  history_append(args->history, result, arm, args->discount);
  args->t++;

}

// Deallocates the sliding history
// @param args stores the current time step and sliding history
void ducb_float_destroy(ducb_float_args_t *args){

  history_destroy(args->history);
  free(args);

}

