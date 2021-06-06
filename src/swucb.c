#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "swucb.h"
#include <time.h>
#include "common.h"
#define CONFIDENCE_LEVEL (double)2

//#define ITERATIONS 10000
//#define WINDOW_SIZE 50
//#define TIME_INTERVAL 50
/*
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
  int selections;

} arm_t;
*/


link_t *link_init(int value, int arm){

  link_t *link = calloc(1, sizeof(link_t));
  link->value = value;
  link->arm = arm;
  link->next = NULL;

  return link;

}


linked_list_t *window_init(){

  linked_list_t *window = calloc(1, sizeof(linked_list_t));
  window->size = 0;
  for(int arm = 0; arm < 16; arm++){
    window->selections[arm] = 0;
    window->sums[arm] = 0;
  }
  window->first = NULL;
  window->last = NULL;

  return window;

}

void window_append(linked_list_t *window, int value, int window_size, int arm){

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

void window_destroy(linked_list_t *window){

  while(window->first != NULL){

    link_t *temp = window->first;
    window->first = window->first->next;
    free(temp);

  }

  free(window);

}

int *shift_window(int window[], int window_size){

  for(int i = 0; i < window_size-1; ++i){

    window[i] = window[i+1];

  }

  window[window_size-1] = -1;

  return window;

}




int min(int a, int b){

  if (a<b) {return a;}
  else {return b;}

}


double estimate(linked_list_t *window, int window_size, int arm){

  int sum = window->sums[arm];
  int selections = window->selections[arm];
  //link_t *iter = window->first;
  /*
  while(iter != NULL){

    if(iter->arm == arm){
      sum += iter->value;
      //selections++;
    }
    iter = iter->next;

  }
  */

  //return (double)sum / (double)min(window->selections, window_size);
  if(selections > 0){
    return (double)sum / (double)selections;
  }
  else{

    return 0;
  }

}


//double padding(int selections, int window_size, int t, int bound, double confidence_level){
double padding(linked_list_t *window, int window_size, int t, int bound, double confidence_level, int arm){

  int selections = window->selections[arm];
  //link_t *iter = window->first;

  /*
  while(iter != NULL){

    if(iter->arm == arm){
      selections++;
    }
    iter = iter->next;

  }
  */
  double log_res = log((double)min(window_size,t));
  double dividend = confidence_level * log_res;
  //TODO: test min instead and/or if zero
  double divisior = (double)selections;
  if(selections <= 0){
    divisior = 1;
  }
  double quotient = dividend/divisior;
  double root = sqrt(quotient);
  double result = bound * root;

//printf("log: %lf, \t dividend: %lf, \t divisior: %lf, \t quotient: %lf, \t root: %lf, \t result: %d\n", log_res,dividend,divisior,quotient,root,result);
//printf("confidence_level: %lf, \t log_res: %lf\n",confidence_level,log_res);
  return result;
  //return bound * sqrt((confidence_level*log((double)min(window_size,t)))/(double)min(window_size, selections));

}


int get_arg_max(int num_arms,
		arm_t init_arms[],
		int t,
		int window_size,
		linked_list_t *window){

  //double upper_bounds[num_arms];

  upper_bound_t max_bound = {0, 0};
  double current_bound;

  //printf("%lf\n", estimate(init_arms[13].window));
  //printf("%d\n", init_arms[13].window->size);

  for(int arm = 0; arm < num_arms; ++arm){

    //current_bound = estimate(init_arms[arm].window) + sqrt((2*log(min(window_size,t)))/init_arms[arm].window->size);

    //current_bound = estimate(init_arms[arm].window) + sqrt((2*log(min(window_size,t)))/init_arms[arm].selections);

    //current_bound = estimate(init_arms[arm].window, window_size) + padding(init_arms[arm].window->selections, window_size, t, 1, CONFIDENCE_LEVEL);
    current_bound = estimate(window, window_size, arm) + padding(window, window_size, t, 1, CONFIDENCE_LEVEL, arm);

    /*printf("Arm: %d, \t PDR: %lf, \t E: %lf, \t P: %lf, \t U: %lf, \t Selections: %d\n",
	   arm,
	   init_arms[arm].probability,
	   estimate(init_arms[arm].window, window_size),
	   padding(init_arms[arm].window->size, window_size, t, 1, CONFIDENCE_LEVEL),
	   current_bound, init_arms[arm].selections);*/

    if(current_bound > max_bound.upper_bound){

      max_bound.arm = arm;
      max_bound.upper_bound = current_bound;

    }
  }

  return max_bound.arm;

}



int swucb_draw_sample(double probability, int seeds[], int i){

  double rndDouble = (double)rand() / RAND_MAX;
  //if(random() <  probability * ((double)RAND_MAX + 1.0)){
  if(rndDouble < probability){
    return 1;

  }
  else{
    return 0;

  }

}

/*
static int *get_seeds(int iterations){

  int *seeds = calloc(iterations, sizeof(int));
  int *iter = seeds;

  for(int i = 1; i <= iterations; ++i){

    *iter = i;

    iter += sizeof(int);

  }


  return seeds;

}
*/


double swucb(double *arms,
	     int num_arms,
	     int stationary,
	     int iterations,
	     pdrs_t *rand_arms,
	     int window_size,
	     int time_interval){

  arm_t init_arms[num_arms];
  int successes = 0;
  int sample = -1;

  int seeds[iterations];

  //TODO: remove this if wrong
  linked_list_t *window = window_init();

  /*
  for(int i = 0; i < iterations; ++i){

    seeds[i] = i;

  }
  */

  //printf("%d %d\n", 0, 0);
  //printf("%d %d\n", 1, 0);

  for(int i = 0; i < num_arms; ++i){

    init_arms[i].arm = i;
    init_arms[i].probability = arms[i];
    //init_arms[i].window = window_init();
    init_arms[i].selections = 1;
    sample = swucb_draw_sample(arms[i], seeds, i);
    if(sample == 1){

      successes++;

    }
    //window_append(init_arms[i].window, sample, window_size);
    window_append(window, sample, window_size, i);

  }
  //printf("%d %d\n", 2, init_arms[0].window->selections);

  int intervals = 0;
  for(int t = num_arms+1; t <= iterations; ++t){

    //printf("t: %d\n", t);

    if(t % time_interval == 0 && stationary != 1){

      for(int i = 0; i < num_arms; ++i){

	arms[i] = (rand_arms+intervals)->pdrs[i];


      }

      ++intervals;
    }

    //TODO: Remove this
    /*
    if(t == 3000){

      arms[2] = 0.9;
      init_arms[2].probability = 0.9;

    }
    else if(t == 5000){

      arms[2] = 0.4;
      init_arms[2].probability = 0.4;
    }
    */



    int arm = get_arg_max(num_arms, init_arms, t+1, window_size, window);




    sample = swucb_draw_sample(arms[arm], seeds, t);



    if(sample == 1){

      successes++;

    }
    /*printf("Selected Arm: %d, \t Prob: %lf, \t Res: %d, \t E: %lf, \t P: %lf, \t PDR: %lf, \t Selections: %d\n",
	   arm,
	   arms[arm],
	   sample,
	   estimate(init_arms[arm].window, window_size),
	   padding(init_arms[arm].window->size,window_size,t, 1, CONFIDENCE_LEVEL),
	   (double)successes / ((double)(t+1)),
	   init_arms[arm].selections);*/
    //window_append(init_arms[arm].window, sample, window_size);
    window_append(window, sample, window_size, arm);
    //init_arms[arm].selections++;
    //printf("Selected Arm: %d, \t Prob: %lf, \t Res: %d, \t E: %lf, \t P: %lf, \t PDR: %lf\n", arm, arms[arm], sample, estimate(init_arms[arm].window, window_size), padding(init_arms[arm].window->size, window_size, t, 1, 1), (double)successes / ((double)(t+1)));
    //printf("Arm: %d, \t Prob: %lf, \t Res: %d, \t E: %lf, \t P: %lf, \t PDR: %lf\n", arm, arms[arm], sample, estimate(init_arms[arm].window), sqrt((2*log(min(window_size,t)))/init_arms[arm].window->size), (double)successes / ((double)(t+1)));
    //printf("%d %d\n", t, init_arms[0].window->selections);
  }

  //printf("Estimate 1: %lf\n", estimate(init_arms[0].window));
  //printf("%d\n", init_arms[13].window->size);

  /*
  for(int i = 0; i < num_arms; ++i){

    window_destroy(init_arms[i].window);


  }
  */
  window_destroy(window);

  //printf("Iter: %d\n", iterations);
  //printf("Float successes: %d\n", successes);


  return (double)successes / iterations;


}
