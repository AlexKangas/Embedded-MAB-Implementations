#include <stdio.h>
#include <stdlib.h>
#include "swucb.h"
#include <stdint.h>
#include "swucbfix.h"
#include "gen-pdr.h"
#include <time.h>
#include "common.h"
#include "logfix.h"
#include <math.h>
#include <pthread.h>
#include <sys/sysinfo.h>

//#define NUM_ARMS 16
#define PERCISION 16
//#define SHIFT_MASK ((1 << PERCISION) - 1)

double standard_deviation(double values[], int samples){

  double sum = 0;

  for(int i = 0; i < samples; ++i){

    sum += values[i];

  }

  double mean = 0.5;//sum / (double)samples;

  printf("Mean = %lf\n", mean);

  double deviation = 0;
  double sumsqr = 0;

  for(int i = 0; i < samples; ++i){

    deviation = values[i] - mean;
    sumsqr += deviation * deviation;

  }

  double variance = sumsqr / (double)(samples - 1);

  return sqrt(variance);

}

typedef struct{

  double arms[NUM_ARMS];
  pdrs_t *rand_arms;
  int window_size;
  int time_interval;

} sim_args_t;

void *simulation_float_concurrent(void *arguments){

  //printf("Yo\n");

  //srand(time(NULL));

  sim_args_t *args = (sim_args_t *)arguments;
  double pdr;

  //printf("%d, %d", args->window_size,args->time_interval);

  for(int i = 0; i < 1; ++i){

    pdr = swucb(args->arms,NUM_ARMS, 0, ITERATIONS, args->rand_arms, args->window_size, args->time_interval);
    printf("1,1,%d,%d,%lf|",args->window_size,args->time_interval,pdr);
    //sum += pdr;
  }

  //4.341688
  //11.811133
  //11.823172



}

void simulation_float(double arms[], pdrs_t *rand_arms, int window_size, int time_interval){

  double pdr; //= swucb(arms,NUM_ARMS, 1, ITERATIONS, rand_arms);
  double pdrsum = 0;

  //printf("Float PDR = %lf \n", pdr);


  //printf("Yo\n");

  //pdr = swucb(arms,NUM_ARMS, 0, ITERATIONS, rand_arms, window_size, time_interval);


  //double sum = 0;
  for(int i = 0; i < 1; ++i){
    pdr = swucb(arms,NUM_ARMS, 0, ITERATIONS, rand_arms, window_size, time_interval);
    pdrsum += pdr;
    printf("1,1,%d,%d,%lf|",window_size,time_interval,pdr);
    //sum += pdr;
  }

  //printf("1,1,%d,%d,%lf|",window_size,time_interval,pdrsum/);

  //double avg = sum / (double)run_count;


  // printf("Non-stationary Float PDR = %lf \n", pdr);

  // sim,float,window_size,time_intervals,pdr
  //printf("1,1,%d,%d,%lf|",window_size,time_interval,pdr);
  //printf("1,1,%d,%d,%lf|",window_size,time_interval,avg);

  /*

    double sum = 0;
    int samples = 10000;
    double values[samples];

    for(int i = 0; i < samples; ++i){
    values[i] = swucb(arms,NUM_ARMS, 0, ITERATIONS, rand_arms, window_size, time_interval);
    }

    double std = standard_deviation(values, samples);

    printf("Standard deviation = %lf\n", std);

  */


}

void simulation_fix(double arms[], pdrs_t *rand_arms, int window_size, int time_interval, uint32_t *logs){



  uint32_t pdrfix; //= swucbfix(arms, (uint32_t)NUM_ARMS, 1, ITERATIONS, PERCISION, rand_arms);

  //printf("Fixed PDR = %lf \n", ((double)pdrfix / ITERATIONS));

  pdrfix = swucbfix(arms, (uint32_t)NUM_ARMS, 0, ITERATIONS, PERCISION, rand_arms, window_size, time_interval, logs);

  //printf("Non-stationary Fixed PDR = %lf \n", ((double)pdrfix / ITERATIONS));
  printf("1,2,%d,%d,%lf|",window_size,time_interval,((double)pdrfix / ((double)ITERATIONS)));
  //printf("1,2,%d,%d,%d\n",window_size,time_interval,pdrfix);
}



