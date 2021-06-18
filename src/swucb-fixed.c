#include "swucb-fixed.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>

#define min(a,b) ((a) < (b) ? (a) : (b))




struct link{

  fix16_t value;
  uint32_t arm;
  link_t *next;

};

struct window{

  link_t *first;
  link_t *last;
  uint32_t size_current;
  uint32_t size_max;
  uint32_t selections[NUM_ARMS];
  uint32_t sums[NUM_ARMS];

};



static link_t *link_init(fix16_t value, uint32_t arm){

  link_t *link = calloc(1, sizeof(link_t));
  link->value = value;
  link->arm = arm;
  link->next = NULL;

  return link;

}


static window_t *window_init(uint32_t window_size){

  window_t *window = calloc(1, sizeof(window_t));
  window->size_current = 0;
  window->size_max = window_size;
  for(uint32_t arm = 0; arm < NUM_ARMS; arm++){
    window->selections[arm] = 0;
    window->sums[arm] = 0;
  }
  window->first = NULL;
  window->last = NULL;

  return window;

}

static void window_append(window_t *window, fix16_t value, uint32_t arm){

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

static void window_destroy(window_t *window){

  while(window->first != NULL){

    link_t *temp = window->first;
    window->first = window->first->next;
    free(temp);

  }

  free(window);

}


static fix16_t get_mean(window_t *window, uint32_t arm){

  fix16_t sum = window->sums[arm]  << FRACTIONAL_BITS;
  fix16_t selections = window->selections[arm]  << FRACTIONAL_BITS;

  
  return fix16_div(sum, selections);
  
}


static fix16_t get_upper_bound(window_t *window, uint32_t t, uint32_t arm, fix16_t logs[]){

  uint32_t selections = window->selections[arm];

  fix16_t log_res = logs[min(window->size_max,t)];
  fix16_t dividend = fix16_mul(CONFIDENCE_LEVEL, log_res);
  fix16_t divisor = selections << FRACTIONAL_BITS;
  fix16_t quotient = fix16_div(dividend, divisor);
  fix16_t root = fix16_sqrt(quotient);
  fix16_t result = fix16_mul(BOUND, root);

  return result;

}


static uint32_t select_arm(window_t *window, uint32_t t, fix16_t logs[]){

  uint32_t max_arm = 0;
  fix16_t max_result = 0;
  for(uint32_t arm = 0; arm < NUM_ARMS; ++arm){

    if(window->selections[arm] == 0){
      return arm;
    }

    fix16_t mean = get_mean(window, arm);

    fix16_t upper_bound = get_upper_bound(window, t, arm, logs);
    
    fix16_t current_result = mean + upper_bound;
    if(current_result > max_result){

      max_arm = arm;
      max_result = current_result;

    }
  }

  return max_arm;

}


fix16_t *get_logs(){

  FILE *file = fopen("src/log10000.txt", "r");

  uint32_t *logs = calloc(1002, sizeof(fix16_t));

  for(uint32_t i = 1 ; fscanf(file,"%" PRIu32 "\n",&logs[i]) == 1  && i <= 1000; ++i);
  fclose(file);

  return logs;

}


swucb_args_t *swucb_init(uint32_t window_size){

  swucb_args_t *args = calloc(1, sizeof(swucb_args_t));

  args->window = window_init(window_size);
  args->t = 1;
  args->logs = get_logs();

  return args;

}


uint32_t swucb_get_arm(swucb_args_t *args){

  window_t *window = args->window;
  uint32_t t = args->t;
  fix16_t *logs = args->logs;
  
  if(t < 17){

    return t-1;

  }
  else{

    uint32_t arm = select_arm(window, t, logs);

    return arm;

  }

}


void swucb_append_result(swucb_args_t *args, uint32_t result, uint32_t arm){

  window_append(args->window, result, arm);
  args->t++;

}


void swucb_destroy(swucb_args_t *args){

  window_destroy(args->window);
  free(args->logs);
  free(args);

}












