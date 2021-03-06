/**
   ht-divchn.h

   Struct declarations and declarations of accessible functions of a hash 
   table with generic hash keys and generic elements. The 
   implementation is based on a division method for hashing into upto  
   the number of slots determined by the largest prime number in the
   C_PRIME_PARTS array, representable as size_t on a given system, and a
   chaining method for resolving collisions. Due to chaining, the number
   of keys and elements that can be inserted is not limited by the hash
   table implementation.
   
   The load factor of a hash table is the expected number of keys in a slot 
   under the simple uniform hashing assumption, and is upper-bounded by the 
   alpha parameter. The alpha parameter does not provide an upper bound 
   after the maximum count of slots in a hash table is reached.

   A hash key is an object within a contiguous block of memory (e.g. a basic 
   type, array, struct). An element is within a contiguous or noncontiguous
   memory block.

   The implementation only uses integer and pointer operations. Integer
   arithmetic is used in load factor operations, thereby eliminating the
   use of float. Given parameter values within the specified ranges,
   the implementation provides an error message and an exit is executed
   if an integer overflow is attempted* or an allocation is not completed
   due to insufficient resources. The behavior outside the specified
   parameter ranges is undefined.

   The implementation does not use stdint.h and is portable under C89/C90
   and C99 with the only requirement that CHAR_BIT * sizeof(size_t) is
   greater or equal to 16 and is even.

   * except intended wrapping around of unsigned integers in modulo
     operations, which is defined, and overflow detection as a part
     of computing bounds, which is defined by the implementation.
*/

#ifndef HT_DIVCHN_H  
#define HT_DIVCHN_H

#include <stddef.h>
#include "dll.h"

typedef struct{
  size_t key_size;
  size_t elt_size;
  size_t pair_size; /* key_size + elt_size for input iterations by user */
  size_t group_ix;
  size_t count_ix; /* max size_t value if last representable prime reached */
  size_t count;
  size_t max_num_elts; /*  >= 0, <= C_SIZE_MAX, represents alpha */
  size_t num_elts;
  size_t alpha_n;
  size_t log_alpha_d; 
  dll_node_t **key_elts; /* array of pointers to nodes */
  void (*free_elt)(void *);
} ht_divchn_t;

/**
   Initializes a hash table. 
   ht          : a pointer to a preallocated block of size 
                 sizeof(ht_divchn_t).
   key_size    : non-zero size of a key object
   elt_size    : - non-zero size of an element, if the element is within a
                 contiguous memory block and a copy of the element is
                 inserted,
                 - size of a pointer to an element, if the element
                 is within a noncontiguous memory block or a pointer to a
                 contiguous element is inserted
   min_num     : minimum number of keys that are known or expected to become 
                 present simultaneously in a hash table, resulting in a
                 speedup by avoiding unnecessary growth steps of a hash
                 table; 0 if a positive value is not specified and all growth
                 steps are to be completed
   alpha_n     : > 0 numerator of load factor upper bound
   log_alpha_d : < CHAR_BIT * sizeof(size_t) log base 2 of denominator of
                 load factor upper bound; denominator is a power of two
   free_elt    : - if an element is within a contiguous memory block and
                 a copy of the element was inserted, then NULL as free_elt
                 is sufficient to delete the element,
                 - if an element is within a noncontiguous memory block or
                 a pointer to a contiguous element was inserted, then an
                 element-specific free_elt, taking a pointer to a pointer to an
                 element as its argument and leaving a block of size elt_size
                 pointed to by the argument, is necessary to delete the element
*/
void ht_divchn_init(ht_divchn_t *ht,
		    size_t key_size,
		    size_t elt_size,
		    size_t min_num,
		    size_t alpha_n,
		    size_t log_alpha_d,
		    void (*free_elt)(void *));

/**
   Inserts a key and an associated element into a hash table. If the key is
   in the hash table, associates the key with the new element. The key and 
   elt parameters are not NULL and point to blocks of size key_size and
   elt_size respectively.
*/
void ht_divchn_insert(ht_divchn_t *ht, const void *key, const void *elt);

/**
   If a key is present in a hash table, returns a pointer to its associated 
   element, otherwise returns NULL. The key parameter is not NULL and points
   to a block of size key_size.
*/
void *ht_divchn_search(const ht_divchn_t *ht, const void *key);

/**
   Removes a key and its associated element from a hash table by copying 
   the element or its pointer into a block of size elt_size pointed to
   by elt. If the key is not in the hash table, leaves the block pointed
   to by elt unchanged. The key and elt parameters are not NULL and point
   to blocks of size key_size and elt_size respectively.
*/
void ht_divchn_remove(ht_divchn_t *ht, const void *key, void *elt);

/**
   If a key is in a hash table, deletes the key and its associated element 
   according to free_elt. The key parameter is not NULL and points
   to a block of size key_size.
*/
void ht_divchn_delete(ht_divchn_t *ht, const void *key);

/**
   Frees a hash table and leaves a block of size sizeof(ht_divchn_t)
   pointed to by the ht parameter.
*/
void ht_divchn_free(ht_divchn_t *ht);

#endif
