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
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;

void* producer (void* v) {
  char *message = (char*)v;
  printf("%s \n", message);
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
  }
  return NULL;
}

void* consumer (void* v) {
  char *message = (char*)v;
  printf("%s \n", message);
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
  }
  return NULL;
}

int main (int argc, char** argv) {
  // pthread_t cons1, prod1;  //start off with 2 threads
  const char *message1 = "consumer";
  const char *message2 = "producer";


  int i;
  pthread_t consArr[NUM_CONSUMERS];
  pthread_t prodArr[NUM_PRODUCERS];
  float consID[NUM_CONSUMERS];
  float prodID[NUM_PRODUCERS];

  for (i=0; i < NUM_CONSUMERS; i++) {
    pthread_create(&consArr[i], NULL, consumer, (void *)message1);
      //start off with null attributes, may need to change
    // printf("created thread id=%lu\n", consArr[i]);
  }

  for (i=0; i < NUM_PRODUCERS; i++) {
    pthread_create(&prodArr[i], NULL, producer, (void *)message2);
      //start off with null attributes, may need to change    
    // printf("created thread id=%lu\n", prodArr[i]);
  }

  for (i = 0; i < NUM_CONSUMERS; i++) {
    pthread_join(consArr[i], NULL);
  }

  for (i = 0; i < NUM_PRODUCERS; i++) {
    pthread_join(prodArr[i], NULL);
  }

  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  // assert (sum == sizeof (consArr) / sizeof (pthread_t) * NUM_ITERATIONS);
  return 0;
}
