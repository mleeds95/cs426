/*
 * Matthew Leeds, CS 426, 2015-09-23, OS Concepts Ch.5 Exercise 37
 * This program manages a shared resource across processes without
 * a race condition using a semaphore. Pass it the number of threads
 * to create and the number of iterations each thread will execute
 * on the command line. Half the threads will try to increase the
 * global available_resources, half will try to decrease. 
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>

#define MAX_RESOURCES 5
int available_resources = MAX_RESOURCES;
pthread_mutex_t mutex;

typedef struct {
  long nsecSleep;
  int numberOfIterations;
} sleepAndCount;

void* increase_count(void* param);
void* decrease_count(void* param);

int main(int argc, char** argv) {
  // input validation
  if (argc != 3) {
    printf("Usage: ./sync <number of threads> <number of iterations>\n");
    return 1;
  }
  int numberOfThreads = atoi(argv[1]);
  int numberOfIterations = atoi(argv[2]);
  if (numberOfThreads == 0 || numberOfIterations == 0) {
    printf("Error: number of threads and number of iterations must be positive integers\n");
    return 2;
  }
  if (numberOfThreads % 2 == 1) numberOfThreads--;

  // create a number of threads to increase and decrease the count
  int i;
  sleepAndCount* params[numberOfThreads];
  pthread_t* tids[numberOfThreads];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_mutex_init(&mutex, NULL);
  for (i = 0; i < numberOfThreads; i += 2) {
    // spawn a thread to increase the count a number of times
    tids[i] = malloc(sizeof(pthread_t));
    params[i] = malloc(sizeof(sleepAndCount));
    params[i]->nsecSleep = (long) rand() % 1000000000;
    params[i]->numberOfIterations = numberOfIterations;
    pthread_create(tids[i], &attr, increase_count, params[i]);

    // spawn a thread to decrease the count a number of times
    tids[i+1] = malloc(sizeof(pthread_t));
    params[i+1] = malloc(sizeof(sleepAndCount));
    params[i+1]->nsecSleep = (long) rand() % 1000000000;
    params[i+1]->numberOfIterations = numberOfIterations;
    pthread_create(tids[i+1], &attr, decrease_count, params[i+1]);
  }

  // join all the threads and free the pthread_t memory
  int** ret = malloc(sizeof(int*));
  for (i = 0; i < numberOfThreads; ++i) {
    pthread_join(*tids[i], (void**)ret);
    free(*ret);
    free(tids[i]);
    free(params[i]);
  }
  free(ret);

  return 0;
}

// returns -1 if insufficient resources are available
// otherwise decrease available_resources by count and return 0
void* decrease_count(void* param) {
  struct timespec time;
  time.tv_sec = 0;
  time.tv_nsec = ((sleepAndCount*)param)->nsecSleep;
  int count = ((sleepAndCount*)param)->numberOfIterations;
  int i;
  int* ret = malloc(sizeof(int));
  *ret = -1;
  for (i = 0; i < count; ++i) {
    nanosleep(&time, NULL);
    pthread_mutex_lock(&mutex); // acquire lock
    // begin critical section
    // use a mutex lock to prevent a race condition on available_resources
    if (available_resources == 0)
      pthread_exit(ret);
    else
      available_resources -= count;
    printf("tid = %d, decrease, available_resources = %d\n",
           (unsigned)pthread_self(), available_resources);
    // end critical section
    pthread_mutex_unlock(&mutex); // release lock
  }
  free(ret);
  pthread_exit(0);
}

// increase available_resources by count
void* increase_count(void* param) {
  struct timespec time;
  time.tv_sec = 0;
  time.tv_nsec = ((sleepAndCount*)param)->nsecSleep;
  int count = ((sleepAndCount*)param)->numberOfIterations;
  int i;
  for (i = 0; i < count; ++i) {
    nanosleep(&time, NULL);
    pthread_mutex_lock(&mutex); // acquire lock
    // begin critical section
    // use a mutex lock to prevent a race condition on available_resources
    available_resources += count;
    printf("tid = %d, increase, available_resources = %d\n",
           (unsigned)pthread_self(), available_resources);
    // end critical section
    pthread_mutex_unlock(&mutex); // release lock
  }
  pthread_exit(0);
}
