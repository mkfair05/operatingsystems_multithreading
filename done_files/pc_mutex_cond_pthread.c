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

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 5;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items
int producer_wait_count = 0;  //number of times producer has to wait
int consumer_wait_count = 0;  //number of times consumer has to wait
int items = 0;
pthread_mutex_t mutex; //to allow threads to add/remove items
pthread_cond_t  cond; //condition variable for when items may be added or removed

void inc () {
  items++;
}

void dec () {
  items--;
}

void addToHistogram (int index) {
  histogram[index]++;
}

void* producer (void* v) {

  for (int i=0; i<NUM_ITERATIONS; i++) {
    VERBOSE_PRINT("producer locking mutex\n");
    pthread_mutex_lock(&mutex);
    
    while (items == MAX_ITEMS) {  //items is full
      //wait until there is some room in items before producing more
      VERBOSE_PRINT("producer waiting\n");
      producer_wait_count++;
      pthread_cond_wait(&cond, &mutex);
    }

    VERBOSE_PRINT("incrementing items\n");
    inc();
    addToHistogram(items);
    
    VERBOSE_PRINT("producer broadcasting and unlocking\n");
    pthread_cond_broadcast(&cond); //broadcast condition variable status to all threads
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

void* consumer (void* v) {

  for (int i=0; i<NUM_ITERATIONS; i++) {
    VERBOSE_PRINT("consumer locking mutex\n");
    pthread_mutex_lock(&mutex);
    
    while (items == 0) {  //items is empty
      // wait until there is some items that can be consumed 
      VERBOSE_PRINT("consumer waiting\n");
      consumer_wait_count++;
      pthread_cond_wait(&cond, &mutex);
    }

    VERBOSE_PRINT("decrementing items\n");
    dec();
    addToHistogram(items);
    
    VERBOSE_PRINT("consumer broadcasting and unlocking\n");
    pthread_cond_broadcast(&cond); //broadcast condition variable status to all threads
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}


int main (int argc, char** argv) {

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);
  pthread_cond_init(&cond, NULL);
  int i;

  pthread_t t[NUM_CONSUMERS+NUM_PRODUCERS];

  VERBOSE_PRINT("creating consumers\n");
  for (i=0; i < NUM_CONSUMERS; i++) {
    //creates two consumers
    pthread_create(&t[i], NULL, consumer, NULL);
  }

  VERBOSE_PRINT("creating producers\n");
  for (i=0; i < NUM_PRODUCERS; i++) {
    //creates two producers
    pthread_create(&t[i+NUM_CONSUMERS], NULL, producer, NULL);
  }

  VERBOSE_PRINT("joining threads\n");
  for (i = 0; i < NUM_CONSUMERS+NUM_PRODUCERS; i++) {
    void *joinThreads;
    pthread_join(t[i], &joinThreads);
  }

  printf ("\nproducer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("\n\nitems value histogram:\n");

  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  printf("  -------\n  total = %d\n", sum);
    
  assert (sum == sizeof (t) / sizeof (pthread_t) * NUM_ITERATIONS);
  return 0;
}