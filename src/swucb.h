#pragma once
#include "common.h"

typedef struct link link_t;
typedef struct linked_list linked_list_t;

struct link{

  int value;
  int arm;
  link_t *next;

};

struct linked_list{

  link_t *first;
  link_t *last;
  int size;
  int selections[16];
  int sums[16];

};

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



double swucb(double *arms,
	     int num_arms,
	     int stationary,
	     int iterations,
	     pdrs_t *rand_arms,
	     int window_size,
	     int time_interval);

/*
link_t *link_init(int value);


linked_list_t *window_init();


void window_append(linked_list_t *window, int value, int window_size);

void window_destroy(linked_list_t *window);

int *shift_window(int window[], int window_size);




int min(int a, int b);


double estimate(linked_list_t *window, int window_size);

double padding(int selections, int window_size, int t, int bound, double confidence_level);

int get_arg_max(int num_arms, arm_t init_arms[], int t, int window_size);



int swucb_draw_sample(double probability, int seeds[], int i);
*/
