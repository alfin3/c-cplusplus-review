/**
   utilities-mod-test.c

   Tests of utility functions in modular arithmetic.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
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

/* tests */
const unsigned char C_UCHAR_MAX = (unsigned char)-1;
const size_t C_SIZE_MAX = (size_t)-1;
const size_t C_BYTE_BIT = CHAR_BIT;
const size_t C_FULL_BIT = CHAR_BIT * sizeof(size_t);
const size_t C_HALF_BIT = CHAR_BIT * sizeof(size_t) / 2;
const size_t C_BASE_MAX = (size_t)1 << (CHAR_BIT / 2); /* >1, <C_SIZE_MAX */

void print_test_result(int res);

/**
   Tests pow_mod.
*/
void run_pow_mod_test(int pow_trials){
  int res = 1;
  size_t i, trials;
  size_t base_sq_max = C_SIZE_MAX, k_max = 0, n_max;
  size_t j, k;
  size_t a, n;
  size_t r, r_wo;
  n_max = pow_two(C_HALF_BIT) - 2;
  trials = pow_two(pow_trials);
  while (base_sq_max / C_BASE_MAX >= C_BASE_MAX){
    base_sq_max /= C_BASE_MAX;
    k_max++;
  }
  printf("Run pow_mod random test\n ");
  printf("\t0 <= a <= %lu, 0 <= k <= %lu, 0 < n <= 2^%lu - 1 --> ",
	 TOLU(C_BASE_MAX), TOLU(k_max), TOLU(C_HALF_BIT));
  for (i = 0; i < trials; i++){
    r_wo = 1;
    a = RANDOM() % (C_BASE_MAX + 1);
    k = RANDOM() % (k_max + 1);
    n = 1 + DRAND() * n_max;
    r = pow_mod(a, k, n);
    for (j = 0; j < k; j++){
      r_wo *= a;
    }
    r_wo = r_wo % n;
    res *= (r == r_wo);
  }
  print_test_result(res);
  res = 1;
  printf("\ta = n - 1, 0 <= k < 2^%lu - 1, where 0 = k (mod 2), "
	 "1 < n <= 2^%lu - 1 --> ", TOLU(C_FULL_BIT), TOLU(C_FULL_BIT));
  k_max = C_SIZE_MAX - 1;
  n_max = C_SIZE_MAX - 2;
  for (i = 0; i < trials; i++){
    k = DRAND() * k_max;
    while (k & 1){
      k = DRAND() * k_max;
    }
    n = 2 + DRAND() * n_max;
    a = n - 1;
    r = pow_mod(a, k, n);
    res *= (r == 1);
  }
  print_test_result(res);
  res = 1;
  n_max = C_SIZE_MAX;
  res *= (pow_mod(0, 0, 1) == 0);
  res *= (pow_mod(2, 0, 1) == 0);
  res *= (pow_mod(0, 0, 2) == 1);
  res *= (pow_mod(2, 0, 2) == 1);
  res *= (pow_mod(n_max, n_max, n_max) == 0);
  res *= (pow_mod(n_max - 1, n_max, n_max) == n_max - 1);
  res *= (pow_mod(n_max, n_max - 1, n_max) == 0);
  printf("\tcorner cases --> ");
  print_test_result(res);
}

/**
   Tests mul_mod.
*/
void run_mul_mod_test(int pow_trials){
  int res = 1;
  size_t i, trials;
  size_t a_max, b_max, n_max;
  size_t a, b, n;
  size_t r, r_wo;
  trials = pow_two(pow_trials);
  a_max = pow_two(C_HALF_BIT) - 1;
  b_max = pow_two(C_HALF_BIT) - 1;
  n_max = C_SIZE_MAX - 1;
  printf("Run mul_mod random test\n");
  for (i = 0; i < trials; i++){
    a = DRAND() * a_max;
    b = DRAND() * b_max;
    n = 1 + DRAND() * n_max;
    r = mul_mod(a, b, n);
    r_wo = (a * b) % n;
    res *= (r == r_wo);
  }
  printf("\ta, b <= 2^%lu - 1, 0 < n <= 2^%lu - 1 --> ",
	 TOLU(C_HALF_BIT), TOLU(C_FULL_BIT));
  print_test_result(res);
  res = 1;
  for (i = 0; i < trials; i++){
    n = 2 + DRAND() * (n_max - 1);
    r = mul_mod(n - 1, n - 1, n);
    res *= (r == 1);
  }
  printf("\ta, b = n - 1, 1 < n <= 2^%lu - 1 --> ", TOLU(C_FULL_BIT));
  print_test_result(res);
  res = 1;
  res *= (mul_mod(0, 0, 1) == 0);
  res *= (mul_mod(1, 0, 2) == 0);
  res *= (mul_mod(0, 1, 2) == 0);
  res *= (mul_mod(0, 2, 2) == 0);
  res *= (mul_mod(1, 1, 2) == 1);
  res *= (mul_mod(0, n_max - 1, n_max) == 0);
  res *= (mul_mod(n_max - 1, 0, n_max) == 0);
  res *= (mul_mod(n_max - 1, 1, n_max) == n_max - 1);
  res *= (mul_mod(1, n_max - 1, n_max) == n_max - 1);
  res *= (mul_mod(n_max - 1, n_max - 1, n_max - 1) == 0);
  res *= (mul_mod(n_max - 1, n_max - 1, n_max) == 1);
  res *= (mul_mod(n_max, n_max, n_max) == 0);
  printf("\tcorner cases --> ");
  print_test_result(res);
}

