
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
const int NUM_ITERATIONS = 20;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;
uthread_mutex_t mutex;
uthread_cond_t cond;

void inc() {
  items++;
}

void dec() {
  items--;
}

void addToHistogram(int index) {
  histogram[index]++;
}

void* producer (void* v) {

  for (int i=0; i<NUM_ITERATIONS; i++) {

    uthread_mutex_lock(&mutex);

    while (items == MAX_ITEMS) {
        producer_wait_count++;
        uthread_cond_wait(&cond);
    }

    inc();
    addToHistogram(items);

    uthread_cond_broadcast(&cond);
    uthread_mutex_unlock(&mutex);
  }
  return NULL;
}

void* consumer (void* v) {

  for (int i=0; i<NUM_ITERATIONS; i++) {

    uthread_mutex_lock(&mutex);

    while (items == 0) {
        consumer_wait_count++;
        uthread_cond_wait(&cond);
    }

    dec();
    addToHistogram(items);

    uthread_cond_broadcast(&cond);
    uthread_mutex_unlock(&mutex);
  }
  
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];
  int i;
  uthread_init (4);
  mutex = uthread_mutex_create();
  cond = uthread_cond_create(&mutex);
  
  for (i =0; i < NUM_CONSUMERS; i++) {
    t[i] = uthread_create(&consumer, NULL);
  }
  for (i = 0; i < NUM_PRODUCERS; i++) {
    t[i+NUM_CONSUMERS] = uthread_create(&producer, NULL);
  }
  for (i = 0; i < 4; i++) {
    void *joinThread;
    uthread_join(t[i], &joinThread);
  }
  //free the memory
  uthread_mutex_destroy(&mutex);
  uthread_cond_destroy(&cond);

  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