void dataset_float(double arms[], pdrs_t *rand_arms, int window_size, int time_interval){



  double pdr = swucb(arms,NUM_ARMS, 1, ITERATIONS, rand_arms, window_size, time_interval);

  printf("Dataset Float PDR = %lf \n", pdr);

}

void dataset_fix(double arms[], pdrs_t *rand_arms, int window_size, int time_interval, uint32_t *logs){



  uint32_t pdrfix = swucbfix(arms, (uint32_t)NUM_ARMS, 1, ITERATIONS, PERCISION, rand_arms, window_size, time_interval, logs);

  printf("Dataset Fixed PDR = %lf \n", ((double)pdrfix / ITERATIONS));

}


int main(int argc, char *argv[]){

  /*
    printf("Start \n\n");
    printf("Results are printed as tuples A,B,C,D,E| where:\n");
    printf("A = 1 (simulation) or 2 (dataset)\n");
    printf("B = 1 (floating-point) or 2 (fixed-point)\n");
    printf("C = Window size\n");
    printf("D = Time interval\n");
    printf("E = PDR\n");
    printf("| marks the end of each sample.\n\n\n\n");

    srand(time(NULL));

    pdrs_t *rand_arms = calloc(ITERATIONS, sizeof(pdrs_t));
    double arms[NUM_ARMS];

    uint32_t *logs = get_logs(PERCISION, ITERATIONS);

    int time_interval = 500;

    //for(int time_interval = 1; time_interval <= ITERATIONS; ++time_interval){

    do{



    for(int window_size = 1; window_size <= ITERATIONS; ++window_size){



    for(int i = 0; i < 10; ++i){

    for(int i = 0; i < NUM_ARMS; ++i){

    arms[i] = (rand() % 10001) / 10000.0;

    }


    for(int i = 0; i < ITERATIONS/time_interval; ++i){

    for(int j = 0; j < NUM_ARMS; ++j){

    (rand_arms+i)->pdrs[j] = (rand() % 10001) / 10000.0;

    }

    }

    for(int j = 0; j < 10; ++j){

    simulation_float(arms, rand_arms, window_size, time_interval);
    simulation_fix(arms, rand_arms, window_size, time_interval, logs);

    }

    }
    }

    time_interval += 500;
    }while(time_interval < ITERATIONS);

    /*

    double *arm = gen_pdr();

    for(int i = 0; i < NUM_ARMS; ++i){

    arms[i] = *(arm + i);

    }
    free(arm);



    dataset_float(arms, rand_arms, WINDOW_SIZE, TIME_INTERVAL);
    dataset_fix(arms, rand_arms, WINDOW_SIZE, TIME_INTERVAL, logs);
  */



  srandom(time(NULL));

  pdrs_t *rand_arms = calloc(ITERATIONS, sizeof(pdrs_t));
  //double arms[NUM_ARMS];

  double arms[NUM_ARMS] = {0.386700,
			   0.426800,
			   0.023100,
			   0.685600,
			   0.354800,
			   0.772600,
			   0.055800,
			   0.860100,
			   0.340900,
			   0.311700,
			   0.710700,
			   0.040100,
			   0.995600,
			   0.995200,
			   0.961500,
			   0.078800,
  };

  uint32_t *logs = get_logs(PERCISION, ITERATIONS);

  int time_interval = 1000;

  double avg = 0;





  double sum = 0;
  for(int i = 0; i < NUM_ARMS; ++i){

    //printf("Arm: %lf\n", arms[i]);
    sum += arms[i];

  }

  //printf("Mean arm: %lf\n", sum/(double)NUM_ARMS);

  sim_args_t args;
  pthread_t threads[NUM_THREADS];
  //int window_size = 1;


  for(int i = 0; i < 1; ++i){
    
    for(int i = 0; i < ITERATIONS; ++i){


	while(!(avg == 0.5)){
	  //printf("Hello\n");
	  avg = 0;

	  for(int j = 0; j < NUM_ARMS; ++j){

	    (rand_arms+i)->pdrs[j] = (random() % 10001) / 10000.0;


	    //printf("Arm2: %lf\n", (rand_arms+i)->pdrs[j]);
	    avg += (rand_arms+i)->pdrs[j];

	  }
	  avg /= NUM_ARMS;
	}
	//printf("Avg = %lf\n", avg);

	avg = 0;

      }
      
    for(int time_interval = 100; time_interval <= 100; ++time_interval){

      /*
	while(!(avg == 0.5)){

	avg = 0;
	for(int i = 0; i < NUM_ARMS; ++i){

	arms[i] = (rand() % 10001) / 10000.0;

	avg += arms[i];

	}
	avg /= NUM_ARMS;
	}
	avg = 0;
      */

      /*
      avg = 0;
      for(int i = 0; i < ITERATIONS; ++i){

	while(!(avg = 0.5)){

	  avg = 0;

	  for(int j = 0; j < NUM_ARMS; ++j){

	    (rand_arms+i)->pdrs[j] = (random() % 10001) / 10000.0;

	    avg += (rand_arms+i)->pdrs[j];

	  }
	  avg /= NUM_ARMS;
	}
	//printf("Avg = %lf\n", avg);
	avg = 0;

      }
      */
      for(int window_size = 1; window_size <= 1000; ++window_size){

	/*
	    for(int i = 0; i < NUM_ARMS; ++i){
	    args.arms[i] = arms[i];
	    }
	    args.rand_arms = rand_arms;
	    args.window_size = window_size;
	    args.time_interval = time_interval;

	    for(int i = 0; i < NUM_THREADS; ++i){
	    pthread_create(&threads[i], NULL, simulation_float_concurrent,(void *)&args);
	    }

	    for(int i = 0; i < NUM_THREADS; ++i){
	    pthread_join(threads[i], NULL);
	    }
	*/


	simulation_float(arms, rand_arms, window_size, time_interval);
	simulation_fix(arms, rand_arms, window_size, time_interval, logs);
	}
    }
  }



  free(rand_arms);
  free(logs);
  return 0;

}








