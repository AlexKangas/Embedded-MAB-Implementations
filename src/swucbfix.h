#pragma once
#include <stdint.h>
#include "common.h"

uint32_t swucbfix(double *arms,
		  uint32_t num_arms,
		  uint32_t stationary,
		  uint32_t iterations,
		  uint32_t percision,
		  pdrs_t *rand_arms,
		  uint32_t window_size,
		  uint32_t time_interval,
		  uint32_t *logs);
