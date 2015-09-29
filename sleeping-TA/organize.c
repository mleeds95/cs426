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
// will be locked when the TA is helping a student
pthread_mutex_t helping_student;
// -1 * the number of students waiting for help
sem_t students_waiting;

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
  sem_init(&students_waiting, 0, 0);

  // make a thread for the TA
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_mutex_init(&helping_student, NULL);
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
  
  // join the threads and free memory
  for (i = 0; i < numberOfStudents; ++i) {
    pthread_join(*tid_students[i], NULL);
    free(tid_students[i]);
    free(params[i]);
  }
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&helping_student);
  sem_destroy(&students_waiting);

  return 0;
}

void* simulate_TA(void* param) {
  //TODO
}

void* simulate_student(void* param) {
  //TODO
}
