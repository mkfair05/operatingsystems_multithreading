#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>


/*
* Meghan Fair
* V00839675
* 
* UVic CSC 360 - Spring 2019
* 
* Implementation of the producer consumer problem using pthreads
* with mutexes and condition variables.
*
*/


#define producer_wait_count  //number of times producer has to wait
#define consumer_wait_count  //number of times consumer has to wait
#define MAX_ITEMS 10
const int NUM_ITERATIONS = 10;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items
int items = 0;

typedef struct {
  int items; //number of items
  pthread_mutex_t mutex; //to allow threads to add/remove items
  pthread_cond_t can_consume; //signal when items added
  pthread_cond_t can_produce; //signal when items removed
} producer_consumer_t;

void inc () {
  items++;
  printf("items: %d \n", items);
}

void dec () {
  items--;
  printf("items: %d \n", items);
}

void addToHistogram (int index) {
  histogram[index]++;
}

void* producer (void* v) {
  producer_consumer_t *pc_t = (producer_consumer_t*)v;

  for (int i=0; i<NUM_ITERATIONS; i++) {

    pthread_mutex_lock(&pc_t->mutex);
    printf("Mutex Locked by producer\n");

    while (items == MAX_ITEMS) {  //items is full
      pthread_cond_wait(&pc_t->can_produce, &pc_t->mutex);
      printf("waiting for condition to produce\n");
    }

    printf("incrementing\n");
    inc();
    addToHistogram(items);
    printf("Signal consumer that producer done\n");
    pthread_cond_signal(&pc_t->can_consume);
    pthread_mutex_unlock(&pc_t->mutex);
    printf("Mutex unlocked by producer\n");
  }
  return NULL;
}

void* consumer (void* v) {
  producer_consumer_t *pc_t = (producer_consumer_t*) v;

  for (int i=0; i<NUM_ITERATIONS; i++) {

    pthread_mutex_lock(&pc_t->mutex);
    printf("Mutex Locked by consumer\n");

    while (items == 0) {  //items is empty
      pthread_cond_wait(&pc_t->can_consume, &pc_t->mutex);
      printf("waiting for condition to consume\n");
    }

    printf("decrementing\n");
    dec();
    addToHistogram(items);
    printf("Signal producer that consumer done\n");
    pthread_cond_signal(&pc_t->can_produce);
    pthread_mutex_unlock(&pc_t->mutex);
    printf("mutex unlocked by consumer\n");
  }
  return NULL;
}


int main (int argc, char** argv) {

  //initialize the data structure
  producer_consumer_t pc_t = {
    .items = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .can_produce = PTHREAD_COND_INITIALIZER,
    .can_consume = PTHREAD_COND_INITIALIZER
  };

  // pthread_t consArr[NUM_CONSUMERS];
  // pthread_t prodArr[NUM_PRODUCERS];
  // float consID[NUM_CONSUMERS];
  // float prodID[NUM_PRODUCERS];
  int i;

  pthread_t cons[NUM_CONSUMERS], prod[NUM_PRODUCERS];
  for (i=0; i < NUM_CONSUMERS; i++) {
    //creates two consumers and two producers
    pthread_create(&prod[i], NULL, producer, (void*)&pc_t);
    pthread_create(&cons[i], NULL, consumer, (void*)&pc_t);
  }

  for (i = 0; i < NUM_CONSUMERS; i++) {
    pthread_join(cons[i], NULL);
    pthread_join(prod[i], NULL);
  }

  printf ("\n\nitems value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  printf("  -------\n  total = %d\n", sum);
  // assert (sum == sizeof (pthread_t) / sizeof (pthread_t) * NUM_ITERATIONS);
  return 0;
}