/**
   Tests sum_mod.
*/
void run_sum_mod_test(int pow_trials){
  int res = 1;
  size_t i, trials;
  size_t a_max, b_max, n_max;
  size_t a, b, n;
  size_t r, r_wo;
  trials = pow_two(pow_trials);
  a_max = pow_two(C_FULL_BIT - 1) - 1;
  b_max = pow_two(C_FULL_BIT - 1) - 1;
  n_max = C_SIZE_MAX - 1;
  printf("Run sum_mod random test\n");
  for (i = 0; i < trials; i++){
    a = DRAND() * a_max;
    b = DRAND() * b_max;
    n = 1 + DRAND() * n_max;
    r = sum_mod(a, b, n);
    r_wo = (a + b) % n;
    res *= (r == r_wo);
  }
  printf("\ta, b <= 2^%lu - 1 (mod n), 0 < n <= 2^%lu - 1 --> ",
	 TOLU(C_FULL_BIT - 1), TOLU(C_FULL_BIT));
  print_test_result(res);
  res = 1;
  for (i = 0; i < trials; i++){
    b = 1 + DRAND() * n_max;
    r = sum_mod(n_max, b, n_max + 1);
    res *= (r == b - 1);
  }
  printf("\ta = 2^%lu - 2, 0 < b <= 2^%lu - 1, n = 2^%lu - 1 --> ",
	 TOLU(C_FULL_BIT), TOLU(C_FULL_BIT), TOLU(C_FULL_BIT));
  print_test_result(res);
  res = 1;
  res *= (sum_mod(0, 0, 1) == 0);
  res *= (sum_mod(1, 0, 2) == 1);
  res *= (sum_mod(0, 1, 2) == 1);
  res *= (sum_mod(1, 1, 2) == 0);
  res *= (sum_mod(n_max - 1, n_max - 1, n_max) == n_max - 2);
  printf("\tcorner cases --> ");
  print_test_result(res);
}

/**
   Tests mem_mod.
*/
void run_mem_mod_test(int pow_trials,
		      int pow_size_start,
		      int pow_size_end){
  int res = 1;
  int j;
  unsigned char *block = NULL;
  size_t i, trials;
  size_t max = C_SIZE_MAX - 1;
  size_t num, n, mod_n;
  size_t size;
  clock_t t;
  trials = pow_two(pow_trials);
  size = sizeof(size_t);
  printf("Run mem_mod in a random test, size = %lu bytes  --> ", TOLU(size));
  for (i = 0; i < trials; i++){
    num = DRAND() * max;
    n = 1 + DRAND() * max;
    res *= (num % n == mem_mod(&num, size, n));
  }
  print_test_result(res);
  res = 1;
  printf("Run mem_mod on large memory blocks \n");
  n = 1 + DRAND() * max;
  for (j = pow_size_start; j <= pow_size_end; j++){
    size = pow_two(j) + 1;
    block = calloc_perror(1, size);
    block[size - 1] = (unsigned char)1;
    t = clock();
    mod_n = mem_mod(block, size, n);
    t = clock() - t;
    res = (mod_n == pow_mod(mul_mod(pow_two(C_BYTE_BIT - 1), 2, n),
			    pow_two(j),
			    n));
    printf("\tblock size:  %lu bytes \n", TOLU(size));
    printf("\truntime:     %.8f seconds \n", (float)t / CLOCKS_PER_SEC);
    printf("\tcorrectness: ");
    print_test_result(res);
    res = 1;
    free(block);
    block = NULL;
  } 
}

