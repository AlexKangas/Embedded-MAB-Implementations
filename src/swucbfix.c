#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "swucbfix.h"
#include <stdint.h>
#include "sqrtfix.h"
#include "logfix.h"


#define ITERATIONS 100
#define WINDOW_SIZE 100

#define PERCISION 16
#define SHIFT_MASK ((1 << PERCISION) - 1)

#define FIX_ADD(a,b) (a + b)
#define FIX_SUB(a,b) (a - b)
#define FIX_MUL(a,b) ((a * b) >> PERCISION)
#define FIX_DIV(a,b) ((uint32_t)(((uint64_t)a << PERCISION)/b))

typedef struct {

  uint32_t value;
  struct link_t *next;
  
} link_t;

typedef struct {

  link_t *first;
  link_t *last;
  uint32_t size;
  
} linked_list_t;

typedef struct {

  uint32_t arm;
  uint32_t upper_bound;

} upper_bound_t;

typedef struct {

  uint32_t arm;
  double probability;
  linked_list_t *window;

} arm_t;

static link_t *link_init(uint32_t value){

  link_t *link = calloc(1, sizeof(link_t));
  link->value = value;
  link->next = NULL;

  return link;

}


static linked_list_t *window_init(){

  linked_list_t *window = calloc(1, sizeof(linked_list_t));
  window->size = 0;
  window->first = NULL;
  window->last = NULL;

  return window;

}

static void window_append(linked_list_t *window, uint32_t value){

  link_t *link = link_init(value);

  if(window->last == NULL){

    window->first = link;
    window->last = link;

  }
  else{

    window->last->next = link;
    window->last = link;

  }

  if(window->size < 100 << PERCISION){

    window->size++;
    
  }
  else{

    link_t *temp = window->first;
    window->first = window->first->next;
    free(temp);
    
  }

}

static void window_destroy(linked_list_t *window){

  while(window->first != NULL){

    link_t *temp = window->first;
    window->first = window->first->next;
    free(temp);

  }

  free(window);

}

static uint32_t *shift_window(uint32_t window[]){

  for(uint32_t i = 0; i < WINDOW_SIZE-1; ++i){

    window[i] = window[i+1];

  }

  //window[WINDOW_SIZE-1] = -1;

  return window;

}




static uint32_t min(uint32_t a, uint32_t b){

  if (a<b) {return a;}
  else {return b;}

}


static uint32_t estimate(linked_list_t *window){

  uint32_t sum = 0;
  link_t *iter = window->first;
  
  while(iter != NULL){

    sum += iter->value;
    iter = iter->next;

  }

  return FIX_DIV(sum, window->size);

}


static uint32_t get_arg_max(uint32_t num_arms, arm_t init_arms[], uint32_t t){

  //double upper_bounds[num_arms];

  upper_bound_t max_bound = {0, 0};
  uint32_t current_bound;

  for(uint32_t arm = 0; arm < num_arms; ++arm){

    current_bound = estimate(init_arms[arm].window) + isqrt_improved((uint32_t)FIX_DIV(FIX_MUL(2 << PERCISION, logfix(min(WINDOW_SIZE << PERCISION,t), PERCISION)), init_arms[arm].window->size));

    //printf("%d %d\n", estimate(init_arms[arm].window), isqrt_improved(FIX_DIV(FIX_MUL(2 << PERCISION, logfix(min(WINDOW_SIZE << PERCISION,t), PERCISION)), init_arms[arm].window->size)));

    //printf("%d\n", arm);
    //printf("%d\n", current_bound);
    
    if(current_bound > max_bound.upper_bound){

      max_bound.arm = arm;
      max_bound.upper_bound = current_bound;

    }
  }

  //printf("%d %d\n", max_bound.upper_bound, max_bound.arm);

  return max_bound.arm;
  
}



static uint32_t swucb_draw_sample(double probability){

  //double probability = ((double)fix / (double)(1 << PERCISION));

  if(rand() <  probability * ((double)RAND_MAX + 1.0)){

    return 1;

  }
  else{

    return 0;

  }

}


uint32_t swucbfix(double *arms, uint32_t num_arms){
  arm_t init_arms[num_arms];
  uint32_t successes = 0;
  uint32_t sample;
  
  for(uint32_t i = 0; i < num_arms; ++i){

    init_arms[i].arm = i;
    init_arms[i].probability = arms[i];
    init_arms[i].window = window_init();
    sample = swucb_draw_sample(arms[i]);
    if(sample == 1){

      successes++;

    }
    window_append(init_arms[i].window, sample);
    
  }

  for(uint32_t i = num_arms; i < ITERATIONS; ++i){

    uint32_t t = (i << PERCISION);

    //printf("%d\n", t >> PERCISION);

    uint32_t arm = get_arg_max(num_arms, init_arms, t);

    sample = swucb_draw_sample(arms[arm]);
    if(sample == 1){

      successes++;

    }
    window_append(init_arms[arm].window, sample);

  }

  for(uint32_t i = 0; i < num_arms; ++i){

    window_destroy(init_arms[i].window);
    
    
  }

  
  uint32_t iterations_fix = ITERATIONS << PERCISION;
  uint32_t pdr_fix = FIX_DIV(successes << PERCISION, iterations_fix);


  
  return pdr_fix;


}
