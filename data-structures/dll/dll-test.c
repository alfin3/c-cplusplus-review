/**
   dll-test.c

   Tests of a generic doubly linked list in a circular representation.

   The following command line arguments can be used to customize tests:
   dll-test
      [0, # bits in int - 2) : i s.t. # inserts = 2**i
      [0, 1] : on/off prepend append free int test
      [0, 1] : on/off prepend append free int_ptr (noncontiguous) test
      [0, 1] : on/off corner cases test

   usage examples:
   ./dll-test
   ./dll-test 23
   ./dll-test 24 1 0 0

   dll-test can be run with any subset of command line arguments in the
   above-defined order. If the (i + 1)th argument is specified then the ith
   argument must be specified for i >= 0. Default values are used for the
   unspecified arguments according to the C_ARGS_DEF array.

   The implementation of tests does not use stdint.h and is portable under
   C89/C90 and C99 with the only requirement that CHAR_BIT * sizeof(size_t)
   is even.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "dll.h"
#include "utilities-mem.h"
#include "utilities-mod.h"

/* input handling */
const char *C_USAGE =
  "dll-test \n"
  "[0, # bits in int - 2) : i s.t. # inserts = 2**i \n"
  "[0, 1] : on/off prepend append free int test \n"
  "[0, 1] : on/off prepend append free int_ptr (noncontiguous) test \n"
  "[0, 1] : on/off corner cases test \n";
const int C_ARGC_MAX = 5;
const size_t C_ARGS_DEF[4] = {13, 1, 1, 1};
const size_t C_INT_BIT = CHAR_BIT * sizeof(int);

/* tests */
const int C_START_VAL = 0;

void prepend_append_free(dll_node_t **head_prep,
			 dll_node_t **head_app,
			 int start_val,
			 int num_ins,
			 size_t elt_size,
			 void (*new_elt)(void *, int),
			 int (*val_elt)(const void *),
			 void (*free_elt)(void *));
void *ptr(const void *block, size_t i, size_t size);
void print_dll(dll_node_t **head, int (*val)(const dll_node_t *));
void print_test_result(int res);

/**
   Run tests of a doubly linked list of integer keys and integer elements. 
   A pointer to an integer is passed as elt in prepend and append ops,
   and the integer is copied into the list. NULL as free_elt is sufficient
   to delete a node.
*/

void new_int(void *a, int val){
  *(int *)a = val;
}

int val_int(const void *a){
  return *(int *)a;
}

int cmp_int(const void *a, const void *b){
  if (*(int *)a == *(int *)b){
    return 0;
  }else{
    return 1;
  }
}

void run_prepend_append_free_int_test(int log_ins){
  int num_ins;
  int start_val = C_START_VAL;
  size_t elt_size = sizeof(int);
  dll_node_t *head_prep, *head_app; /* uninitialized pointers for testing */
  num_ins = pow_two_perror(log_ins);
  dll_init(&head_prep);
  dll_init(&head_app);
  printf("Run prepend, append, free test on int keys and int "
	 "elements\n");
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d\n",
	 start_val, start_val, num_ins);
  prepend_append_free(&head_prep,
		      &head_app,
		      start_val,
		      num_ins,
		      elt_size,
		      new_int,
		      val_int,
		      NULL);
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d (repeat test)\n",
	 start_val, start_val, num_ins);
  prepend_append_free(&head_prep,
		      &head_app,
		      start_val,
		      num_ins,
		      elt_size,
		      new_int,
		      val_int,
		      NULL);
  start_val += num_ins;
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d\n",
	 start_val, start_val, num_ins);
  prepend_append_free(&head_prep,
		      &head_app,
		      start_val,
		      num_ins,
		      elt_size,
		      new_int,
		      val_int,
		      NULL);
}

/**
   Run tests of a doubly linked list of integer keys and int_ptr_t elements. 
   A pointer to a pointer to an int_ptr_t element is passed as elt in 
   prepend and append ops and the pointer to the int_ptr_t element is
   copied into the list. A int_ptr_t-specific free_elt is necessary to
   delete a node.
*/

typedef struct{
  int *val;
} int_ptr_t;