/**
   Tests fast_mem_mod. A little-endian machine is assumed for the comparison
   test.
*/
void run_fast_mem_mod_test(int pow_trials,
			   int pow_size_start,
			   int pow_size_end){
  int res = 1;
  int j;
  unsigned char *block = NULL;
  size_t i, trials;
  size_t max = C_SIZE_MAX - 1;
  size_t num, n, mod_n;
  size_t k, size;
  clock_t t;
  trials = pow_two(pow_trials);
  size = sizeof(size_t);
  printf("Run fast_mem_mod in a random test, size = %lu bytes  --> ",
	 TOLU(size));
  for (i = 0; i < trials; i++){
    num = DRAND() * max;
    n = 1 + DRAND() * max;
    res *= (num % n == fast_mem_mod(&num, size, n));
  }
  print_test_result(res);
  res = 1;
  printf("Run fast_mem_mod on large memory blocks, n <= 2^%lu - 1 \n",
	 TOLU(C_HALF_BIT));
  max = pow_two(C_HALF_BIT) - 2;
  n = 1 + DRAND() * max;
  for (j = pow_size_start; j <= pow_size_end; j++){
    size = pow_two(j) + 1;
    block = calloc_perror(1, size);
    block[size - 1] = (unsigned char)1;
    t = clock();
    mod_n = fast_mem_mod(block, size, n);
    t = clock() - t;
    res = (mod_n == pow_mod(mul_mod(pow_two(C_BYTE_BIT - 1), 2, n),
			    pow_two(j),
			    n));
    printf("\tblock size:  %lu bytes \n", TOLU(size));
    printf("\truntime:     %.8f seconds \n", (float)t / CLOCKS_PER_SEC);
    printf("\tcorrectness: ");
    print_test_result(res);
    res = 1;
    free(block);
    block = NULL;
  }
  printf("Run fast_mem_mod on large memory blocks, "
	 "2^%lu - 1 < n <= 2^%lu - 1 \n",
	 TOLU(C_HALF_BIT), TOLU(C_FULL_BIT));
  max = C_SIZE_MAX;
  n = pow_two(C_HALF_BIT) + DRAND() * (max - pow_two(C_HALF_BIT));
  for (j = pow_size_start; j <= pow_size_end; j++){
    size = pow_two(j) + 1;
    block = calloc_perror(1, size);
    block[size - 1] = (unsigned char)1;
    t = clock();
    mod_n = fast_mem_mod(block, size, n);
    t = clock() - t;
    res = (mod_n == pow_mod(mul_mod(pow_two(C_BYTE_BIT - 1), 2, n),
			    pow_two(j),
			    n));
    printf("\tblock size:  %lu bytes \n", TOLU(size));
    printf("\truntime:     %.8f seconds \n", (float)t / CLOCKS_PER_SEC);
    printf("\tcorrectness: ");
    print_test_result(res);
    res = 1;
    free(block);
    block = NULL;
  } 
  printf("Run fast_mem_mod and mem_mod comparison on random blocks "
	 "of random size (little endian machine req.) --> ");
  fflush(stdout);
  max = C_SIZE_MAX - 1;
  for (i = 0; i < trials; i++){
    size = 1 + RANDOM() % C_UCHAR_MAX;
    block = calloc_perror(1, size);
    n = 1 + DRAND() * max;
    for (k = 0; k < size; k++){
      block[k] = DRAND() * C_UCHAR_MAX;
    }  
    res *= (fast_mem_mod(block, size, n) == mem_mod(block, size, n));
    free(block);
    block = NULL;
  }
  print_test_result(res);
}

/**
   Tests mul_mod_pow_two.
*/
void run_mul_mod_pow_two_test(int pow_trials){
  int res = 1;
  size_t i, trials;
  size_t max;
  size_t a, b, h, l, ret;
  trials = pow_two(pow_trials);
  max = pow_two(C_HALF_BIT) - 1;
  printf("Run mul_mod_pow_two random test\n");
  for (i = 0; i < trials; i++){
    a = DRAND() * max;
    b = DRAND() * max;
    ret = mul_mod_pow_two(a, b);
    res *= (ret == a * b);
  }
  printf("\t0 <= a, b <= 2^%lu - 1  --> ", TOLU(C_HALF_BIT));
  print_test_result(res);
  res = 1;
  max = C_SIZE_MAX - 1;
  for (i = 0; i < trials; i++){
    a = 1 + DRAND() * max;
    b = 1 + DRAND() * max;
    mul_ext(a, b, &h, &l);
    ret = mul_mod_pow_two(a, b);
    res *= (ret == l && ret == a * b);
  }
  printf("\t0 < a, b <= 2^%lu - 1 --> ", TOLU(C_FULL_BIT));
  print_test_result(res);
  res = 1;
  res *= (mul_mod_pow_two(0, 0) == 0);
  res *= (mul_mod_pow_two(1, 0) == 0);
  res *= (mul_mod_pow_two(0, 1) == 0);
  res *= (mul_mod_pow_two(1, 1) == 1);
  res *= (mul_mod_pow_two(pow_two(C_HALF_BIT),
			  pow_two(C_HALF_BIT)) == 0);
  res *= (mul_mod_pow_two(pow_two(C_FULL_BIT - 1),
			  pow_two(C_FULL_BIT - 1)) == 0);
  res *= (mul_mod_pow_two(C_SIZE_MAX, C_SIZE_MAX) == 1);
  printf("\tcorner cases --> ");
  print_test_result(res);
}

