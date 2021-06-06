#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "swucbfix.h"
#include <stdint.h>
#include "sqrtfix.h"
#include "logfix.h"



uint32_t logfix(uint32_t value, uint32_t percision){

  /*if(value >> percision < 1 || value >> percision > 100){

    return 0;
    
    }*/

  FILE *file = fopen("logs1000.txt", "r");

  uint32_t logs[100];
  uint32_t num_fix;
  uint32_t i = 0;

  for(uint32_t i = 0 ; fscanf(file,"%d\n",&logs[i]) == 1  && i < 100; ++i);

  /*fscanf (file, "%d", &i);    
  while (!feof (file))
    {  
      logs[i] = num_fix;
      fscanf (file, "%d", &i);
      }*/
  /*

  while(fscanf(file, "%d", &num_fix)){

    logs[i] = num_fix;
    ++i;

    }*/
  fclose(file);

  return logs[value >> percision];
  


}



uint32_t *get_logs(uint32_t percision, uint32_t window){


  /*
  FILE *file = fopen("logs1000.txt", "r");

  uint32_t logs[window];

  for(uint32_t i = 0 ; fscanf(file,"%d\n",&logs[i]) == 1  && i < window; ++i);
  fclose(file);

  uint32_t *results = calloc(window, sizeof(uint32_t));

  for(uint32_t i = 0; i < window; ++i){

    *(results + i) = logs[i];

  }
  */
  

  
  //TODO: CHange window
  window = 10000;
  uint32_t *results = calloc(window+1, sizeof(uint32_t));

  
  for(uint32_t i = 1; i <= window; ++i){

    //printf("%d\n", (uint32_t)(log(i) * (1 << 16)));
    //*(results + i) = log(i * (1 << percision));
    *(results + i) = (uint32_t)(log(i)  * (uint32_t)(1 << percision));
  }
  
  return results;


}
