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
  pthread_cond_t  smokersDone;
  bool runAgent;
} Agent;

Agent *createAgent() {
  Agent *agent = malloc (sizeof(Agent));
  pthread_mutex_init(&agent->agentMutex, NULL);
  pthread_cond_init(&agent->smokersDone, NULL); //when smokers done smoking
  agent->runAgent = true;  
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

int numIter = 0; //finish program when equal to NUM_ITERATIONS

//Resource conditionals---------------
bool haveMatch = false;
bool havePaper = false;
bool haveTobac = false;
//------------------------------------

//Actor conditionals------------------
bool matchWillSmoke = false;
bool paperWillSmoke = false;
bool tobacWillSmoke = false;
//------------------------------------

Agent*  a;
pthread_mutex_t smokerMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  matchAvail  = PTHREAD_COND_INITIALIZER;
pthread_cond_t  paperAvail  = PTHREAD_COND_INITIALIZER;
pthread_cond_t  tobacAvail  = PTHREAD_COND_INITIALIZER;

pthread_cond_t  tobacSmoker = PTHREAD_COND_INITIALIZER;
pthread_cond_t  matchSmoker = PTHREAD_COND_INITIALIZER;
pthread_cond_t  paperSmoker = PTHREAD_COND_INITIALIZER;


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
    
    while (!&a->runAgent) {
      //wait until smokers done
      pthread_cond_wait(&a->smokersDone, &a->agentMutex);
    }
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
    a->runAgent = false;
    pthread_mutex_unlock(&a->agentMutex);
  }
  return NULL;
}

void* matchFunc (void* av) {
  while (numIter < NUM_ITERATIONS) {
    
    pthread_mutex_lock(&smokerMutex);
    
    while (!haveMatch) {
      //wait until agent makes match available
      pthread_cond_wait(&matchAvail, &smokerMutex);
    }

    if (havePaper) {
      //if paper and match available
      VERBOSE_PRINT("call the tobacco smoker\n");
      tobacWillSmoke = true;
      pthread_cond_signal(&tobacSmoker);
    }
    
    if (haveTobac) {
      //if tobacco and match available
      VERBOSE_PRINT("call the paper smoker\n");
      paperWillSmoke = true;
      pthread_cond_signal(&paperSmoker);
    }

    pthread_mutex_unlock(&smokerMutex);
  }
  return 0;
}

void *paperFunc (void* av) {
  while (numIter < NUM_ITERATIONS) {
    pthread_mutex_lock(&smokerMutex);
    
    while (!havePaper) {
      //wait until agent makes paper available
      pthread_cond_wait(&paperAvail, &smokerMutex);
    }
    if (haveMatch) {
      //if paper and match available
      VERBOSE_PRINT("call the tobacco smoker\n");
      tobacWillSmoke = true;
      pthread_cond_signal(&tobacSmoker);
    }
    if (haveTobac) {
      //if paper and tobacco available
      VERBOSE_PRINT("call the match smoker\n");
      matchWillSmoke = true;
      pthread_cond_signal(&matchSmoker);
    }

    pthread_mutex_unlock(&smokerMutex);

  }
  return 0;
}

void* tobacFunc (void* av) {
  while (numIter < NUM_ITERATIONS) {
    pthread_mutex_lock(&smokerMutex);
    
    while (!haveTobac) {
      //wait until agent makes tobacco available
      pthread_cond_wait(&tobacAvail, &smokerMutex);
    }

    if (haveMatch) {
      //if tobacco and match available
      VERBOSE_PRINT("call the paper smoker\n");
      paperWillSmoke = true;
      pthread_cond_signal(&paperSmoker);
    }

    if (havePaper) {
      //if tobacco and paper available
      VERBOSE_PRINT("call the match smoker\n");
      matchWillSmoke = true;
      pthread_cond_signal(&matchSmoker);
    }

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
  while (numIter < NUM_ITERATIONS) {
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

  pthread_create(&agent, NULL, agentFunc, (void*)&a);
  pthread_create(&match, NULL, matchFunc, NULL);
  pthread_create(&paper, NULL, paperFunc, NULL);
  pthread_create(&tobacco, NULL, tobacFunc, NULL);

  pthread_join (agent, NULL);
  pthread_join (match, NULL);
  pthread_join (paper, NULL);
  pthread_join (tobacco, NULL);

  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}