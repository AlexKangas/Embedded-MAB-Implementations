#include <string.h>
#include <CUnit/Basic.h>
#include "swucb.h"
#include <stdlib.h>


//Här skriver man sina tester
void test_window_append()
{
  linked_list_t *window = window_init();
  CU_ASSERT_PTR_NULL(window->first);
  CU_ASSERT_PTR_NULL(window->last);
  CU_ASSERT_EQUAL(window->size, 0);
  window_destroy(window);

  for(int window_size = 1; window_size <= 1000; window_size++){

    window = window_init();

    for(int t = 1; t <= 2000; ++t){

      window_append(window, 1, window_size);

      if(t <= window_size){

	CU_ASSERT_EQUAL(window->size, t);

      }
      else{

	CU_ASSERT_EQUAL(window->size, window_size);

      }

    }
    window_destroy(window);

  }
}

void test_min()
{

  for (int i = 1; i <= 1000; ++i){

    for(int j = 1; j <= 1000; ++j){

      if(i < j){
	CU_ASSERT_EQUAL(min(i, j), i);
      }
      else if(j < i){
	CU_ASSERT_EQUAL(min(i, j), j);
      }
      else{
	CU_ASSERT_EQUAL(min(i, j), i);
	CU_ASSERT_EQUAL(min(i, j), j);
      }
    }

  }
}

void test_estimate()
{

  linked_list_t *window;
  double granularity = 0.00005;
  int value;
  int stored[1000];
  int index_first = 0;
  int sum = 0;

  for(int window_size = 1; window_size <= 100; ++window_size){

    window = window_init();
    value = 1;
    for(int i = 1; i <= 100; i++){



      window_append(window, value, window_size);

      if(i < window_size){
        CU_ASSERT_EQUAL(window->size, i);
	      CU_ASSERT_DOUBLE_EQUAL(estimate(window), 1, granularity);
      }
      else{
        CU_ASSERT_EQUAL(window->size, window_size);
	      CU_ASSERT_DOUBLE_EQUAL(estimate(window), 1, granularity);
      }

    }

    window_destroy(window);

    window = window_init();
    value = 0;

    for(int i = 1; i <= 100; i++){

      window_append(window, value, window_size);

      CU_ASSERT_DOUBLE_EQUAL(estimate(window), 0, granularity);

    }
    window_destroy(window);

    window = window_init();

    for(int i = 1; i <= 100; ++i){

      value = rand() % 2;

      stored[i-1] = value;


      window_append(window, value, window_size);

      sum += value;

      if(i <= window_size){

	CU_ASSERT_DOUBLE_EQUAL(estimate(window), (double)sum / i, granularity);
      }
      else{
	sum -= stored[index_first];
	index_first++;

	CU_ASSERT_DOUBLE_EQUAL(estimate(window), (double)sum / window_size, granularity);
      }

    }
    index_first = 0;
    sum = 0;

  }



}


void test_padding()
{
  double granularity = 0.00005;
  int value;
  int stored[1000];
  int index_first = 0;
  int sum = 0;
  double dividend;
  double divider;
  double quotient;
  double real;


  for(int window_size = 1; window_size <= 100; ++window_size){

    for(int selections = 1; selections <= 100; ++selections){

      for(int t = 1; t <= 100; ++t){

	if(t < window_size){
	  dividend = 2*log(t);
	}
	else{
	  dividend = 2*log(window_size);
	}

	if(selections < window_size){
	  divider = selections;
	}
	else{
	  divider = window_size;
	}

	quotient = dividend / divider;

	real = padding(selections, window_size, t, 1, 2);

	CU_ASSERT_DOUBLE_EQUAL(real, sqrt(quotient), granularity);

      }

    }

  }

}

//Kan initiera en testmiljö/data mha denna
int init_suite(void)
{
  return 0;
}

//Städar bort testmiljön/data mha denna
int clean_suite(void)
{
return 0;
}


int main()
{
  //Första raderna är bara CUnit-magi
  CU_pSuite test_suite1 = NULL;

  if (CUE_SUCCESS != CU_initialize_registry())
    return CU_get_error();


  //Skapar en test_suite
  test_suite1 = CU_add_suite("Test Suite 1", NULL, NULL); //Kan skriva NULL NULL eller init_suite clean_suite
  if (NULL == test_suite1)
    {
      CU_cleanup_registry();
      return CU_get_error();
    }

  //Lägg in tester i de skapade test_suite(s). Varje inlagt test separeras av ||
  if (
      (NULL == CU_add_test(test_suite1, "test_window_append", test_window_append)) ||
      (NULL == CU_add_test(test_suite1, "test_min", test_min)) ||
      (NULL == CU_add_test(test_suite1, "test_estimate", test_estimate)) ||
      (NULL == CU_add_test(test_suite1, "test_padding", test_padding)) //||
      //(NULL == CU_add_test(test_suite1, "test_get_arg_max", get_arg_max)) ||
      //(NULL == CU_add_test(test_suite1, "test_swucb_draw_sample", test_swucb_draw_sample))
  )
    {
      CU_cleanup_registry();
      return CU_get_error();
    }

  //Slutkonfig av CUnit. Behöver ej ändras ifall man inte har specifika behov
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  return CU_get_error();
}
