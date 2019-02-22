
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"
/*
* Meghan Fair
* V00839675
* 
* UVic CSC 360 - Spring 2019
* 
* Implementation of the producer consumer problem using uthreads.
* This file is a copy of of pc_spinlock_uthread.c, but replaces 
* spinlocks with blocking mutexes and spinning with condition variables.
*
*/

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items


void* producer (void* pv) {

  for (int i=0; i<NUM_ITERATIONS; i++) {

  }
  return NULL;
}

void* consumer (void* v) {

  for (int i=0; i<NUM_ITERATIONS; i++) {
    
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];

  uthread_init (4);

  for (int i=0; i<NUM_CONSUMERS; i++) {
    t[i] = uthread_create(&consumer, 0);
  }
  for (int i=0; i<NUM_PRODUCERS; i++) {
    t[i+NUM_CONSUMERS] = uthread_create(&producer, 0);
  }

  for (int i=0; i<NUM_CONSUMERS+NUM_PRODUCERS; i++) {
    void *joinThreads;
    uthread_join(t[i], &joinThreads);
  }

  printf ("producer_wait_count=%d, consumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}