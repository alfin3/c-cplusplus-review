/**
   mergesort-pthread-test.c

   Optimization and correctness tests of a generic merge sort algorithm with
   parallel sorting and parallel merging.

   The following command line arguments can be used to customize tests:
   mergesort-pthread-test
      [0, # bits in size_t - 1) : a
      [0, # bits in size_t - 1) : b s.t. 2^a <= count <= 2^b
      [0, # bits in size_t) : c
      [0, # bits in size_t) : d s.t. 2^c <= sort base case bound <= 2^d
      [1, # bits in size_t) : e
      [1, # bits in size_t) : f s.t. 2^e <= merge base case bound <= 2^f
      [0, 1] : int corner test on/off
      [0, 1] : int performance test on/off
      [0, 1] : double corner test on/off
      [0, 1] : double performance test on/off

   usage examples: 
   ./mergesort-pthread-test
   ./mergesort-pthread-test 17 17
   ./mergesort-pthread-test 20 20 15 20 15 20
   ./mergesort-pthread-test 20 20 15 20 15 20 0 1 0 1

   mergesort-pthread-test can be run with any subset of command line
   arguments in the above-defined order. If the (i + 1)th argument is
   specified then the ith argument must be specified for i >= 0. Default
   values are used for the unspecified arguments according to the
   C_ARGS_DEF array.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 with the requirements that CHAR_BIT * sizeof(size_t) is even and
   pthreads API is available.
*/

#define _POSIX_C_SOURCE 200112L

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include "mergesort-pthread.h"
#include "utilities-mem.h"
#include "utilities-mod.h"

/**
   Generate random numbers in a portable way for test purposes only; rand()
   in the Linux C Library uses the same generator as random(), which may not
   be the case on older rand() implementations, and on current
   implementations on different systems.
*/
#define RGENS_SEED() do{srand(time(NULL));}while (0)
#define RANDOM() (rand()) /* [0, RAND_MAX] */
#define DRAND() ((double)rand() / RAND_MAX) /* [0.0, 1.0] */

#define TOLU(i) ((unsigned long int)(i)) /* printing size_t under C89/C90 */

/* input handling */
const char *C_USAGE =
  "mergesort-pthread-test \n"
  "[0, # bits in size_t - 1) : a \n"
  "[0, # bits in size_t - 1) : b s.t. 2^a <= count <= 2^b \n"
  "[0, # bits in size_t) : c \n"
  "[0, # bits in size_t) : d s.t. 2^c <= sort base case bound <= 2^d \n"
  "[1, # bits in size_t) : e \n"
  "[1, # bits in size_t) : f s.t. 2^e <= merge base case bound <= 2^f \n"
  "[0, 1] : int corner test on/off \n"
  "[0, 1] : int performance test on/off \n"
  "[0, 1] : double corner test on/off \n"
  "[0, 1] : double performance test on/off \n";
const int C_ARGC_MAX = 11;
const size_t C_ARGS_DEF[10] = {15, 15, 10, 15, 10, 15, 1, 1, 1, 1};
const size_t C_FULL_BIT = CHAR_BIT * sizeof(size_t);

/* corner cases */
const size_t C_CORNER_TRIALS = 10;
const size_t C_CORNER_COUNT_MAX = 17;
const size_t C_CORNER_SBASE_START = 1;
const size_t C_CORNER_SBASE_END = 17;
const size_t C_CORNER_MBASE_START = 2;
const size_t C_CORNER_MBASE_END = 20;
const double C_HALF_PROB = 0.5;

/* performance tests */
const size_t C_TRIALS = 5;

double timer();
void print_uint_elts(const size_t *a, size_t count);
void print_test_result(int res);

int cmp_int(const void *a, const void *b){
  if (*(int *)a > *(int *)b){
    return 1;
  }else if  (*(int *)a < *(int *)b){
    return -1;
  }else{
    return 0;
  }
}

int cmp_double(const void *a, const void *b){
  if (*(double *)a > *(double *)b){
    return 1;
  }else if  (*(double *)a < *(double *)b){
    return -1;
  }else{
    return 0;
  }
}

