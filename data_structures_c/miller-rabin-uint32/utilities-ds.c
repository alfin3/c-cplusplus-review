/**
   utilities-ds.c

   Utility functions across the areas of randomness, modular arithmetic, 
   and binary representation.

   Update: 6/14/2020
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

uint32_t random_range_uint32(uint32_t n);
uint32_t pow_mod_uint32(uint32_t a, uint32_t k, uint32_t n);
void represent_uint64(uint64_t n, int *k, uint64_t *u);
uint64_t pow_two_uint64(int k);

/** Randomness */

/**
   Returns a generator-uniform random uint32_t in [0 , n) where 
   0 < n <= 2^32 - 1.
*/
static uint32_t random_range_helper(uint32_t n);

uint32_t random_range_uint32(uint32_t n){
  assert(RAND_MAX == 2147483647);
  assert(0 < n);
  uint32_t rand_max = RAND_MAX;
  uint32_t upper;
  uint32_t ret;
  if (n - 1 <= rand_max){
    upper = n; 
    ret = random_range_helper(upper);
  }else{
    //need two calls to the generator
    upper = rand_max + 1;
    ret = random_range_helper(upper);
    upper = n - rand_max;
    ret += random_range_helper(upper);
  }
  return ret;
}

/**
   Returns a generator-uniform random uint32_t in [0 , n) where 
   0 < n <= RAND_MAX + 1.
*/
static uint32_t random_range_helper(uint32_t n){
  uint32_t rand_max = RAND_MAX;
  uint32_t cut;
  uint32_t rand_num;
  uint32_t ret;
  if (rand_max % n == n - 1){
    ret =  random() % n;
  }else{
    cut = (rand_max % n) + 1;
    rand_num = random();
    while(rand_num > rand_max - cut){
      rand_num = random();
    }
    ret = rand_num % n;
  }
  return ret;
}

/** Modular arithmetic */

/**
   Computes mod n of the kth power in O(logk) time, based on the 
   binary representation of k and the following relations :
   if a1 ≡ b1 (mod n) and a2 ≡ b2 (mod n) then 
   a1 a2 ≡ b1 b2 (mod n), and a1 + a2 ≡ b1 + b2 (mod n).
*/
uint32_t pow_mod_uint32(uint32_t a, uint32_t k, uint32_t n){
  assert(n > 0);
  if(!k){return 1 % n;};
  //"no overflow" guarantee
  uint64_t new_a = a;
  uint64_t new_n = n;
  //initial mods
  uint64_t ret = 1;
  new_a = new_a % new_n;
  //mods of products
  while (k){
    if (k & 1){
      ret = (ret * new_a) % new_n; //update for each set bit
    }
    new_a = (new_a * new_a) % new_n; //repetitive squaring between updates
    k >>= 1;
  }
  assert(ret < pow_two_uint64(32));
  return (uint32_t)ret;
}

/** Binary representation */

/**
   Represents n as u * 2^k, where u is odd.
*/
void represent_uint64(uint64_t n, int *k, uint64_t *u){
  int c = 0;
  uint64_t shift_n = n;
  while(shift_n){
    c++;
    shift_n <<= 1;
  }
  *k = sizeof(uint64_t) * 8 - c;
  *u = n >> *k; //*k <= sizeof(uint64_t)
}

/**
   Returns the kth power of 2, where 0 <= k <= 63.
*/
uint64_t pow_two_uint64(int k){
  assert(0 <= k && k <= 63);
  uint64_t ret = 1;
  return ret << k;
} 