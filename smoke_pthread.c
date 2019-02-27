#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define NUM_ITERATIONS 2

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

typedef struct Agent {
  pthread_mutex_t agentMutex;
  pthread_cond_t  match;
  pthread_cond_t  paper;
  pthread_cond_t  tobacco;
  pthread_cond_t  smokersDone;
} Agent;

Agent *createAgent() {
  Agent *agent = malloc (sizeof(Agent));
  pthread_mutex_init(&agent->agentMutex, NULL);
  pthread_cond_init(&agent->match, NULL); //for when agent chooses match
  pthread_cond_init(&agent->paper, NULL); //for when agent chooses paper
  pthread_cond_init(&agent->tobacco, NULL); //for when agent chooses tobacco
  pthread_cond_init(&agent->smokersDone, NULL); //when smokers done smoking
  return agent;
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
bool haveMatch = false;
bool havePaper = false;
bool haveTobac = false;
Agent*  a;
pthread_mutex_t smokerMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  matchAvail  = PTHREAD_COND_INITIALIZER;
pthread_cond_t  paperAvail  = PTHREAD_COND_INITIALIZER;
pthread_cond_t  tobacAvail  = PTHREAD_COND_INITIALIZER;

/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can re-write it if you like, but be sure that all it does
 * is choose 2 random reasources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agentFunc (void* av) {
  Agent* a = av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};
  
  for (int i = 0; i < NUM_ITERATIONS; i++) {
    pthread_mutex_lock(&a->agentMutex);
    int r = random() % 3;
    signal_count [matching_smoker [r]] ++;
    int c = choices [r];

    if (c & MATCH) {
      VERBOSE_PRINT ("match available\n");
      haveMatch = true;
      pthread_cond_signal (&matchAvail);
    }

    if (c & PAPER) {
      VERBOSE_PRINT ("paper available\n");
      havePaper = true;
      pthread_cond_signal (&paperAvail);
    }

    if (c & TOBACCO) {
      VERBOSE_PRINT ("tobacco available\n");
      haveTobac = true;
      pthread_cond_signal (&tobacAvail);
    }
    
    VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
    pthread_cond_wait (&a->smokersDone, &a->agentMutex);
    numIter = i;
    pthread_mutex_unlock(&a->agentMutex);
  }
  return NULL;
}

void* matchFunc (void* av) {
  Agent* a = av;
  while (numIter < NUM_ITERATIONS) {
    
    pthread_mutex_lock(&smokerMutex);
    
    while (!haveMatch) {
      pthread_cond_wait(&tobacAvail, &smokerMutex);
    }

    if (havePaper) {
      havePaper = false;
      VERBOSE_PRINT("call the match smoker\n");
      pthread_cond_signal(&paperAvail);
    }
    
    if (haveTobac) {
      haveTobac = false;
      VERBOSE_PRINT("call the tobacco smoker\n");
      pthread_cond_signal(&tobacAvail);
    }

    pthread_mutex_unlock(&smokerMutex);
  }
}

int main (int argc, char** argv) {
  a = createAgent();

  pthread_t agent, match;
  // , paper, tobacco; 

  pthread_create(&agent, NULL, agentFunc, (void*)&a);
  pthread_create(&match, NULL, matchFunc, (void*)&a);
  // pthread_create(&paper, NULL, smokerLock, (void*)&paperAvail);
  // pthread_create(&tobacco, NULL, smokerLock, (void*)&tobacAvailco);

  pthread_join (agent, NULL);
  pthread_join (match, NULL);
  // pthread_join (paper, NULL);
  // pthread_join (tobacco, NULL);

  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}