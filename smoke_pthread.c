#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

/*
* Meghan Fair
* V00839675
* 
* UVic CSC 360 - Spring 2019
* Assignment 2 - part 2 "The Smokers Problem"
* 
* Implementation of the smokers problem using pthreads using
* mutexes and condition variables.
*
*/


#define NUM_ITERATIONS 2

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

struct Agent{
  pthread_mutex_t mutex;
  pthread_cond_t  match;
  pthread_cond_t  paper;
  pthread_cond_t  tobacco;
  pthread_cond_t  smoke;
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

struct Smoker{
  struct ResourcePool* pool;
  int smokeMethod;
  struct Agent* agent;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof(struct Agent));
  pthread_mutex_init(&agent->mutex, NULL);
  pthread_cond_init(&agent->match, NULL);
  pthread_cond_init(&agent->paper, NULL);
  pthread_cond_init(&agent->tobacco, NULL);
  pthread_cond_init(&agent->smoke, NULL);
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
  pool->matchAvail = false;
  pool->paperAvail = false;
  pool->tobacAvail = false;
  pool->agent = a;
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
void* agentLock (void* pv) {
  struct ResourcePool* pool = pv;
  struct Agent* a = pool->agent;

  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};
  
  pthread_mutex_lock(&a->mutex);

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
      pthread_cond_signal (&a->match);
    }

    if (c & PAPER) {
      VERBOSE_PRINT ("paper available\n");
      pool->paperAvail = true;
    if (c & TOBACCO) {
      VERBOSE_PRINT ("tobacco available\n");
      haveTobac = true;
      pthread_cond_signal (&tobacAvail);
    }
    
    VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
    pthread_cond_wait (&a->smokersDone, &a->agentMutex);
    numIter = i;
    a->runAgent = false;
    VERBOSE_PRINT("unlocking agent mutex\n");

  }
  pthread_mutex_unlock(&a->agentMutex);
  return NULL;
}

void* matchFunc (void* av) {
  VERBOSE_PRINT("inside matchfunc\n");
  while (numIter < NUM_ITERATIONS) {
    VERBOSE_PRINT("mathcfunc locking smoker mutex\n");
    
    pthread_mutex_lock(&smokerMutex);
    
    while (haveMatch == false) {
      //wait until agent makes match available
      VERBOSE_PRINT("waiting for matches to be made available\n");
      pthread_cond_wait(&matchAvail, &smokerMutex);
    }

    if (havePaper) {
      //if paper and match available
      VERBOSE_PRINT("call the tobacco smoker\n");
      havePaper = false;
      tobacWillSmoke = true;
      pthread_cond_signal(&tobacSmoker);
    }
    
    if (haveTobac) {
      //if tobacco and match available
      VERBOSE_PRINT("call the paper smoker\n");
      haveTobac = false;
      paperWillSmoke = true;
      pthread_cond_signal(&paperSmoker);
    }
    VERBOSE_PRINT("matchfunc unlocking smoker mutex\n");

    pthread_mutex_unlock(&smokerMutex);
  }
  return 0;
}

void *paperFunc (void* av) {
  VERBOSE_PRINT("inside paperfunc\n");

  while (numIter < NUM_ITERATIONS) {
    VERBOSE_PRINT("paperfunc locking smoker mutex\n");

    pthread_mutex_lock(&smokerMutex);
    
    while (havePaper == false) {
      //wait until agent makes paper available
      VERBOSE_PRINT("waiting for paper\n");
      pthread_cond_wait(&paperAvail, &smokerMutex);

    }
    if (haveMatch) {
      //if paper and match available
      VERBOSE_PRINT("call the tobacco smoker\n");
      haveMatch = false;
      tobacWillSmoke = true;
      pthread_cond_signal(&tobacSmoker);
    }
    if (haveTobac) {
      //if paper and tobacco available
      VERBOSE_PRINT("call the match smoker\n");
      haveTobac = false;
      matchWillSmoke = true;
      pthread_cond_signal(&matchSmoker);
    }
    VERBOSE_PRINT("paperfunc unlocking smoker mutex\n");

    pthread_mutex_unlock(&smokerMutex);

  }
  return 0;
}