/**
   Tests mul_ext.
*/
void run_mul_ext_test(int pow_trials){
  int res = 1;
  size_t i, trials;
  size_t max;
  size_t a, b, n, h, l;
  size_t *hl = NULL;
  trials = pow_two(pow_trials);
  max = pow_two(C_HALF_BIT) - 1;
  hl = malloc_perror(2, sizeof(size_t));
  printf("Run mul_ext random test\n");
  for (i = 0; i < trials; i++){
    a = DRAND() * max;
    b = DRAND() * max;
    mul_ext(a, b, &h, &l);
    res *= (h == 0);
    res *= (l == a * b);
  }
  printf("\t0 <= a, b <= 2^%lu - 1  --> ", TOLU(C_HALF_BIT));
  print_test_result(res);
  res = 1;
  max = C_SIZE_MAX - 1;
  for (i = 0; i < trials; i++){
    a = 1 + DRAND() * max;
    b = 1 + DRAND() * max;
    n = 1 + DRAND() * max;
    mul_ext(a, b, &h, &l);
    hl[0] = l;
    hl[1] = h;
    res *= (mem_mod(hl, 2 * sizeof(size_t), n) == mul_mod(a, b, n));
  }
  printf("\t0 < a, b <= 2^%lu - 1 --> ", TOLU(C_FULL_BIT));
  print_test_result(res);
  res = 1;
  mul_ext(0, 0, &h, &l);
  res *= (h == 0 && l == 0);
  mul_ext(1, 0, &h, &l);
  res *= (h == 0 && l == 0);
  mul_ext(0, 1, &h, &l);
  res *= (h == 0 && l == 0);
  mul_ext(1, 1, &h, &l);
  res *= (h == 0 && l == 1);
  mul_ext(pow_two(C_HALF_BIT), pow_two(C_HALF_BIT), &h, &l);
  res *= (h == 1 && l == 0);
  mul_ext(pow_two(C_FULL_BIT - 1), pow_two(C_FULL_BIT - 1), &h, &l);
  res *= (h == pow_two(C_FULL_BIT - 2) && l == 0);
  mul_ext(C_SIZE_MAX, C_SIZE_MAX, &h, &l);
  res *= (h == C_SIZE_MAX - 1 && l == 1);
  printf("\tcorner cases --> ");
  print_test_result(res);
}

/**
   Tests represent_uint.
*/
void run_represent_uint_test(int pow_trials){
  int res = 1;
  int i, trials;
  size_t n, k, u;
  size_t j;
  trials = pow_two(pow_trials);
  printf("Run represent_uint odds test --> ");
  for (i = 0; i < trials; i++){
    n = RANDOM();
    if (!(n & 1)) n++;
    represent_uint(n, &k, &u);
    res *= (k == 0 && u == n);
  }
  print_test_result(res);
  res = 1;
  printf("Run represent_uint odds * 2^k test --> ");
  for (i = 0; i < trials; i++){
    for (j = 0; j <= C_FULL_BIT - C_BYTE_BIT; j++){
      n = RANDOM() % C_UCHAR_MAX;
      if (!(n & 1)) n++; /* <= C_UCHAR_MAX as size_t */
      represent_uint(pow_two(j) * n, &k, &u);
      res *= (k == j && u == n);
    }
  }
  print_test_result(res);
  res = 1;
  printf("Run represent_uint corner cases test --> ");
  represent_uint(0, &k, &u);
  res *= (k == C_FULL_BIT && u == 0);
  represent_uint(1, &k, &u);
  res *= (k == 0 && u == 1);
  print_test_result(res);
}

/**
   Tests pow_two.
*/
void run_pow_two_test(){
  int res = 1;
  int i, trials = C_FULL_BIT;
  size_t prod = 1;
  for (i = 0; i < trials; i++){
    res *= (prod == pow_two(i));
    prod *= 2;
  }
  printf("Run pow_two test --> ");
  print_test_result(res);
}

void print_test_result(int res){
  if (res){
    printf("SUCCESS\n");
  }else{
    printf("FAILURE\n");
  }
}

int main(){
  RGENS_SEED();
  run_pow_mod_test(20);
  run_mul_mod_test(20);
  run_sum_mod_test(20);
  run_mem_mod_test(20, 10, 17);
  run_fast_mem_mod_test(10, 10, 17);
  run_mul_mod_pow_two_test(20);
  run_mul_ext_test(20);
  run_represent_uint_test(20);
  run_pow_two_test();
  return 0;
}
