/**
   utilities-rand-uint32.c

   Utility functions in randomness.

   The implementation is based on the random() function that returns a
   random number from 0 to RAND_MAX, where RAND_MAX is 2^31 - 1, with a
   large period of approx. 16 * (2^31 - 1). The provided functions are
   seeded by seeding random() outside the functions. The implementation is
   not suitable for cryptographic use.
   (https://man7.org/linux/man-pages/man3/random.3.html)
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "utilities-rand-uint32.h"
#include "utilities-mod.h"

static const uint32_t FULL_BIT_COUNT = 8 * sizeof(uint32_t);
static const uint32_t RAND_MAX_UINT32_TEST = 2147483647;
static const uint32_t RAND_MAX_UINT32 = RAND_MAX; //associate with type

static uint32_t random_mod_pow_two(uint32_t k);
static uint32_t random_gen_range(uint32_t n);
static void fprintf_stderr_exit(const char *s, int line);

/**
   Returns a generator-uniform random uint32_t in [0 , n), where n > 0.
   In the while loop, the probability of not finding a number decreases
   exponentially, and is less or equal to 0.5^n under the assumption of
   generator uniformity, where n is the number of random() calls if 
   n <= RAND_MAX, and the number of random_uint32() otherwise.
*/
uint32_t random_range_uint32(uint32_t n){
  if (n <= RAND_MAX_UINT32){
    ret = random_gen_range(n);
  }else{
    ret = random_uint32();
    while(ret > n - 1){
      ret = random_uint32();
    }
  }
  return ret;
}

/**
   Returns a generator-uniform random uint32_t. 
*/
uint32_t random_uint32(){
  return random_mod_pow_two(FULL_BIT_COUNT)
}

/**
   Returns a generator-uniform random uint32_t mod 2^k. 
*/
static uint32_t random_mod_pow_two(uint32_t k){
  uint32_t ret, n;
  if (RAND_MAX_UINT32 != RAND_MAX_UINT32_TEST){
    fprintf_stderr_exit("RAND_MAX value error", __LINE__);
  }
  ret = random();
  if (k < FULL_BIT_COUNT){
    ret = ret >> FULL_BIT_COUNT - k - 1;
  }else{
    n = random();
    ret = ret | n << FULL_BIT_COUNT - 1;
  return ret;
}

/**
   Returns a generator-uniform random uint32_t in [0 , n) where 
   0 < n <= RAND_MAX + 1. In the while loop, the probability of not finding
   a number decreases exponentially, and is less or equal to 0.5^n under
   the assumption of generator uniformity, where n is the number of random()
   calls.
*/
static uint32_t random_gen_range(uint32_t n){
  uint32_t rand_val, rem, ret;
  if (RAND_MAX_UINT32 != RAND_MAX_UINT32_TEST){
    fprintf_stderr_exit("RAND_MAX value error", __LINE__);
  }
  rem = RAND_MAX_UINT32 % n;
  if (rem == n - 1){
    ret =  random();
  }else{
    ret = random();
    while(ret > RAND_MAX_UINT32 - rem - 1){
      ret = random();
    }
  }
  if (ret > n - 1) ret = ret % n;
  return ret;
}

/**
   Prints an error message and exits.
*/
static void fprintf_stderr_exit(const char *s, int line){
  fprintf(stderr, "%s in %s at line %d\n", s,  __FILE__, line);
  exit(EXIT_FAILURE);
}