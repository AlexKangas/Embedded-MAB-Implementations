#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "swucbfix.h"
#include <stdint.h>
#include "sqrtfix.h"
#include "logfix.h"
#include "libfixmath/trunk/libfixmath/fix16.h"
#include <time.h>
#include "common.h"
#define CONFIDENCE_LEVEL (uint32_t)2

//#define WINDOW_SIZE 50
//#define TIME_INTERVAL 50
#define FIX_MUL(a,b,percision) ((a * b) >> percision)
#define FIX_DIV(a,b,percision) (((uint64_t)a << percision) / b)

typedef struct {

  uint32_t value;
  uint32_t arm;
  struct link_t *next;
  
} link_t;

typedef struct {

  link_t *first;
  link_t *last;
  uint32_t size;
  uint32_t selections[16];
  uint32_t sums[16];
  
} linked_list_t;

typedef struct {

  uint32_t arm;
  uint32_t upper_bound;

} upper_bound_t;

typedef struct {

  uint32_t arm;
  double probability;
  linked_list_t *window;
  uint32_t selections;

} arm_t;

static link_t *link_init(uint32_t value, uint32_t arm){

  link_t *link = calloc(1, sizeof(link_t));
  link->value = value;
  link->arm = arm;
  link->next = NULL;
  

  return link;

}


static linked_list_t *window_init(){

  linked_list_t *window = calloc(1, sizeof(linked_list_t));
  window->size = 0;
  for(uint32_t arm = 0; arm < 16; arm++){
    window->selections[arm] = 0;
    window->sums[arm] = 0;
  }
  window->first = NULL;
  window->last = NULL;

  return window;

}

