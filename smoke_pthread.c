#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define NUM_ITERATIONS 5

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
  bool matchAvail;
  bool paperAvail;
  bool tobacAvail;
  struct Agent* agent;
  bool waitingToSmoke;
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
/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can re-write it if you like, but be sure that all it does
 * is choose 2 random reasources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agentLock (void* pv) {
  // struct Agent* a = av;
  struct ResourcePool* pool = pv;
  struct Agent* a = pool->agent;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};
  
  pthread_mutex_lock(&a->mutex);
  for (int i = 0; i < NUM_ITERATIONS; i++) {
    VERBOSE_PRINT("\nIteration: %d\n----------\n", i);
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
      pthread_cond_signal (&a->paper);
    }

    if (c & TOBACCO) {
      VERBOSE_PRINT ("tobacco available\n");
      pool->tobacAvail = true;
      pthread_cond_signal (&a->tobacco);
    }
    
    numIter++;
    while (pool->waitingToSmoke){
      VERBOSE_PRINT ("agent is waiting for smoker to smoke\n"); 
      pthread_cond_wait (&a->smoke, &a->mutex);
    }
  }
  pthread_mutex_unlock(&a->mutex);
  return NULL;
}

void* smokerLock (void* av) {
  struct Smoker* smoker = av;
  struct ResourcePool* pool = smoker->pool;
  struct Agent* agent = smoker->agent;

  pthread_mutex_lock (&agent->mutex);
  while (1) {

    //while loop runs the same amount of times as the agent
    if (smoker->smokeMethod == MATCH) {
      VERBOSE_PRINT ("match smoker waiting for matches\n");
      pthread_cond_wait(&agent->match, &agent->mutex); //wait until agent decides match is available
      
      if (pool->tobacAvail && pool->paperAvail) {
        //match smoker has all resources needed
        pool->tobacAvail = false;
        pool->paperAvail = false;
        pool->matchAvail = false;
        smoke_count[smoker->smokeMethod]++;
        pool->waitingToSmoke = false;
        VERBOSE_PRINT ("match smoking\n");
        pthread_cond_signal(&agent->smoke); //signal that match is smoking
      }
    
    }else if (smoker->smokeMethod == PAPER) {
      VERBOSE_PRINT ("paper smoker waiting for paper\n"); 
      pthread_cond_wait(&agent->paper, &agent->mutex);
      
      if (pool->matchAvail && pool->tobacAvail) {
        //paper smoker has all resources needed
        pool->matchAvail = false;
        pool->tobacAvail = false;
        pool->paperAvail = false;
        smoke_count[smoker->smokeMethod]++;
        pool->waitingToSmoke = false;
        VERBOSE_PRINT ("paper smoking\n");
        pthread_cond_signal(&agent->smoke); //signal that paper is smoking
      }

    }else if (smoker->smokeMethod == TOBACCO) {
      VERBOSE_PRINT ("tobacco smoker waiting for tobacco\n");
      pthread_cond_wait(&agent->tobacco, &agent->mutex);

      if (pool->matchAvail && pool->paperAvail) {
        //tobacco smoker has all resources needed
        pool->matchAvail = false;
        pool->paperAvail = false;
        pool->tobacAvail = false;
        smoke_count[smoker->smokeMethod]++;
        pool->waitingToSmoke = false;
        VERBOSE_PRINT ("tobacco smoking\n");
        pthread_cond_signal(&agent->smoke); //signal that tobacco is smoking
      }

    }
    if (pool->matchAvail && pool->paperAvail) {
      pthread_cond_signal(&agent->tobacco);
    } else if (pool->matchAvail && pool->tobacAvail) {
      pthread_cond_signal(&agent->paper);
    } else if (pool->paperAvail && pool->tobacAvail) {
      pthread_cond_signal(&agent->match);
    }
  }
  pthread_mutex_unlock(&agent->mutex);
}

int main (int argc, char** argv) {
  
  //all smokers have access to one agent, which holds the mutex and all condition variables
  struct Agent* a = createAgent();

  //all smokers have access to one resource pool
  struct ResourcePool* pool = createResourcePool(a);

  pthread_t agent, match, paper, tobacco;
  pthread_create(&agent, NULL, agentLock, (void*)(pool));
  pthread_create(&match, NULL, smokerLock, createSmoker(MATCH, pool, a));
  pthread_create(&paper, NULL, smokerLock, createSmoker(PAPER, pool, a));
  pthread_create(&tobacco, NULL, smokerLock, createSmoker(TOBACCO, pool, a));

  pthread_join(agent, NULL);
  pthread_join(match, NULL);
  pthread_join(paper, NULL);
  pthread_join(tobacco, NULL);
  
  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}