void new_int_ptr(void *a, int val){
  int_ptr_t **s = a;
  (*s) = malloc_perror(1, sizeof(int_ptr_t));
  (*s)->val = malloc_perror(1, sizeof(int));
  *((*s)->val) = val;
}

int val_int_ptr(const void *a){
  return *((*(int_ptr_t **)a)->val);
}

int cmp_int_ptr(const void *a, const void *b){
  if (*((*(int_ptr_t **)a)->val) > *((*(int_ptr_t **)b)->val)){
    return 1;
  }else if (*((*(int_ptr_t **)a)->val) < *((*(int_ptr_t **)b)->val)){
    return -1;
  }else{
    return 0;
  }
}

void free_int_ptr(void *a){
  int_ptr_t **s = a;
  free((*s)->val);
  (*s)->val = NULL;
  free(*s);
  *s = NULL;
}

void run_prepend_append_free_int_ptr_test(int log_ins){
  int num_ins;
  int start_val = C_START_VAL;
  size_t elt_size = sizeof(int_ptr_t *);
  dll_node_t *head_prep, *head_app; /* uninitialized pointers for testing */
  num_ins = pow_two_perror(log_ins);
  dll_init(&head_prep);
  dll_init(&head_app);
  printf("Run prepend, append, free test on int keys and noncontiguous "
         "int_ptr_t elements \n");
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d\n",
	 start_val, start_val, num_ins);
  prepend_append_free(&head_prep,
		      &head_app,
		      start_val,
		      num_ins,
		      elt_size,
		      new_int_ptr,
		      val_int_ptr,
		      free_int_ptr);
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d (repeat test)\n",
	 start_val, start_val, num_ins);
  prepend_append_free(&head_prep,
		      &head_app,
		      start_val,
		      num_ins,
		      elt_size,
		      new_int_ptr,
		      val_int_ptr,
		      free_int_ptr);
  start_val += num_ins;
  printf("\tstart key value: %d, "
	 "start elt value: %d, "
	 "# nodes: %d\n",
	 start_val, start_val, num_ins);
  prepend_append_free(&head_prep,
		      &head_app,
		      start_val,
		      num_ins,
		      elt_size,
		      new_int_ptr,
		      val_int_ptr,
		      free_int_ptr);
}

