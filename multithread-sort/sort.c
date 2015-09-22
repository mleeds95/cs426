/*
 * Matthew Leeds, CS 426, 2015-09-23, OS Concepts Ch.4 Project 2
 * This program implements the mergesort algorithm on an array of
 * integers using seperate threads for sorting each sublist, and
 * for each merge.
 *
 */

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: ./sort <filename>\n");
    return 1;
  }
  int* values = malloc(sizeof(int)); // unsorted input values
  unsigned int numValues = 0;
  unsigned int lengthValues = 1;
  FILE* inputFile = fopen(argv[1], "r");
  if (inputFile == NULL) {
	printf("Error: %s not accessible\n", argv[1]);
	return 2;
  } else {
    // read integers from the file, dynamically allocating an array
	char line[256]; // temporary buffer
	unsigned int i;
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
	  values[numValues] = atoi(line);
      numValues++;
	}
	fclose(inputFile);
    //for (i = 0; i < numValues; ++i) printf("%d\n", values[i]);
  }
  //TODO
  free(values);
  return 0;
}
