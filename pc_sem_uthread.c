#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_sem.h"

/*
* Meghan Fair
* V00839675
* 
* UVic CSC 360 - Spring 2019
* 
* Implementation of the producer consumer problem using uthreads
* with semaphores.
*
*/

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;

uthread_sem_t notFull, notEmpty;

void inc() {
  items++;
  VERBOSE_PRINT("items = %d\n", items);
}

void dec() {
  items--;
  VERBOSE_PRINT("items = %d\n", items);
}

void addToHistogram(int index) {
  histogram[index]++;
}

void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    
    VERBOSE_PRINT("producer locking semaphore\n");
    uthread_sem_wait(notFull); //decrement/block

    VERBOSE_PRINT("increment items\n");
    inc();
    addToHistogram(items);

    VERBOSE_PRINT("producer unlocking semaphore\n");
    uthread_sem_signal(notEmpty); //inc/unblock
  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    VERBOSE_PRINT("consumer locking semaphore\n");
    uthread_sem_wait(notEmpty); //wait until semaphore is >0
    
    VERBOSE_PRINT("decrementing items\n");
    dec();
    addToHistogram(items);

    VERBOSE_PRINT("consumer unlocking semaphore\n");
    uthread_sem_signal(notFull); //indicate that the producer is allowed to produce
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];

  uthread_init (4);

  notFull = uthread_sem_create(MAX_ITEMS);
  notEmpty = uthread_sem_create(0);
  VERBOSE_PRINT("creating consumers\n");
  int i;
  
  for (i=0; i < NUM_CONSUMERS; i++) {
    //creates two consumers
    t[i] = uthread_create(consumer, NULL);
  }

  VERBOSE_PRINT("creating producers\n");
  for (i=0; i < NUM_PRODUCERS; i++) {
    //creates two producers
    t[i+NUM_CONSUMERS] = uthread_create(producer, NULL);
  }

  VERBOSE_PRINT("joining threads\n");
  for (i = 0; i < NUM_CONSUMERS+NUM_PRODUCERS; i++) {
    //joins threads
    void *joinThreads;
    uthread_join(t[i], &joinThreads);
  }


  printf ("\nitems value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
