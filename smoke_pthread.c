#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_ITERATIONS 1000

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

typedef struct Agent{
  pthread_mutex_t mutex;
  pthread_cond_t  match;
  pthread_cond_t  paper;
  pthread_cond_t  tobacco;
  pthread_cond_t  smoke;
} Agent;

typedef struct Smoker{
  int paper;
  int tobacco;
  int match;
  int smokeMethod;
  Agent *agent;
} Smoker;

Agent *createAgent() {
  Agent *agent = malloc (sizeof(Agent));
  pthread_mutex_init(&agent->mutex, NULL);
  pthread_cond_init(&agent->match, NULL);
  pthread_cond_init(&agent->paper, NULL);
  pthread_cond_init(&agent->tobacco, NULL);
  pthread_cond_init(&agent->smoke, NULL);
  return agent;
}

Smoker *createSmoker(Agent *agent, int type) {
  Smoker *smoker = malloc (sizeof(Smoker));
  smoker->paper = 0;
  smoker->tobacco = 0;
  smoker->match = 0;
  smoker->agent = agent;
  smoker->smokeMethod = type;
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

/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can re-write it if you like, but be sure that all it does
 * is choose 2 random reasources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agentLock (void* av) {
  Agent* a = av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};
  
  pthread_mutex_init(&a->mutex, NULL);

  for (int i = 0; i < NUM_ITERATIONS; i++) {
    int r = random() % 3;
    signal_count [matching_smoker [r]] ++;
    int c = choices [r];

    if (c & MATCH) {
      VERBOSE_PRINT ("match available\n");
      pthread_cond_signal (&a->match);
    }

    if (c & PAPER) {
      VERBOSE_PRINT ("paper available\n");
      pthread_cond_signal (&a->paper);
    }

    if (c & TOBACCO) {
      VERBOSE_PRINT ("tobacco available\n");
      pthread_cond_signal (&a->tobacco);
    }
    
    VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
    pthread_cond_wait (&a->smoke, &a->mutex);
  }
  pthread_mutex_unlock(&a->mutex);
  return NULL;
}

void* smokerLock (void* av) {
  printf("inside smokerlock\n");
  Smoker* s = av;
  Agent *a = s->agent;
  printf("locking mutex in smokerlock by %d\n", s->smokeMethod);
  pthread_mutex_lock(&a->mutex);
  for (int i = 0; i < NUM_ITERATIONS; i++) {
    if (s->smokeMethod == PAPER) {
      pthread_cond_wait(&a->paper,&a->mutex);
      if (s->match > 0 && s->tobacco > 0) {
        s->match--;
        s->tobacco--;
        smoke_count[s->smokeMethod]++;
        pthread_cond_signal(&a->smoke);
      } else {
        s->paper++;
      }
    } else if (s->smokeMethod == MATCH) {
      pthread_cond_wait(&a->match,&a->mutex);
      if (s->paper > 0 && s->tobacco > 0) {
        s->tobacco--;
        s->paper--;
        smoke_count[s->smokeMethod]++;
        pthread_cond_signal(&a->smoke);
      } else {
        s->match++;
      }
    } else if (s->smokeMethod == TOBACCO) {
      pthread_cond_wait(&a->tobacco,&a->mutex);
      if (s->paper > 0 && s->match > 0) {
        s->match--;
        s->paper--;
        smoke_count[s->smokeMethod]++;
        pthread_cond_signal(&a->smoke);
      } else {
        s->tobacco++;
      }
    }
    if (s->paper > 0 && s->tobacco) {
      pthread_cond_signal(&a->match);
    } else if (s->paper > 0 && s->match > 0) {
      pthread_cond_signal(&a->tobacco);
    } else if (s->tobacco > 0 && s->match > 0) {
      pthread_cond_signal(&a->paper);
    }
    pthread_mutex_unlock(&a->mutex);
  }
  return NULL;
}

int main (int argc, char** argv) {
  Agent*  a = createAgent();
  
  pthread_t agent, match, paper, tobacco; 
  printf("creating smokers\n");
  Smoker* smokerMatch = createSmoker(a, MATCH);
  Smoker* smokerPaper = createSmoker(a, PAPER);
  Smoker* smokerTobacco = createSmoker(a, TOBACCO);
  printf("creating pthreads\n");

  pthread_create(&agent, NULL, agentLock, (void*)&a);
  pthread_create(&match, NULL, smokerLock, (void*)&smokerMatch);
  pthread_create(&paper, NULL, smokerLock, (void*)&smokerPaper);
  pthread_create(&tobacco, NULL, smokerLock, (void*)&smokerTobacco);

  void *joinThreads;
  pthread_join (agent, &joinThreads);
  pthread_join (match, &joinThreads);
  pthread_join (paper, &joinThreads);
  pthread_join (tobacco, &joinThreads);

  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}