/**
   Runs a corner cases tests.
*/
void run_corner_cases_test(){
  int res = 1;
  int *key = NULL;
  int elt;
  int i;
  size_t key_size = sizeof(int);
  size_t elt_size = sizeof(int);
  dll_node_t *head_none;
  dll_node_t *head_one_prep, *head_one_app;
  dll_node_t *head_two_prep, *head_two_app;
  key = calloc_perror(1, key_size);
  dll_init(&head_none);
  dll_init(&head_one_prep);
  dll_init(&head_one_app);
  dll_init(&head_two_prep);
  dll_init(&head_two_app);
  for (i = 0; i < 2; i++){
    if (i < 1){
      dll_prepend_new(&head_one_prep, &i, &i, key_size, elt_size);
      dll_append_new(&head_one_app, &i, &i, key_size, elt_size);
    }
    dll_prepend_new(&head_two_prep, &i, &i, key_size, elt_size);
    dll_append_new(&head_two_app, &i, &i, key_size, elt_size);
  }
  /* search */
  *key = 0;
  elt = 0;
  res *= (NULL == dll_search_key(&head_none, key, key_size));
  res *= (NULL == dll_search_elt(&head_none, &elt, key_size, cmp_int));
  res *= (NULL != dll_search_key(&head_one_prep, key, key_size));
  res *= (NULL != dll_search_elt(&head_one_prep, &elt, key_size, cmp_int));
  res *= (NULL != dll_search_key(&head_one_app, key, key_size));
  res *= (NULL != dll_search_elt(&head_one_app, &elt, key_size, cmp_int));
  res *= (NULL != dll_search_key(&head_two_prep, key, key_size));
  res *= (NULL != dll_search_elt(&head_two_prep, &elt, key_size, cmp_int));
  res *= (NULL != dll_search_key(&head_two_app, key, key_size));
  res *= (NULL != dll_search_elt(&head_two_app, &elt, key_size, cmp_int));
  *key = 1;
  elt = 1;
  res *= (NULL == dll_search_key(&head_none, key, key_size));
  res *= (NULL == dll_search_elt(&head_none, &elt, key_size, cmp_int));
  res *= (NULL == dll_search_key(&head_one_prep, key, key_size));
  res *= (NULL == dll_search_elt(&head_one_prep, &elt, key_size, cmp_int));
  res *= (NULL == dll_search_key(&head_one_app, key, key_size));
  res *= (NULL == dll_search_elt(&head_one_app, &elt, key_size, cmp_int));
  res *= (NULL != dll_search_key(&head_two_prep, key, key_size));
  res *= (NULL != dll_search_elt(&head_two_prep, &elt, key_size, cmp_int));
  res *= (NULL != dll_search_key(&head_two_app, key, key_size));
  res *= (NULL != dll_search_elt(&head_two_app, &elt, key_size, cmp_int));
  *key = 2;
  elt = 2;
  res *= (NULL == dll_search_key(&head_none, key, key_size));
  res *= (NULL == dll_search_elt(&head_none, &elt, key_size, cmp_int));
  res *= (NULL == dll_search_key(&head_one_prep, key, key_size));
  res *= (NULL == dll_search_elt(&head_one_prep, &elt, key_size, cmp_int));
  res *= (NULL == dll_search_key(&head_one_app, key, key_size));
  res *= (NULL == dll_search_elt(&head_one_app, &elt, key_size, cmp_int));
  res *= (NULL == dll_search_key(&head_two_prep, key, key_size));
  res *= (NULL == dll_search_elt(&head_two_prep, &elt, key_size, cmp_int));
  res *= (NULL == dll_search_key(&head_two_app, key, key_size));
  res *= (NULL == dll_search_elt(&head_two_app, &elt, key_size, cmp_int));
  /* delete */
  dll_delete(&head_none, NULL, key_size, NULL);
  dll_delete(&head_one_prep, NULL, key_size, NULL);
  dll_delete(&head_one_app, NULL, key_size, NULL);
  dll_delete(&head_two_prep, NULL, key_size, NULL);
  dll_delete(&head_two_app, NULL, key_size, NULL);
  res *= (NULL == head_none);
  res *= (0 == *(int *)dll_ptr(head_one_prep, key_size));
  res *= (0 == *(int *)dll_ptr(head_one_prep, 0));
  res *= (0 == *(int *)dll_ptr(head_one_app, key_size));
  res *= (0 == *(int *)dll_ptr(head_one_app, 0));
  res *= (1 == *(int *)dll_ptr(head_two_prep, key_size));
  res *= (1 == *(int *)dll_ptr(head_two_prep, 0));
  res *= (0 == *(int *)dll_ptr(head_two_app, key_size));
  res *= (0 == *(int *)dll_ptr(head_two_app, 0));
  dll_delete(&head_one_prep, head_one_prep, key_size, NULL);
  dll_delete(&head_one_app, head_one_app, key_size, NULL);
  dll_delete(&head_two_prep, head_two_prep, key_size, NULL);
  dll_delete(&head_two_app, head_two_app, key_size, NULL);
  res *= (NULL == head_one_prep);
  res *= (NULL == head_one_app);
  res *= (0 == *(int *)dll_ptr(head_two_prep, key_size));
  res *= (0 == *(int *)dll_ptr(head_two_prep, 0));
  res *= (1 == *(int *)dll_ptr(head_two_app, key_size));
  res *= (1 == *(int *)dll_ptr(head_two_app, 0));
  dll_delete(&head_two_prep, head_two_prep, key_size, NULL);
  dll_delete(&head_two_app, head_two_app, key_size, NULL);
  res *= (NULL == head_two_prep);
  res *= (NULL == head_two_app);
  printf("Run corner cases test --> ");
  print_test_result(res);
  free(key);
  key = NULL;
}

/** Helper functions */

