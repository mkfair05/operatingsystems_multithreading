#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>


#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;

sem_t sem;

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
        
        sem_wait(&sem); //dec semaphore to 0 (blocks)
        
        inc();

        sem_post(&sem); //inc semaphore to 1 (signals)
        
    }
    return NULL;
}

void* consumer (void* v) {
    for (int i=0; i<NUM_ITERATIONS; i++) {
        sem_wait(&sem); //decrements/blocks semaphore
        
        dec();

        sem_post(&sem); //inc/signal sempahore
    }
    return NULL;
}

int main (int argc, char** argv) {
    pthread_t t[4];

    sem_init(&sem, 0, 1);
    int i;
    for (i=0; i < NUM_CONSUMERS; i++) {
        //creates two consumers
        pthread_create(&t[i], NULL, consumer, NULL);
    }

    for (i=0; i < NUM_PRODUCERS; i++) {
        //creates two producers
        pthread_create(&t[i+NUM_CONSUMERS], NULL, producer, NULL);
    }

    for (i = 0; i < NUM_CONSUMERS+NUM_PRODUCERS; i++) {
        //joins threads
        void *joinThreads;
        pthread_join(t[i], &joinThreads);
    }

    printf ("items value histogram:\n");
    int sum=0;
    for (int i = 0; i <= MAX_ITEMS; i++) {
        printf ("  items=%d, %d times\n", i, histogram [i]);
        sum += histogram [i];
    }
    assert (sum == sizeof (t) / sizeof (pthread_t) * NUM_ITERATIONS);
}
