#pragma once

#define NUM_ARMS 16
#define FLOAT_CONFIDENCE_LEVEL (double)2
#define FLOAT_BOUND (double)1

typedef struct link link_t;
typedef struct window window_t;
typedef struct {

  window_t *window;
  int t;

} swucb_float_args_t;


swucb_float_args_t *swucb_float_init(int window_size);

int swucb_float_get_arm(swucb_float_args_t *args);

void swucb_float_append_result(swucb_float_args_t *args, int result, int arm);

void swucb_float_destroy(swucb_float_args_t *args);


