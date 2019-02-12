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
int ITEMS;  
sem_t sem; //the shared resource pool -- generalized semaphore
#define MAX_ITEMS = 10
#define MIN_ITEMS = 0

void producer(void);
void consumer(void);

int main(int argc, int **argv) {
    sem_init(&sem, 0, 0);  //initialize generalized semaphore at 0
    //pthread()   //initialize 2 producer threads
    //pthread()   //initialize 2 consumer threads
    sem_destroy(&sem);  //destory semaphore before exiting program
    printf("Semaphore Program Done\n");
    return 0;
}

// producer function purpose is to add items to the shared resource pool
void producer() {
    //should sleep when items is full, notified by consumer to wake up when data removed
    


    // do (
    //    // produce an item

    //     wait(mutex);

    //     //place in buffer

    //     signal(mutex);
    //     signal(full);
    // ) while (true);

}

// consumer function purpose is to remove items from the shared resource pool
void consumer() {
    //should sleep when item is empty, notified by producer to wake up when data added

    // do{

    //    wait(full);
    //    wait(mutex);

    // // remove item from buffer

    //    signal(mutex);
    //    signal(empty);

    // // consumes item

    // } while(true);
}


//semaphore: an integer variable, accessed through wait() and signal()
//         : Binary Semaphore: aka mutex lock. values 0 and 1. implements solution of a 
//           critical section problem with multiple processes.
//         : Counting Semaphore: used to control access to a resource that has multiple instances

