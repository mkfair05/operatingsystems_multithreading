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
const int NUM_ITERATIONS = 50;
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

    if (pc_t->items == MAX_ITEMS) {  //items is full
      pthread_cond_wait(&pc_t->can_produce, &pc_t->mutex);
    }

    // pc_t->items++;
    inc();
    // addToHistogram(pc_t->items);
  }
  // printf("%d\n", pc_t->items);
  pthread_cond_signal(&pc_t->can_consume);
  pthread_mutex_unlock(&pc_t->mutex);
  return NULL;
}

void* consumer (void* v) {
  producer_consumer_t *pc_t = (producer_consumer_t*)v;

  for (int i=0; i<NUM_ITERATIONS; i++) {
    pthread_mutex_lock(&pc_t->mutex);

    if (pc_t->items == 0) {  //items is empty
      pthread_cond_wait(&pc_t->can_produce, &pc_t->mutex);
    }

    // pc_t->items--;
    dec();
  // printf("%d\n", pc_t->items);
    // addToHistogram(pc_t->items);
  }
  pthread_cond_signal(&pc_t->can_consume);
  pthread_mutex_unlock(&pc_t->mutex);
  return NULL;
}


int main (int argc, char** argv) {

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

  // for (i=0; i < NUM_CONSUMERS; i++) {
  //     //start off with null attributes, may need to change
  //   // printf("created thread id=%lu\n", consArr[i]);
  // }
  pthread_t cons, prod;
  pthread_create(&cons, NULL, producer, (void*)&pc_t);
  pthread_create(&prod, NULL, consumer, (void*)&pc_t);

  // for (i = 0; i < NUM_CONSUMERS; i++) {
  //   pthread_join(consArr[i], NULL);
  //   pthread_join(prodArr[i], NULL);
  // }
  pthread_join(cons, NULL);
  pthread_join(prod, NULL);

  // printf ("items value histogram:\n");
  // int sum=0;
  // for (int i = 0; i <= MAX_ITEMS; i++) {
  //   printf ("  items=%d, %d times\n", i, histogram [i]);
  //   sum += histogram [i];
  // }
  // assert (sum == sizeof (consArr) / sizeof (pthread_t) * NUM_ITERATIONS);
  return 0;
}