/**
   Runs a mergesort_pthread corner cases test on random integer arrays.
*/
void run_int_corner_test(){
  int res = 1;
  int *arr_a = NULL, *arr_b = NULL;
  size_t count, sb, mb;
  size_t i, j;
  size_t elt_size =  sizeof(int);
  arr_a =  malloc_perror(C_CORNER_COUNT_MAX, elt_size);
  arr_b =  malloc_perror(C_CORNER_COUNT_MAX, elt_size);
  printf("Test mergesort_pthread on corner cases on random "
	 "integer arrays\n");
  for (count = 1; count <= C_CORNER_COUNT_MAX; count++){
    for (sb = C_CORNER_SBASE_START; sb <= C_CORNER_SBASE_END; sb++){
      for (mb = C_CORNER_MBASE_START; mb <= C_CORNER_MBASE_END; mb++){
	for(i = 0; i < C_CORNER_TRIALS; i++){
	  for (j = 0; j < count; j++){
	    arr_a[j] = (DRAND() < C_HALF_PROB ? -1 : 1) * RANDOM();
	  }
	  memcpy(arr_b, arr_a, count * elt_size);
	  mergesort_pthread(arr_a, count, elt_size, sb, mb, cmp_int);
	  qsort(arr_b, count, elt_size, cmp_int);
	  for (j = 0; j < count; j++){
	    res *= (arr_a[j] == arr_b[j]);
	  }
	}
      }
    }
  }
  printf("\tcorrectness:       ");
  print_test_result(res);
  free(arr_a);
  free(arr_b);
  arr_a = NULL;
  arr_b = NULL;
}

/**
   Runs a test comparing mergesort_pthread vs. qsort performance on random 
   integer arrays across sort and merge base count bounds.
*/
void run_int_opt_test(int pow_count_start,
		      int pow_count_end,
		      int pow_sbase_start,
		      int pow_sbase_end,
		      int pow_mbase_start,
		      int pow_mbase_end){
  int res = 1;
  int *arr_a = NULL, *arr_b = NULL;
  int ci, si, mi;
  size_t count, sbase, mbase;
  size_t i, j;
  size_t elt_size = sizeof(int);
  double tot_m, tot_q, t_m, t_q;
  arr_a =  malloc_perror(pow_two(pow_count_end), elt_size);
  arr_b =  malloc_perror(pow_two(pow_count_end), elt_size);
  printf("Test mergesort_pthread performance on random integer arrays\n");
  for (ci = pow_count_start; ci <= pow_count_end; ci++){
    count = pow_two(ci); /* > 0 */
    printf("\t# trials: %lu, array count: %lu\n",
	   TOLU(C_TRIALS), TOLU(count));
    for (si = pow_sbase_start; si <= pow_sbase_end; si++){
      sbase = pow_two(si);
      printf("\t\tsort base count: %lu\n", TOLU(sbase));
      for (mi = pow_mbase_start; mi <= pow_mbase_end; mi++){
	mbase = pow_two(mi);
	printf("\t\t\tmerge base count: %lu\n", TOLU(mbase));
	tot_m = 0.0;
	tot_q = 0.0;
	for(i = 0; i < C_TRIALS; i++){
	  for (j = 0; j < count; j++){
	    arr_a[j] = (DRAND() < C_HALF_PROB ? -1 : 1) * RANDOM();
	  }
	  memcpy(arr_b, arr_a, count * elt_size);
	  t_m = timer();
	  mergesort_pthread(arr_a, count, elt_size, sbase, mbase, cmp_int);
	  t_m = timer() - t_m;
	  t_q = timer();
	  qsort(arr_b, count, elt_size, cmp_int);
	  t_q = timer() - t_q;
	  tot_m += t_m;
	  tot_q += t_q;
	  for (j = 0; j < count; j++){
	    res *= (arr_a[j] == arr_b[j]);
	  }
	}
	printf("\t\t\tave pthread mergesort: %.6f seconds\n",
	       tot_m / C_TRIALS);
	printf("\t\t\tave qsort:             %.6f seconds\n",
	       tot_q / C_TRIALS);
	printf("\t\t\tcorrectness:           ");
	print_test_result(res);
      }
    }
  }
  free(arr_a);
  free(arr_b);
  arr_a = NULL;
  arr_b = NULL;
}

/**
   Runs a mergesort_pthread corner cases test on random double arrays.
*/
void run_double_corner_test(){
  int res = 1;
  size_t count, sb, mb;
  size_t i, j;
  size_t elt_size =  sizeof(double);
  double *arr_a = NULL, *arr_b = NULL;
  arr_a =  malloc_perror(C_CORNER_COUNT_MAX, elt_size);
  arr_b =  malloc_perror(C_CORNER_COUNT_MAX, elt_size);
  printf("Test mergesort_pthread on corner cases on random "
	 "double arrays\n");
  for (count = 1; count <= C_CORNER_COUNT_MAX; count++){
    for (sb = C_CORNER_SBASE_START; sb <= C_CORNER_SBASE_END; sb++){
      for (mb = C_CORNER_MBASE_START; mb <= C_CORNER_MBASE_END; mb++){
	for(i = 0; i < C_CORNER_TRIALS; i++){
	  for (j = 0; j < count; j++){
	    arr_a[j] = (DRAND() < C_HALF_PROB ? -1 : 1) * DRAND();
	  }
	  memcpy(arr_b, arr_a, count * elt_size);
	  mergesort_pthread(arr_a, count, elt_size, sb, mb, cmp_double);
	  qsort(arr_b, count, elt_size, cmp_double);
	  for (j = 0; j < count; j++){
	    res *= (arr_a[j] == arr_b[j]);
	  }
	}
      }
    }
  }
  printf("\tcorrectness:       ");
  print_test_result(res);
  free(arr_a);
  free(arr_b);
  arr_a = NULL;
  arr_b = NULL;
}