static void window_append(linked_list_t *window, uint32_t value, uint32_t percision, uint32_t window_size, uint32_t arm){

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

  if(window->size < window_size){

    window->size++;
    
  }
  else{

    link_t *temp = window->first;
    window->first = window->first->next;
    window->selections[temp->arm]--;
    window->sums[temp->arm] -= temp->value;
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

static uint32_t *shift_window(uint32_t window[], uint32_t window_size){

  for(uint32_t i = 0; i < window_size-1; ++i){

    window[i] = window[i+1];

  }

  return window;

}




static uint32_t min(uint32_t a, uint32_t b){

  if (a<b) {return a;}
  else {return b;}

}


static uint32_t estimate(linked_list_t *window, uint32_t percision, uint32_t arm){

  uint32_t sum = window->sums[arm];
  uint32_t selections = window->selections[arm];
  //link_t *iter = window->first;
  /*
  while(iter != NULL){

    if(iter->arm == arm){
      sum += iter->value;
      selections++;
    }
    iter = iter->next;

  }
  */
  //printf("Sum: %d\n Window: %d\n est1: %lf\n est2: %lf\n est3: %lf\n est4: %lf\n", sum, window->size, (double)FIX_DIV(sum, window->size, percision)/(double)(1 << percision), (double)FIX_DIV(sum << percision, window->size << percision, percision)/(double)(1 << percision), (double)fix16_div(sum, window->size) / (1 << percision), (double)fix16_div(sum << percision, window->size << percision)/(1 << percision));
  /*
  printf("Arm: %d,\t Sum: %d,\t Selections: %d,\t Window: %d,\t Res: %lf\n",
	 arm,
	 sum,
	 selections,
	 window->size,
	 (double)fix16_div(sum << percision, selections << percision) / (1 << percision));
*/

  //return FIX_DIV(sum, window->size, percision);
  if(selections > 0){
    
    //return fix16_div(sum << percision, window->size << percision);
    return fix16_div(sum << percision, selections << percision);
  }
  else{
    return 0;
  }

}

double to_float(uint32_t fix){

  return (double)fix / (1 << 16);

}

static uint32_t padding(linked_list_t *window,
		 uint32_t window_size,
		 uint32_t t,
		 uint32_t bound,
		 uint32_t confidence_level,
		 uint32_t arm,
		 uint32_t *logs,
		 uint32_t percision){

  uint32_t selections = window->selections[arm];
  /*
  link_t *iter = window->first;

  while(iter != NULL){

    if(iter->arm == arm){
      selections++;
    }
    iter = iter->next;

  }
  */

  uint32_t log_res = *(logs + (min(window_size,t)));
  uint32_t dividend = fix16_mul(confidence_level, log_res);
  //TODO: test min instead and/or if zero
  uint32_t divisior = selections << percision;
  if(selections <= 0){
    divisior = 1 << percision;;
  }
  
  uint32_t quotient = fix16_div(dividend, divisior);
  uint32_t root = fix16_sqrt(quotient);
  uint32_t result = fix16_mul(bound, root);

  /*
  printf("log: %lf, \t dividend: %lf, \t divisior: %lf, \t quotient: %lf, \t root: %lf, \t result: %lf\n", to_float(log_res),
	 to_float(dividend),
	 to_float(divisior),
	 to_float(quotient),
	 to_float(root),
	 to_float(result));

  printf("window size: %d, \t t: %d, \t t_normal: %d, \t log: %d, \t log_normal: %lf\n",
	 window_size,
	 t,
	 t >> percision,
	 log_res,
	 to_float(log_res)
	 );
  */

  return result;

}

static uint32_t get_arg_max(uint32_t num_arms,
			    arm_t init_arms[],
			    uint32_t t,
			    uint32_t *logs,
			    uint32_t percision,
			    uint32_t window_size,
			    linked_list_t *window){

  upper_bound_t max_bound = {0, 0};
  uint32_t current_bound;

  for(uint32_t arm = 0; arm < num_arms; ++arm){

    //current_bound = estimate(init_arms[arm].window, percision) + fix16_sqrt((uint32_t)FIX_DIV(FIX_MUL(2 << percision, *(logs + (min(window_size << percision,t) >> percision)), percision), init_arms[arm].window->size, percision));

    
    //current_bound = estimate(init_arms[arm].window, percision) + fix16_sqrt((uint32_t)fix16_div(fix16_mul(2 << percision, *(logs + (min(window_size << percision,t) >> percision))), init_arms[arm].window->size));

    uint32_t estimated = estimate(window, percision, arm);
    uint32_t padded = padding(window,
			       window_size,
			       t,
			       1 << percision,
			       CONFIDENCE_LEVEL << percision,
			       arm,
			       logs,
			       percision);


    current_bound =  estimated + padded;

    

    /*
      printf("Arm: %d, \t PDR: %lf, \t E: %lf, \t P: %lf, \t U: %lf, \t Selections: %d\n",
      arm,
      init_arms[arm].probability,
      (double)estimated / (1 << percision),
      (double)padded / (1 << percision),
      (double)current_bound / (1 << percision),
      init_arms[arm].selections);
    */
    

    
    //current_bound = estimate(init_arms[arm].window, percision) + fix16_sqrt((uint32_t)fix16_div(fix16_mul(2 << percision, *(logs + (min(window_size << percision,t) >> percision))), init_arms[arm].selections));
    
    if(current_bound >= max_bound.upper_bound){

      max_bound.arm = arm;
      max_bound.upper_bound = current_bound;

    }
  }

  return max_bound.arm;
  
}



static uint32_t swucb_draw_sample(double probability, uint32_t seeds[], uint32_t i){

  double rndDouble = (double)rand() / RAND_MAX;

  //if(random() <  probability * ((double)RAND_MAX + 1.0)){
  if(rndDouble < probability){

    return 1;

  }
  else{

    return 0;

  }

}


uint32_t swucbfix(double *arms,
		  uint32_t num_arms,
		  uint32_t stationary,
		  uint32_t iterations,
		  uint32_t percision,
		  pdrs_t *rand_arms,
		  uint32_t window_size,
		  uint32_t time_interval,
		  uint32_t *logs){
  
  arm_t init_arms[num_arms];
  uint32_t successes = 0;
  uint32_t sample;

  //uint32_t *logs = get_logs(percision, window_size);

  uint32_t seeds[iterations];

  linked_list_t *window = window_init();

  /*
  for(uint32_t i = 0; i < iterations; ++i){

    seeds[i] = i;

  }
  */
  
  for(uint32_t i = 0; i < num_arms; ++i){

    init_arms[i].arm = i;
    init_arms[i].probability = arms[i];
    //init_arms[i].window = window_init();
    init_arms[i].selections = 1;
    sample = swucb_draw_sample(arms[i], seeds, i);
    if(sample == 1){

      successes++;

    }
    //window_append(init_arms[i].window, sample, percision, window_size);
    window_append(window, sample, percision, window_size, i);
    
  }

  uint32_t intervals = 0;
  for(uint32_t i = num_arms; i < iterations; ++i){

    if(i % time_interval == 0 && stationary != 1){

      for(uint32_t j = 0; j < num_arms; ++j){

        //arms[j] = (rand() % 10001) / 10000.0;
        //arms[i] = rand_arms[intervals][j];
	arms[j] = (rand_arms+intervals)->pdrs[j];
	init_arms[j].probability = arms[j];
	//rand_arms = rand_arms + sizeof(double);
	
      }

      ++intervals;

    }

    //uint32_t t = (i << percision);
    uint32_t t = i+1;

    uint32_t arm = get_arg_max(num_arms, init_arms, t, logs, percision, window_size, window);

    sample = swucb_draw_sample(arms[arm], seeds, i);

    
    if(sample == 1){
      successes++;

    }

    /*
    printf("Selected arm: %d, \t Prob: %lf, \t Res: %d, \t E: %lf, \t P: %lf, \t PDR: %lf, \t Selections: %d\n",
	   arm,
	   arms[arm],
	   sample,
	   (double)(estimate(window, percision, arm) / (double)(1 << percision)),
	   (double)(padding(window,window_size,t,1 << percision,CONFIDENCE_LEVEL << percision,arm,logs,percision) / (double)(1 << percision)),
	   (double)successes / (double)t,
	   init_arms[arm].selections);
    */
    
    
    //window_append(init_arms[arm].window, sample, percision, window_size);
    window_append(window, sample, percision, window_size, arm);
    init_arms[arm].selections++;
    //printf("Window: %d\n", window_size);
    //printf("Arm: %d, \t Prob: %lf, \t Res: %d, \t E: %lf, \t P: %lf, \t PDR: %lf\n", arm, arms[arm], sample, estimate(init_arms[arm].window, percision)/((double)(1 << percision)),(double)fix16_sqrt((uint32_t)FIX_DIV(FIX_MUL(2 << percision, *(logs + (min(window_size << percision,t) >> percision)), percision), init_arms[arm].window->size, percision))/((double)(1 << percision)),(double)successes / ((double)(i+1)));


    //printf("Arm: %d, \t Prob: %lf, \t Res: %d, \t E: %lf, \t P: %lf, \t PDR: %lf\n", arm, arms[arm], sample, estimate(init_arms[arm].window, percision)/((double)(1 << percision)), (double)fix16_sqrt((uint32_t)fix16_div(fix16_mul(2 << percision, *(logs + (min(window_size << percision,t) >> percision))), init_arms[arm].window->size)) / (1 << percision), (double)successes / ((double)(i+1)));
    

  }

  //printf("Estimate 2: %lf\n", estimate(init_arms[0].window, percision)/((double)(1 << percision)));
  /*
  for(uint32_t i = 0; i < num_arms; ++i){

    window_destroy(init_arms[i].window);
    
    
  }
  */
  //free(logs);
  window_destroy(window);
  

  //printf("Fix successes: %d\n", successes);
  return successes;


}
