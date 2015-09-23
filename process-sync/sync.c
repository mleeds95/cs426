/*
 * Matthew Leeds, CS 426, 2015-09-23, OS Concepts Ch.5 Exercise 37
 * This program manages a shared resource across processes without
 * a race condition using a semaphore.
 *
 */

#include <stdio.h>
#include <semaphore.h>

#define MAX_RESOURCES 5
int available_resources = MAX_RESOURCES;

int main(int argc, char** argv) {
  //TODO
  return 0;
}

// returns -1 if insufficient resources are available
// otherwise decrease available_resources by count and return 0
int decrease_count(int count) {
  if (available_resources < count)
    return -1;
  else
    available_resources -= count;
  return 0;
}

// increase available_resources by count
int increase_count(int count) {
  available_resources += count;
  return 0;
}
