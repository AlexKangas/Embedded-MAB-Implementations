#pragma once

#include "fixed-point.h"


#define NUM_ARMS 16
#define CONFIDENCE_LEVEL 2 << FRACTIONAL_BITS
#define BOUND 1 << FRACTIONAL_BITS

typedef struct link link_t;
typedef struct window window_t;

typedef struct {

  window_t *window;
  uint32_t t;
  fix16_t *logs;

} swucb_args_t;



swucb_args_t *swucb_init(uint32_t window_size);

uint32_t swucb_get_arm(swucb_args_t *args);

void swucb_append_result(swucb_args_t *args, uint32_t result, uint32_t arm);

void swucb_destroy(swucb_args_t *args);

fix16_t *get_logs();
