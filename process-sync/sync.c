/*
 * Matthew Leeds, CS 426, 2015-09-28, OS Concepts Ch.5 Exercises 37 and 38
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
pthread_cond_t resources_positive;
pthread_cond_t resources_under_limit;

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
  pthread_cond_init(&resources_positive, NULL);
  pthread_cond_init(&resources_under_limit, NULL);
  for (i = 0; i < numberOfThreads; i += 2) {
    srand(time(NULL) * (i + 2));
    // spawn a thread to decrease the count a number of times
    tids[i+1] = malloc(sizeof(pthread_t));
    params[i+1] = malloc(sizeof(sleepAndCount));
    params[i+1]->nsecSleep = (long) rand() % 1000000000;
    params[i+1]->numberOfIterations = numberOfIterations;
    pthread_create(tids[i+1], &attr, decrease_count, params[i+1]);

    srand(time(NULL) * (i + 1));
    // spawn a thread to increase the count a number of times
    tids[i] = malloc(sizeof(pthread_t));
    params[i] = malloc(sizeof(sleepAndCount));
    params[i]->nsecSleep = (long) rand() % 1000000000;
    params[i]->numberOfIterations = numberOfIterations;
    pthread_create(tids[i], &attr, increase_count, params[i]);
  }

  // join all the threads and free memory
  for (i = 0; i < numberOfThreads; ++i) {
    pthread_join(*tids[i], NULL);
    free(tids[i]);
    free(params[i]);
  }
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&resources_positive);
  pthread_cond_destroy(&resources_under_limit);

  return 0;
}

// decrement available_resources thread-safely
// waits if insufficient resources are available
// repeat a number of times on timed intervals
void* decrease_count(void* param) {
  struct timespec time;
  time.tv_sec = 0;
  time.tv_nsec = ((sleepAndCount*)param)->nsecSleep;
  int count = ((sleepAndCount*)param)->numberOfIterations;
  int i;
  for (i = 0; i < count; ++i) {
    nanosleep(&time, NULL);
    // use a mutex lock to prevent a race condition on available_resources
    pthread_mutex_lock(&mutex);
    // begin critical section
    if (available_resources == 0) {
      // wait for available_resources to be positive
      pthread_cond_wait(&resources_positive, &mutex);
    }
    available_resources--;
    if (available_resources == MAX_RESOURCES - 1)
      pthread_cond_signal(&resources_under_limit);
    printf("tid = 0x%08x, decrease, available_resources = %d\n",
           (unsigned)pthread_self(), available_resources);
    // end critical section
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(0);
}

// increment available_resources thread-safely
// repeat a number of times on timed intervals
void* increase_count(void* param) {
  struct timespec time;
  time.tv_sec = 0;
  time.tv_nsec = ((sleepAndCount*)param)->nsecSleep;
  int count = ((sleepAndCount*)param)->numberOfIterations;
  int i;
  for (i = 0; i < count; ++i) {
    nanosleep(&time, NULL);
    // use a mutex lock to prevent a race condition on available_resources
    pthread_mutex_lock(&mutex);
    // begin critical section
    if (available_resources == MAX_RESOURCES)
      pthread_cond_wait(&resources_under_limit, &mutex);
    available_resources++;
    if (available_resources > 0)
      pthread_cond_signal(&resources_positive);
    printf("tid = 0x%08x, increase, available_resources = %d\n",
           (unsigned)pthread_self(), available_resources);
    // end critical section
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(0);
}