void* tobacFunc (void* av) {
  VERBOSE_PRINT("inside tobacco func\n");

  while (numIter < NUM_ITERATIONS) {
    VERBOSE_PRINT("tobacfunc locking smoker mutex\n");
    pthread_mutex_lock(&smokerMutex);
    
    while (haveTobac == false) {
      //wait until agent makes tobacco available
      VERBOSE_PRINT("waiting for tobacco\n");
      pthread_cond_wait(&tobacAvail, &smokerMutex);  //DEADLOCK HERE
    }

    if (haveMatch) {
      //if tobacco and match available
      VERBOSE_PRINT("call the paper smoker\n");
      haveMatch = false;
      paperWillSmoke = true;
      pthread_cond_signal(&paperSmoker);
    }

    if (havePaper) {
      //if tobacco and paper available
      VERBOSE_PRINT("call the match smoker\n");
      havePaper = false;
      matchWillSmoke = true;
      pthread_cond_signal(&matchSmoker);
    }
    VERBOSE_PRINT("tobacfunc unlocking smoker mutex\n");

    pthread_mutex_unlock(&smokerMutex);

  }
  return 0;
}

void resetBools () {
  haveMatch = false;
  havePaper = false;
  haveTobac = false;
  a->runAgent = true;
}

void* smokeTobacco (void* av) {
  VERBOSE_PRINT("inside smokeTobaccon\n");
  while (numIter < NUM_ITERATIONS) {
    VERBOSE_PRINT("locking smoker mutex for tobacco to smoke\n");
    pthread_mutex_lock(&smokerMutex);

    while (!tobacWillSmoke) {
      //wait until tobacco smoker able to smoke
      pthread_cond_wait(&tobacSmoker, &smokerMutex);
    }

    VERBOSE_PRINT("Tobacco smoker smoking\n");
    //INCREMENT SMOKECOUNT ARRAY HERE
    resetBools();
    pthread_cond_signal(&a->smokersDone);
    pthread_mutex_unlock(&smokerMutex);
  }
  return 0;
}

void* smokeMatch (void* av) {
  while (numIter < NUM_ITERATIONS) {
    pthread_mutex_lock(&smokerMutex);

    while (!matchWillSmoke) {
      //wait until match smoker able to smoke
      pthread_cond_wait(&matchSmoker, &smokerMutex);
    }

    VERBOSE_PRINT("Match smoker smoking\n");
    //INCREMENT SMOKECOUNT ARRAY HERE
    resetBools();
    pthread_cond_signal(&a->smokersDone);
    pthread_mutex_unlock(&smokerMutex);
  }
}

void* smokePaper (void* av) {
  while (numIter < NUM_ITERATIONS) {
    pthread_mutex_lock(&smokerMutex);

    while (!paperWillSmoke) {
      //wait until paper smoker able to smoke
      pthread_cond_wait(&paperSmoker, &smokerMutex);
    }

    VERBOSE_PRINT("Paper smoker smoking\n");
    //INCREMENT SMOKECOUNT ARRAY HERE
    resetBools();
    pthread_cond_signal(&a->smokersDone);
    pthread_mutex_unlock(&smokerMutex);
  }
}

