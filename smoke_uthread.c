#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

/*
* Meghan Fair
* V00839675
* 
* UVic CSC 360 - Spring 2019
* Assignment 2 - part 2 "The Smokers Problem"
*
* Implementation of the smokers problem using uthreads using
* mutexes and condition variables.
*
*/


#define NUM_ITERATIONS 1000

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

struct Agent {
  uthread_mutex_t mutex;
  uthread_cond_t  match;
  uthread_cond_t  paper;
  uthread_cond_t  tobacco;
  uthread_cond_t  smoke;
};

struct Smoker {
  struct ResourcePool* pool;
  int smokeMethod;
  struct Agent* agent;
};

struct ResourcePool {
  struct Agent* agent;
  bool matchAvail;
  bool paperAvail;
  bool tobacAvail;
  bool waitingToSmoke;
  bool matchWaiting;
  bool tobacWaiting;
  bool paperWaiting;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  agent->mutex   = uthread_mutex_create();
  agent->paper   = uthread_cond_create (agent->mutex);
  agent->match   = uthread_cond_create (agent->mutex);
  agent->tobacco = uthread_cond_create (agent->mutex);
  agent->smoke   = uthread_cond_create (agent->mutex);
  return agent;
}

struct Smoker* createSmoker (int type, struct ResourcePool* pool, struct Agent* a) {
  struct Smoker* smoker = malloc (sizeof(struct Smoker));
  smoker->pool = pool;
  smoker->smokeMethod = type;
  smoker->agent = a;
  return smoker;
}

struct ResourcePool* createResourcePool (struct Agent* a) {
  struct ResourcePool* pool = malloc (sizeof(struct ResourcePool));
  pool->agent = a;
  pool->matchAvail = false;
  pool->paperAvail = false;
  pool->tobacAvail = false;
  pool->waitingToSmoke = true;
  pool->matchWaiting = false;
  pool->paperWaiting = false;
  pool->tobacWaiting = false;
  return pool;
}

/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked
int numIter = 0;

void resetWaitingValues(struct ResourcePool* pool) {
  pool->paperWaiting = false;
  pool->matchWaiting = false;
  pool->tobacWaiting = false;
}

void resetValues (struct ResourcePool* pool) {
  pool->tobacAvail = false;
  pool->paperAvail = false;
  pool->matchAvail = false;
}

/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can re-write it if you like, but be sure that all it does
 * is choose 2 random reasources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agent (void* av) {
  struct ResourcePool* pool = av;
  struct Agent* a = pool->agent;
  
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};
  
  uthread_mutex_lock (a->mutex);

    for (int i = 0; i < NUM_ITERATIONS; i++) {
      numIter = i;
      VERBOSE_PRINT("\nIteration: %d\n----------\n", i);
      resetWaitingValues(pool);    
      
      int r = random() % 3;
      signal_count [matching_smoker [r]] ++;
      int c = choices [r];
     
     
      if (c & MATCH) {
        VERBOSE_PRINT ("match available\n");
        pool->matchAvail = true;
        uthread_cond_signal (a->match);
      }

      if (c & PAPER) {
        VERBOSE_PRINT ("paper available\n");
        pool->paperAvail = true;
        uthread_cond_signal (a->paper);
      }

      if (c & TOBACCO) {
        VERBOSE_PRINT ("tobacco available\n");
        pool->tobacAvail = true;
        uthread_cond_signal (a->tobacco);
      }
      
      VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
      uthread_cond_wait (a->smoke);
    }

  uthread_mutex_unlock (a->mutex);
  return NULL;
}

/*
* The smoker procedure recieves signals from the agent that resources
* are available. If two resources are available, and the smoker with the 
* third resource is called, then that smoker gets to smoke while the other
* two smokers and the agent wait.
* Once done smoker, the smoker signals the agent that he/she can choose
* the next set of resources.
*/
void* smoker (void* pv) {
  struct Smoker* smoker = pv;
  struct ResourcePool* pool = smoker->pool;
  struct Agent* agent = smoker->agent;

  uthread_mutex_lock(agent->mutex);
  
  while(1) {

    if (smoker->smokeMethod == MATCH) {

      if (pool->tobacAvail && pool->paperAvail) {
        //match smoker has all resources needed
        VERBOSE_PRINT ("Match can smoke!\n");
        resetValues(pool);
        smoke_count[smoker->smokeMethod]++;
        pool->waitingToSmoke = false;
        VERBOSE_PRINT ("match smoking\n");
        uthread_cond_signal(agent->smoke);
      
      } else {
        VERBOSE_PRINT ("match smoker waiting\n");
        pool->matchWaiting = true;
        uthread_cond_wait(agent->match);
      }

    } else if (smoker->smokeMethod == PAPER) {

      if (pool->matchAvail && pool->tobacAvail) {
        //paper smoker has all resources needed
        VERBOSE_PRINT ("paper can smoke!\n");
        resetValues(pool);
        smoke_count[smoker->smokeMethod]++;
        pool->waitingToSmoke = false;
        VERBOSE_PRINT ("paper smoking\n");
        uthread_cond_signal(agent->smoke);
      
      } else {
        VERBOSE_PRINT ("paper smoker waiting\n"); 
        pool->paperWaiting = true;
        uthread_cond_wait(agent->paper);
      }

    } else if (smoker->smokeMethod == TOBACCO) {
      
      if (pool->matchAvail && pool->paperAvail) {
        //tobacco smoker has all resources needed
        VERBOSE_PRINT ("Tobacco can smoke!\n");
        resetValues(pool);
        smoke_count[smoker->smokeMethod]++;
        pool->waitingToSmoke = false;
        VERBOSE_PRINT ("tobacco smoking\n");
        uthread_cond_signal(agent->smoke);

      } else {
        VERBOSE_PRINT ("tobacco smoker waiting\n");
        pool->tobacWaiting = true;
        uthread_cond_wait(agent->tobacco);
      }
    }

    if (pool->matchWaiting && pool->paperWaiting && pool->tobacWaiting && (numIter == NUM_ITERATIONS)) {
      //end of iterations,return to agent thread who returns to main
      break;
    
    } else {
      if (pool->matchAvail && pool->paperAvail) {
        VERBOSE_PRINT("signalling tobacco\n");
        uthread_cond_signal(agent->tobacco);

      } else if (pool->matchAvail && pool->tobacAvail) {
        VERBOSE_PRINT ("signalling paper\n");
        uthread_cond_signal(agent->paper);
        
      } else if (pool->paperAvail && pool->tobacAvail) {
        VERBOSE_PRINT ("signalling match\n");
        uthread_cond_signal(agent->match);
      }
    }
  }

  uthread_mutex_unlock(agent->mutex);
}

int main (int argc, char** argv) {
  uthread_init (7);
  //all smokers have access to one agent, which holds the mutex and all condition variables
  struct Agent*  a = createAgent();

  //all smokers have access to one resource pool
  struct ResourcePool* p = createResourcePool(a);

  VERBOSE_PRINT ("creating threads\n");
  uthread_t tobac = uthread_create(smoker, createSmoker(TOBACCO, p, a));
  uthread_t paper = uthread_create(smoker, createSmoker(PAPER, p, a));
  uthread_t match = uthread_create(smoker, createSmoker(MATCH, p, a));

  VERBOSE_PRINT ("joining threads\n");
  uthread_join (uthread_create (agent, p), 0);
  //We only need to join the agent thread because the smoker threads
  //are dependent on the agent thread. Once the agent thread terminates,
  //the smoker threads will terminate too.

  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}