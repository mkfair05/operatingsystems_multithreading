#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
/*
use the pthread flag with gcc to compile this code
~$ gcc -pthread producer_consumer.c -o producer_consumer
*/

/*
* Meghan Fair V00839675
* CSC 360 - Spring 2019
* 
* The program is an implementation of the producer consumer problem using semaphores.
*/
#define ITEMS  //the shared resource pool
#define MAX_ITEMS = 10

void producer(void);
void consumer(void);

int main(int argc, int **argv) {
    printf("hii");
    return 0;
}

// producer function purpose is to add items to the shared resource pool
void producer() {
    //should sleep when items is full, notified by consumer to wake up when data removed
}

// consumer function purpose is to remove items from the shared resource pool
void consumer() {
    //should sleep when item is empty, notified by producer to wake up when data added
}


//semaphore: an integer variable, accessed through wait() and signal()
//         : Binary Semaphore: aka mutex lock. values 0 and 1. implements solution of a 
//           critical section problem with multiple processes.
//         : Counting Semaphore: used to control access to a resource that has multiple instances

