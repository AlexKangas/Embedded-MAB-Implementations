#pragma once

#define NUM_ARMS 16
#define PERCISION 16
#define SHIFT_MASK ((1 << PERCISION) - 1)
#define ITERATIONS 1000 //At least 16
#define WINDOW_SIZE 50
#define TIME_INTERVAL 10
#define NUM_THREADS 4

typedef struct {

  double pdrs[16];

} pdrs_t;
