#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"
#include "spinlock.h"
/*
* Meghan Fair
* V00839675
* 
* UVic CSC 360 - Spring 2019 Part 1 "Producer Consumer Problem"
* 
* Implementation of the producer consumer problem using uthreads and spinlocks
* to gaurantee mutual exclusion.
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

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;
spinlock_t spinLock;

void inc() {
  VERBOSE_PRINT("incrementing items\n");
  items++;
}

void dec() {
  VERBOSE_PRINT("decrementing items\n");
  items--;
}

void addToHistogram(int index) {
  histogram[index]++;
}

void* producer (void* v) {

  for (int i=0; i<NUM_ITERATIONS; i++) {
    VERBOSE_PRINT("producer locking spinlock\n");
    spinlock_lock(&spinLock);

    while (items == MAX_ITEMS) {
      // VERBOSE_PRINT("producer waiting\n");
      producer_wait_count++;
      spinlock_unlock(&spinLock);
      spinlock_lock(&spinLock);
    }

    inc();
    addToHistogram(items);

    VERBOSE_PRINT("producer unlocking spinlock\n");
    spinlock_unlock(&spinLock);
  }
  return NULL;
}

void* consumer (void* v) {

  for (int i=0; i<NUM_ITERATIONS; i++) {

    VERBOSE_PRINT("consumer locking spinlock\n");
    spinlock_lock(&spinLock);

    while (items == 0) {
      // VERBOSE_PRINT("consumer waiting\n");  <--this is printed a lot
      consumer_wait_count++;
      spinlock_unlock(&spinLock);
      spinlock_lock(&spinLock);
    }

    dec();
    addToHistogram(items);

    VERBOSE_PRINT("consumer unlocking spinlock\n");
    spinlock_unlock(&spinLock);
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];
  int i;
  spinlock_create(&spinLock);
  uthread_init (4);
  
  VERBOSE_PRINT("creating consumers\n");
  for (i =0; i < NUM_CONSUMERS; i++) {
    t[i] = uthread_create(&consumer, NULL);
  }

  VERBOSE_PRINT("creating producers\n");
  for (i = 0; i < NUM_PRODUCERS; i++) {
    t[i+NUM_CONSUMERS] = uthread_create(&producer, NULL);
  }

  VERBOSE_PRINT("joining threads\n");
  for (i = 0; i < 4; i++) {
    void *joinThread;
    uthread_join(t[i], &joinThread);
  }
  
  printf ("\nproducer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
