#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "swucb.h"

#define ITERATIONS 1000000
#define WINDOW_SIZE 100

typedef struct {

  int value;
  struct link_t *next;
  
} link_t;

typedef struct {

  link_t *first;
  link_t *last;
  int size;
  
} linked_list_t;

typedef struct {

  int arm;
  double upper_bound;

} upper_bound_t;

typedef struct {

  int arm;
  double probability;
  linked_list_t *window;

} arm_t;

static link_t *link_init(int value){

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

static void window_append(linked_list_t *window, int value){

  link_t *link = link_init(value);

  if(window->last == NULL){

    window->first = link;
    window->last = link;

  }
  else{

    window->last->next = link;
    window->last = link;

  }

  if(window->size < 100){

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

static int *shift_window(int window[]){

  for(int i = 0; i < WINDOW_SIZE-1; ++i){

    window[i] = window[i+1];

  }

  window[WINDOW_SIZE-1] = -1;

  return window;

}




static int min(int a, int b){

  if (a<b) {return a;}
  else {return b;}

}


static double estimate(linked_list_t *window){

  int sum = 0;
  link_t *iter = window->first;
  
  while(iter != NULL){

    sum += iter->value;
    iter = iter->next;

  }

  return (double)sum / (double)window->size;

}


static int get_arg_max(int num_arms, arm_t init_arms[], int t){

  //double upper_bounds[num_arms];

  upper_bound_t max_bound = {-1, -1};
  double current_bound;

  for(int arm = 0; arm < num_arms; ++arm){

    current_bound = estimate(init_arms[arm].window) + sqrt((2*log(min(WINDOW_SIZE,t)))/init_arms[arm].window->size);

    if(current_bound > max_bound.upper_bound){

      max_bound.arm = arm;
      max_bound.upper_bound = current_bound;

    }
  }

  return max_bound.arm;
  
}



static int swucb_draw_sample(double probability){

  if(rand() <  probability * ((double)RAND_MAX + 1.0)){

    return 1;

  }
  else{

    return 0;

  }

}


double swucb(double *arms, int num_arms){
  arm_t init_arms[num_arms];
  int successes = 0;
  int sample = -1;
  
  for(int i = 0; i < num_arms; ++i){

    init_arms[i].arm = i;
    init_arms[i].probability = arms[i];
    init_arms[i].window = window_init();
    sample = swucb_draw_sample(arms[i]);
    if(sample == 1){

      successes++;

    }
    window_append(init_arms[i].window, sample);
    
  }

  for(int t = num_arms; t < ITERATIONS; ++t){

    int arm = get_arg_max(num_arms, init_arms, t);

    sample = swucb_draw_sample(arms[arm]);
    if(sample == 1){

      successes++;

    }
    window_append(init_arms[arm].window, sample);

  }

  for(int i = 0; i < num_arms; ++i){

    window_destroy(init_arms[i].window);
    
    
  }

  return (double)successes / ITERATIONS;


}