int main (int argc, char** argv) {
  a = createAgent();

  pthread_t agent, match, paper, tobacco;
  VERBOSE_PRINT("creating pthreads\n");
  pthread_create(&agent, NULL, agentFunc, (void*)&a);
  pthread_create(&match, NULL, matchFunc, NULL);
  pthread_create(&paper, NULL, paperFunc, NULL);
  pthread_create(&tobacco, NULL, tobacFunc, NULL);
  VERBOSE_PRINT("joining threads\n");
  void* joinThreads;
  pthread_join (agent, &joinThreads);
  VERBOSE_PRINT("agent joined\n");
  pthread_join (match, &joinThreads);
  VERBOSE_PRINT("match joined\n");

  pthread_join (paper, &joinThreads);
  VERBOSE_PRINT("paper joined\n");

  pthread_join (tobacco, &joinThreads);
  VERBOSE_PRINT("tobacco joined\n");

=======
      pool->tobacAvail = true;
      pthread_cond_signal (&a->tobacco);
    }
    
    VERBOSE_PRINT ("agent is waiting for smoker to smoke\n"); 
    pthread_cond_wait (&a->smoke, &a->mutex);
  }

  pthread_mutex_unlock(&a->mutex);
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
void* smokerLock (void* av) {
  struct Smoker* smoker = av;
  struct ResourcePool* pool = smoker->pool;
  struct Agent* agent = smoker->agent;

  pthread_mutex_lock (&agent->mutex);

  while (1) {
    if (smoker->smokeMethod == MATCH) {
      
      if (pool->tobacAvail && pool->paperAvail) {
        //match smoker has all resources needed
        VERBOSE_PRINT ("Match can smoke!\n");
        resetValues(pool);
        smoke_count[smoker->smokeMethod]++;
        pool->waitingToSmoke = false;
        VERBOSE_PRINT ("match smoking\n");
        pthread_cond_signal(&agent->smoke); //signal that match is smoking
      
      } else {
        VERBOSE_PRINT ("match smoker waiting\n");
        pool->matchWaiting = true;
        pthread_cond_wait(&agent->match, &agent->mutex); //wait until agent decides match is available
      }
    
    }else if (smoker->smokeMethod == PAPER) {
      
      if (pool->matchAvail && pool->tobacAvail) {
        //paper smoker has all resources needed
        VERBOSE_PRINT ("paper can smoke!\n");
        resetValues(pool);
        smoke_count[smoker->smokeMethod]++;
        pool->waitingToSmoke = false;
        VERBOSE_PRINT ("paper smoking\n");
        pthread_cond_signal(&agent->smoke); //signal that paper is smoking
     
      } else {
        VERBOSE_PRINT ("paper smoker waiting\n"); 
        pool->paperWaiting = true;
        pthread_cond_wait(&agent->paper, &agent->mutex);
      }

    }else if (smoker->smokeMethod == TOBACCO) {

      if (pool->matchAvail && pool->paperAvail) {
        //tobacco smoker has all resources needed
        VERBOSE_PRINT ("Tobacco can smoke!\n");
        resetValues(pool);
        smoke_count[smoker->smokeMethod]++;
        pool->waitingToSmoke = false;
        VERBOSE_PRINT ("tobacco smoking\n");
        pthread_cond_signal(&agent->smoke); //signal that tobacco is smoking
      
      } else {
        VERBOSE_PRINT ("tobacco smoker waiting\n");
        pool->tobacWaiting = true;
        pthread_cond_wait(&agent->tobacco, &agent->mutex);
      }

    }

    if (pool->matchWaiting && pool->paperWaiting && pool->tobacWaiting && (numIter == NUM_ITERATIONS)) {
      //end of iterations, return to agent thread who should return null
      break;
    
    } else {
      if (pool->matchAvail && pool->paperAvail) {
        VERBOSE_PRINT("signalling tobacco\n");
        pthread_cond_signal(&agent->tobacco);

      } else if (pool->matchAvail && pool->tobacAvail) {
        VERBOSE_PRINT ("signalling paper\n");
        pthread_cond_signal(&agent->paper);
        
      } else if (pool->paperAvail && pool->tobacAvail) {
        VERBOSE_PRINT ("signalling match\n");
        pthread_cond_signal(&agent->match);
      }
    }
  }

  pthread_mutex_unlock(&agent->mutex);
}

int main (int argc, char** argv) {
  //all smokers have access to one agent, which holds the mutex and all condition variables
  struct Agent* a = createAgent();

  //all smokers have access to one resource pool
  struct ResourcePool* pool = createResourcePool(a);

  VERBOSE_PRINT ("creating threads\n");
  pthread_t agent, match, paper, tobacco;
  pthread_create(&agent, NULL, agentLock, (void*)(pool));
  pthread_create(&match, NULL, smokerLock, createSmoker(MATCH, pool, a));
  pthread_create(&paper, NULL, smokerLock, createSmoker(PAPER, pool, a));
  pthread_create(&tobacco, NULL, smokerLock, createSmoker(TOBACCO, pool, a));

  VERBOSE_PRINT ("joining threads\n");
  pthread_join(agent, NULL);
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