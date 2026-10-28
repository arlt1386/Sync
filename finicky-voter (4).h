/*
 * 
 *
 * CS 441/541: Finicky Voter 
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "semaphore_support.h"
#include <string.h>
#include <ctype.h>

/*****************************
 * Defines
 *****************************/

/*****************************
 * Structures
 *****************************/
struct voter{
  pthread_t thread;
  int identify;
  char party; /* 'D', 'R', 'I' */ 
};
typedef struct voter voter;

/*****************************
 * Global Variables
 *****************************/
voter * threads;
semaphore_t printf_sem;
semaphore_t lock;
semaphore_t dem_mutex;
semaphore_t rep_mutex;
semaphore_t turnstile;
semaphore_t booths;
semaphore_t booth_mutex;
semaphore_t ready;
semaphore_t print_mutex;
semaphore_t barrier;
int num_booths = 10;
int num_dem = 5;
int num_rep = 5;
int num_ind = 5;
int total_voters = 15;
int dem_count = 0;
int rep_count = 0;
int ready_count = 0;
char * booths_free;

/*****************************
 * Function Declarations
 *****************************/
void * vote(void *);
void print_state(int tid,/*char party, int id,*/ const char *state, int booth_num); 
int assign_booth(int);
