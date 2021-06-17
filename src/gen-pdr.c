#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>



typedef double arm_t;




int main(int argc, char *argv[]){

  srandom(time(NULL));

  double epsilon = 0.000001;

  arm_t arms[16];
  double avg = 0;
  int clock = 0;
  double diff = 0;
  for(int iter = 0; iter <= 1000; ++iter){
    do{
      avg = 0;
      for(int i = 0; i < 16; ++i){
	arms[i] = (double)rand() / RAND_MAX;
	avg += arms[i];
      }
      avg = avg / 16;
      ++clock;
      diff = avg - 0.5;
      if(diff < 0){diff = -diff;}
      //printf("%d, %lf\n", clock, avg);
    }while(diff >= epsilon);
    for(int i = 0; i < 16; ++i){

      printf("%lf\n", arms[i]);

    }
  }
  
  return 0;

}
