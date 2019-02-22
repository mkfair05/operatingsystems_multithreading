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
const int NUM_ITERATIONS = 5;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items
int items = 0;
pthread_mutex_t mutex; //to allow threads to add/remove items
pthread_cond_t  can_consume; //signal when items added
pthread_cond_t  can_produce; //signal when items removed

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
  // producer_consumer_t  = (producer_consumer_t*)v;

  for (int i=0; i<NUM_ITERATIONS; i++) {

    while (items == MAX_ITEMS) {  //items is full
      printf("waiting to produce\n");
      pthread_cond_wait(&can_produce, &mutex);
    }

    pthread_mutex_lock(&mutex);
    printf("Mutex Locked by producer\n");
    printf("incrementing\n");
    inc();
    addToHistogram(items);


    printf("Signal consumer that producer done\n");
    pthread_cond_signal(&can_consume);
    printf("Mutex unlocked by producer\n");
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

void* consumer (void* v) {
  // producer_consumer_t  = (producer_consumer_t*) v;

  for (int i=0; i<NUM_ITERATIONS; i++) {

    pthread_mutex_lock(&mutex);
    while (items == 0) {  //items is empty
      printf("waiting to consume\n");
      pthread_cond_wait(&can_consume, &mutex);
    }
    printf("Mutex Locked by consumer\n");
    printf("decrementing\n");
    dec();
    addToHistogram(items);


    printf("Signal producer that consumer done\n");
    pthread_cond_signal(&can_produce);
    printf("mutex unlocked by consumer\n");
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}


int main (int argc, char** argv) {

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&can_consume, NULL);
  pthread_cond_init(&can_produce, NULL);
  int i;

  pthread_t cons[NUM_CONSUMERS], prod[NUM_PRODUCERS];

  for (i=0; i < 2; i++) {
    //creates two consumers and two producers
    pthread_create(&prod[i], NULL, producer, 0);
    pthread_create(&cons[i], NULL, consumer, 0);
  }

  for (i = 0; i < 2; i++) {
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

  //TODO: sum is quadrupled with 4 threads. fix this
    
  assert (sum == sizeof (pthread_t) / sizeof (pthread_t) * NUM_ITERATIONS);
  //printf("%lu \n",sizeof (pthread_t) / sizeof (pthread_t) * NUM_ITERATIONS);
  return 0;
}
