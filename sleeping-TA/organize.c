/*
 * Matthew Leeds, CS 426, 2015-09-28, OS Concepts Ch.5 Project 1
 * This program uses process synchronization mechanisms available
 * in pthreads to coordinate students getting help from a TA who
 * sleeps when they're not busy.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <semaphore.h>

// how many times students alternate between programming and seeking help
#define NUM_ITERATIONS 10
pthread_mutex_t mutex;
sem_t sem;
int number_waiting;
int go_home;

// this will be passed to each new student thread
typedef struct {
  long nsecSleep;
  int numberOfIterations;
} sleepAndCount;

// to be run by their respective threads
void* simulate_student(void* param);
void* simulate_TA(void* param);

int main(int argc, char** argv) {
  // input validation
  if (argc != 2) {
    printf("Usage: ./organize <number of students>\n");
    return 1;
  }
  int numberOfStudents = atoi(argv[1]);
  if (numberOfStudents <= 0) {
    printf("Error: Number of students must be a positive integer.\n");
    return 2;
  }
  
  // initialize the semaphore
  sem_init(&sem, 0, 0);
  number_waiting = 0;
  go_home = 0;

  // make a thread for the TA
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_mutex_init(&mutex, NULL);
  pthread_t tid_TA;
  pthread_create(&tid_TA, &attr, simulate_TA, NULL);

  // make a thread for each student
  pthread_t* tid_students[numberOfStudents];
  sleepAndCount* params[numberOfStudents];
  int i;
  for (i = 0; i < numberOfStudents; ++i) {
    srand(time(NULL) + i);
    tid_students[i] = malloc(sizeof(pthread_t));
    params[i] = malloc(sizeof(sleepAndCount));
    params[i]->nsecSleep = (long) rand() % 1000000000;
    params[i]->numberOfIterations = NUM_ITERATIONS;
    pthread_create(tid_students[i], &attr, simulate_student, params[i]);
  }
  
  // wait for all the students to get help
  while (number_waiting != 0) ;

  // join the threads and free memory
  for (i = 0; i < numberOfStudents; ++i) {
    pthread_join(*tid_students[i], NULL);
    free(tid_students[i]);
    free(params[i]);
  }
  go_home = 1; // tell the TA to go home
  pthread_join(tid_TA, NULL);
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&mutex);
  sem_destroy(&sem);

  return 0;
}

void* simulate_TA(void* param) {
  srand(time(NULL) / 2);
  struct timespec time;
  time.tv_sec = 0;
  time.tv_nsec = (long) rand() % 1000000000;
  while (!go_home) {
    if (number_waiting == 0) {
      printf("tid = 0x%08x, TA sleeping, number_waiting = %d\n",
             (unsigned)pthread_self(), number_waiting);
      sem_wait(&sem); // sleep
    }
    // help for a time
    printf("tid = 0x%08x, TA helping student, number_waiting = %d\n",
           (unsigned)pthread_self(), number_waiting);
    nanosleep(&time, NULL);
    pthread_mutex_lock(&mutex);
    number_waiting--;
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(0);
}

void* simulate_student(void* param) {
  struct timespec time;
  time.tv_sec = 0;
  time.tv_nsec = ((sleepAndCount*)param)->nsecSleep;
  int count = ((sleepAndCount*)param)->numberOfIterations;
  int i;
  for (i = 0; i < count; ++i) {
    // program for a time
    printf("tid = 0x%08x, student programming, number_waiting = %d\n",
           (unsigned)pthread_self(), number_waiting);
    nanosleep(&time, NULL);
    // if all the chairs are full, keep programming
    while (number_waiting == 3) {
      printf("tid = 0x%08x, student waiting for chair, number_waiting = %d\n",
             (unsigned)pthread_self(), number_waiting);
      nanosleep(&time, NULL);
    }
    pthread_mutex_lock(&mutex);
    number_waiting++;
    pthread_mutex_unlock(&mutex);
    // wait in line or wake up the TA if necessary
    printf("tid = 0x%08x, student waiting for TA, number_waiting = %d\n",
           (unsigned)pthread_self(), number_waiting);
    sem_post(&sem);
  }
  pthread_exit(0);
}