/**
   Runs a test comparing mergesort_pthread vs. qsort performance on random 
   double arrays across sort and merge base count bounds.
*/
void run_double_opt_test(int pow_count_start,
			 int pow_count_end,
			 int pow_sbase_start,
			 int pow_sbase_end,
			 int pow_mbase_start,
			 int pow_mbase_end){
  int res = 1;
  int ci, si, mi;
  size_t count, sbase, mbase;
  size_t i, j;
  size_t elt_size = sizeof(double);
  double *arr_a = NULL, *arr_b = NULL;
  double tot_m, tot_q, t_m, t_q;
  arr_a =  malloc_perror(pow_two(pow_count_end), elt_size);
  arr_b =  malloc_perror(pow_two(pow_count_end), elt_size);
  printf("Test mergesort_pthread performance on random double arrays\n");
  for (ci = pow_count_start; ci <= pow_count_end; ci++){
    count = pow_two(ci); /* > 0 */
    printf("\t# trials: %lu, array count: %lu\n",
	   TOLU(C_TRIALS), TOLU(count));
    for (si = pow_sbase_start; si <= pow_sbase_end; si++){
      sbase = pow_two(si);
      printf("\t\tsort base count: %lu\n", TOLU(sbase));
      for (mi = pow_mbase_start; mi <= pow_mbase_end; mi++){
	mbase = pow_two(mi);
	printf("\t\t\tmerge base count: %lu\n", TOLU(mbase));
	tot_m = 0.0;
	tot_q = 0.0;
	for(i = 0; i < C_TRIALS; i++){
	  for (j = 0; j < count; j++){
	    arr_a[j] = (DRAND() < C_HALF_PROB ? -1 : 1) * DRAND();
	  }
	  memcpy(arr_b, arr_a, count * elt_size);
	  t_m = timer();
	  mergesort_pthread(arr_a, count, elt_size, sbase, mbase, cmp_double);
	  t_m = timer() - t_m;
	  t_q = timer();
	  qsort(arr_b, count, elt_size, cmp_double);
	  t_q = timer() - t_q;
	  tot_m += t_m;
	  tot_q += t_q;
	  for (j = 0; j < count; j++){
	    res *= (arr_a[j] == arr_b[j]);
	  }
	}
	printf("\t\t\tave pthread mergesort: %.6f seconds\n",
	       tot_m / C_TRIALS);
	printf("\t\t\tave qsort:             %.6f seconds\n",
	       tot_q / C_TRIALS);
	printf("\t\t\tcorrectness:           ");
	print_test_result(res);
      }
    }
  }
  free(arr_a);
  free(arr_b);
  arr_a = NULL;
  arr_b = NULL;
}

/**
   Times execution.
*/
double timer(){
  struct timeval tm;
  gettimeofday(&tm, NULL);
  return tm.tv_sec + tm.tv_usec / (double)1000000;
}

/**
   Print helper functions.
*/

void print_uint_elts(const size_t *a, size_t count){
  size_t i;
  for (i = 0; i < count; i++){
    printf("%lu ", TOLU(a[i]));
  }
  printf("\n");
}

void print_test_result(int res){
  if (res){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(int argc, char *argv[]){
  int i;
  size_t *args = NULL;
  RGENS_SEED();
  if (argc > C_ARGC_MAX){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  args = malloc_perror(C_ARGC_MAX - 1, sizeof(size_t));
  memcpy(args, C_ARGS_DEF, (C_ARGC_MAX - 1) * sizeof(size_t));
  for (i = 1; i < argc; i++){
    args[i - 1] = atoi(argv[i]);
  }
  if (args[0] > C_FULL_BIT - 2 ||
      args[1] > C_FULL_BIT - 2 ||
      args[2] > C_FULL_BIT - 1 ||
      args[3] > C_FULL_BIT - 1 ||
      args[4] > C_FULL_BIT - 1 ||
      args[5] > C_FULL_BIT - 1 ||
      args[4] < 1 ||
      args[5] < 1 ||
      args[0] > args[1] ||
      args[2] > args[3] ||
      args[4] > args[5] ||
      args[6] > 1 ||
      args[7] > 1 ||
      args[8] > 1 ||
      args[9] > 1){
    printf("USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  if (args[6]) run_int_corner_test();
  if (args[7]) run_int_opt_test(args[0],
				args[1],
				args[2],
				args[3],
				args[4],
				args[5]);
  if (args[8]) run_double_corner_test();
  if (args[9]) run_double_opt_test(args[0],
				   args[1],
				   args[2],
				   args[3],
				   args[4],
				   args[5]);
  free(args);
  args = NULL;
  return 0;
}
