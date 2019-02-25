#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_sem.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;

uthread_sem_t  semLock = uthread_sem_create(1); //init open semaphore for locking/unlocking
uthread_sem_t  isFull = uthread_sem_create(MAX_ITEMS);
uthread_sem_t  isEmpty = uthread_sem_create(0);

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
    
    uthread_sem_wait(&isFull); //decrement/block

    uthread_sem_wait(&semLock); //lock thread
    
    assert(items < MAX_ITEMS);
    inc();
    addToHistogram(items);

    uthread_sem_signal(&isFull); //inc/unblock
    uthread_sem_signal(&semLock); //unlock thread
  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    uthread_sem_wait(isEmpty);
    uthread_sem_wait(semLock);

    assert(items > 0);
    dec();
    addToHistogram(items);

    uthread_sem_signal(isEmpty);
    uthread_sem_signal(semLock);
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];

  uthread_init (4);


  // TODO: Create Threads and Join

  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
