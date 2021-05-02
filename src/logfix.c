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

  uint32_t logs[1000];
  uint32_t num_fix;

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
