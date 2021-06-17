#include "swucb-float.h"


#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>


#define min(a,b) ((a) < (b) ? (a) : (b))



typedef struct link link_t;
typedef struct window window_t;

struct link{

  int value;
  int arm;
  link_t *next;

};

struct window{

  link_t *first;
  link_t *last;
  int size_current;
  int size_max;
  int selections[NUM_ARMS];
  int sums[NUM_ARMS];

};



link_t *link_init(int value, int arm){

  link_t *link = calloc(1, sizeof(link_t));
  link->value = value;
  link->arm = arm;
  link->next = NULL;

  return link;

}


window_t *window_init(int window_size){

  window_t *window = calloc(1, sizeof(window_t));
  window->size_current = 0;
  window->size_max = window_size;
  for(int arm = 0; arm < NUM_ARMS; arm++){
    window->selections[arm] = 0;
    window->sums[arm] = 0;
  }
  window->first = NULL;
  window->last = NULL;

  return window;

}

void window_append(window_t *window, int value, int arm){

  link_t *link = link_init(value, arm);

  if(window->last == NULL){

    window->first = link;
    window->last = link;

  }
  else{

    window->last->next = link;
    window->last = link;

  }

  window->selections[arm]++;
  window->sums[arm] += value;

  if(window->size_current < window->size_max){

    window->size_current++;

  }
  else{

    link_t *temp = window->first;
    window->first = window->first->next;
    window->selections[temp->arm]--;
    window->sums[temp->arm] -= temp->value;
    free(temp);

  }

}

void window_destroy(window_t *window){

  while(window->first != NULL){

    link_t *temp = window->first;
    window->first = window->first->next;
    free(temp);

  }

  free(window);

}

double get_mean(window_t *window, int arm){

  int sum = window->sums[arm];
  int selections = window->selections[arm];

  
  return (double)sum / (double)selections;
  

}


double get_upper_bound(window_t *window, int t, int arm){

  int selections = window->selections[arm];

  double log_res = log((double)min(window->size_max,t));
  double dividend = FLOAT_CONFIDENCE_LEVEL * log_res;
  //TODO: test min instead and/or if zero
  double divisor = (double)selections;
  double quotient = dividend/divisor;
  double root = sqrt(quotient);
  double result = FLOAT_BOUND * root;

  return result;

}


int select_arm(window_t *window, int t){

  int max_arm = 0;
  double max_result = 0;
  for(int arm = 0; arm < NUM_ARMS; ++arm){

    if(window->selections[arm] == 0){
      return arm;
    }

    double mean = get_mean(window, arm);

    double upper_bound = get_upper_bound(window, t, arm);
    
    double current_result = mean + upper_bound;

    
    if(current_result > max_result){

      max_arm = arm;
      max_result = current_result;

    }
  }

  return max_arm;

}


swucb_float_args_t *swucb_float_init(int window_size){

  swucb_float_args_t *args = calloc(1, sizeof(swucb_float_args_t));

  args->window = window_init(window_size);
  args->t = 1;

  return args;

}


int swucb_float_get_arm(swucb_float_args_t *args){

  window_t *window = args->window;
  int t = args->t;
  
  if(t < 17){

    return t-1;

  }
  else{

    return select_arm(window, t);

  }

}


void swucb_float_append_result(swucb_float_args_t *args, int result, int arm){

  window_append(args->window, result, arm);
  args->t++;

}


void swucb_float_destroy(swucb_float_args_t *args){

  window_destroy(args->window);
  free(args);

}






