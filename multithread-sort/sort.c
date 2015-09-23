/*
 * Matthew Leeds, CS 426, 2015-09-23, OS Concepts Ch.4 Project 2
 * This program uses selection sort to sort each half of a given
 * array of integers, merges the halves in another thread, and
 * prints the sorted numbers to stdout.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>

// global variables
int* values; // unsorted input values
int* sortedValues; // sorted output values

void* selectionSort(void* arrAndLength);
void* mergeSortedSubarrays(void* arrAndLength);

// structure to pass to threads
typedef struct {
  int* arr;
  unsigned int arrLength;
} arrayAndLength;

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: ./sort <filename>\n");
    return 1;
  }

  unsigned int numValues = 0; // number of currently in-use slots in values
  unsigned int lengthValues = 1; // total number of slots in values
  unsigned int i; // useful for loops

  // Attempt to read data from the given filename
  FILE* inputFile = fopen(argv[1], "r");
  if (inputFile == NULL) {
    printf("Error: %s not accessible\n", argv[1]);
    return 2;
  } else {
    // read integers from the file, dynamically allocating an array
    char line[256]; // temporary buffer
    values = malloc(sizeof(int)); // unsorted input values
    while (fgets(line, sizeof(line), inputFile) != NULL) {
      if (numValues + 1 > lengthValues) { // resize array
        int* valuesBigger = malloc(sizeof(int) * lengthValues * 2);
        for (i = 0; i < numValues; ++i) { // copy the data
          valuesBigger[i] = values[i];
        }
        free(values);
        lengthValues = lengthValues * 2;
        values = valuesBigger;
      }
      values[numValues] = atoi(line); // convert value to int and store it
      numValues++;
    }
    fclose(inputFile);
  }

  // Now sort the numbers in the values array
  if (numValues < 2) { // trivial
    sortedValues = values;
  } else { // sort it using threads
    pthread_t tid1, tid2, tid3; // thread IDs
    pthread_attr_t attr; // thread attributes
    pthread_attr_init(&attr); // get default attributes

    // spawn a thread to sort the first half
    arrayAndLength* firstHalf = malloc(sizeof(arrayAndLength));
    firstHalf->arr = values;
    firstHalf->arrLength = numValues / 2;
    pthread_create(&tid1, &attr, selectionSort, firstHalf);

    // spawn a thread to sort the second half
    arrayAndLength* secondHalf = malloc(sizeof(arrayAndLength));
    secondHalf->arr = values + (numValues / 2); // pointer to the next element
    secondHalf->arrLength = (numValues % 2 == 0) ? (numValues / 2) : (numValues / 2 + 1);
    pthread_create(&tid2, &attr, selectionSort, secondHalf);

    // wait for both sorting threads to finish
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    
    // allocate memory for the sorted values
    sortedValues = malloc(sizeof(int) * numValues);

    // spawn and wait for a thread to merge the sorted sublists
    arrayAndLength* partiallySorted = malloc(sizeof(arrayAndLength));
    partiallySorted->arr = values;
    partiallySorted->arrLength = numValues;
    pthread_create(&tid3, &attr, mergeSortedSubarrays, partiallySorted);
    pthread_join(tid3, NULL);

    free(firstHalf);
    free(secondHalf);
    free(partiallySorted);
  }
  
  // Print the sorted values to stdout
  for (i = 0; i < numValues; ++i) {
    printf("%d\n", sortedValues[i]);
  }

  // free memory
  if (sortedValues != values) free(sortedValues);
  free(values);

  return 0;
}

void* selectionSort(void* param) {
  int* arr = ((arrayAndLength*)param)->arr;
  unsigned int arrLength = ((arrayAndLength*)param)->arrLength;
  unsigned int i, j, k;
  int temp;
  for (i = 0; i < arrLength; i++) {
    j = i;
    for (k = i + 1; k < arrLength; k++) {
      if (arr[k] < arr[j]) {
        j = k;
      }
    }
    if (j != i) {
      temp = arr[i];
      arr[i] = arr[j];
      arr[j] = temp;
    }
  }
  pthread_exit(0);
}

void* mergeSortedSubarrays(void* param) {
  int* arr = ((arrayAndLength*)param)->arr;
  unsigned int arrLength = ((arrayAndLength*)param)->arrLength;
  unsigned int leftIndex = 0;
  unsigned int rightIndex = arrLength / 2;
  unsigned int i;
  for (i = 0; i < arrLength; ++i) {
    if (rightIndex == arrLength)
      sortedValues[i] = arr[leftIndex++];
    else if (leftIndex == arrLength / 2)
      sortedValues[i] = arr[rightIndex++];
    else if (arr[leftIndex] < arr[rightIndex])
      sortedValues[i] = arr[leftIndex++];
    else
      sortedValues[i] = arr[rightIndex++];
  }
  pthread_exit(0);
}