/**
   Runs the prepend, append, and free test routine.
*/
void prepend_append_free(dll_node_t **head_prep,
			 dll_node_t **head_app,
			 int start_val,
			 int num_ins,
			 size_t elt_size,
			 void (*new_elt)(void *, int),
			 int (*val_elt)(const void *),
			 void (*free_elt)(void *)){
  int res = 1;
  int sum_val = 2 * start_val + num_ins - 1; /* < 2**(C_INT_BIT - 1) - 1 */
  int i;
  int *keys = NULL;
  size_t key_size = sizeof(int);
  void *elts_prep = NULL, *elts_app = NULL;
  dll_node_t *node_prep = NULL, *node_app = NULL;
  clock_t t_prep, t_app, t_free_prep, t_free_app;
  keys = calloc_perror(num_ins, key_size);
  elts_prep = malloc_perror(num_ins, elt_size);
  elts_app = malloc_perror(num_ins, elt_size);
  for (i = 0; i < num_ins; i++){
    keys[i] = start_val + i;
    new_elt(ptr(elts_prep, i, elt_size), start_val + i);
    new_elt(ptr(elts_app, i, elt_size), start_val + i);
  }  
  t_prep = clock();
  for (i = 0; i < num_ins; i++){
    dll_prepend_new(head_prep,
		    &keys[i],
		    ptr(elts_prep, i, elt_size),
		    key_size,
		    elt_size);
  }
  t_prep = clock() - t_prep;
  t_app = clock();
  for (i = 0; i < num_ins; i++){
    dll_append_new(head_app,
		   &keys[i],
		   ptr(elts_app, i, elt_size),
		   key_size,
		   elt_size);
  }
  t_app = clock() - t_app;
  node_prep = *head_prep;
  node_app = *head_app;
  for (i = 0; i < num_ins; i++){
    res = (*(int *)dll_ptr(node_prep, 0) +
	   *(int *)dll_ptr(node_app, 0) ==
	   sum_val);
    res = (val_elt(dll_ptr(node_prep, key_size)) +
	   val_elt(dll_ptr(node_app, key_size)) ==
	   sum_val);
    node_prep = node_prep->next;
    node_app = node_app->next;
  }
  t_free_prep = clock();
  dll_free(head_prep, key_size, free_elt);
  t_free_prep = clock() - t_free_prep;
  t_free_app = clock();
  dll_free(head_app, key_size, free_elt);
  t_free_app = clock() - t_free_app;
  res *= (head_prep != NULL && *head_prep == NULL);
  res *= (head_app != NULL && *head_app == NULL);
  printf("\t\tprepend time:            %.4f seconds\n",
	 (float)t_prep / CLOCKS_PER_SEC);
  printf("\t\tappend time:             %.4f seconds\n",
	 (float)t_app / CLOCKS_PER_SEC);
  printf("\t\tfree after prepend time: %.4f seconds\n",
	 (float)t_free_prep / CLOCKS_PER_SEC);
  printf("\t\tfree after append time:  %.4f seconds\n",
	 (float)t_free_app / CLOCKS_PER_SEC);
  printf("\t\tcorrectness:             ");
  print_test_result(res);
  free(keys);
  free(elts_prep);
  free(elts_app);
  keys = NULL;
  elts_prep = NULL;
  elts_app = NULL;
}

/**
   Computes a pointer to an element in an array of elements of size size.
*/
void *ptr(const void *block, size_t i, size_t size){
  return (void *)((char *)block + i * size);
}

/**
   Prints a test result.
*/
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
  if (argc > C_ARGC_MAX){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  args = malloc_perror(C_ARGC_MAX - 1, sizeof(size_t));
  memcpy(args, C_ARGS_DEF, (C_ARGC_MAX - 1) * sizeof(size_t));
  for (i = 1; i < argc; i++){
    args[i - 1] = atoi(argv[i]);
  }
  if (args[0] > C_INT_BIT - 3 ||
      args[1] > 1 ||
      args[2] > 1 ||
      args[3] > 1){
    fprintf(stderr, "USAGE:\n%s", C_USAGE);
    exit(EXIT_FAILURE);
  }
  if (args[1]){
    run_prepend_append_free_int_test(args[0]);
  }
  if (args[2]){
    run_prepend_append_free_int_ptr_test(args[0]);
  }
  if (args[3]){
    run_corner_cases_test();
  }
  free(args);
  args = NULL;
  return 0;
}