/*

Values with many really good
double arms[NUM_ARMS] = {0.078800,
			   0.386700,
			   0.426800,
			   0.023100,
			   0.685600,
			   0.354800,
			   0.772600,
			   0.055800,
			   0.860100,
			   0.340900,
			   0.311700,
			   0.710700,
			   0.040100,
			   0.995600,
			   0.995200,
			   0.961500
  };

More even
double arms[NUM_ARMS] = {0.4,
			   0.386700,
			   0.426800,
			   0.3,
			   0.685600,
			   0.3,
			   0.772600,
			   0.455800,
			   0.760100,
			   0.340900,
			   0.311700,
			   0.710700,
			   0.240100,
			   0.695600,
			   0.895200,
			   0.561500
  };


Even more even but one bad
double arms[NUM_ARMS] = {0.01,
			   0.686700,
			   0.726800,
			   0.8,
			   0.685600,
			   0.7,
			   0.772600,
			   0.655800,
			   0.760100,
			   0.840900,
			   0.811700,
			   0.710700,
			   0.900100,
			   0.795600,
			   0.955200,
			   0.761500
  };


No Bad

double arms[NUM_ARMS] = {0.81,
       0.786700,
       0.726800,
       0.8,
       0.785600,
       0.7,
       0.772600,
       0.755800,
       0.760100,
       0.840900,
       0.811700,
       0.710700,
       0.900100,
       0.795600,
       0.955200,
       0.761500
};


Paper
 double arms[NUM_ARMS] = {0.5,
  			   0.3,
  			   0.4,
    };
*/
