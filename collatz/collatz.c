/*
 * This program executes the procedure specified by the Collatz conjecture
 * on a positive integer passed on the command line and prints the resulting
 * sequence from a forked child process. Specifically:
 *   if n is even, n = n/2
 *   if n is odd, n = 3*n + 1
 * which should eventually make n = 1. If the Collatz conjecture is not true,
 * this program could run indefinitely.
 *
 */

#include <stdio.h> // provides frpintf, printf
#include <unistd.h> // provides fork()
#include <sys/types.h> // provides pid_t

int nextCollatz(int current);

int main(int argc, char** argv) {
  if (argc != 2 || atoi(argv[1]) <= 0) {
    printf("Usage: ./collatz <positive integer>\n");
    return 1;
  }
  int start = atoi(argv[1]); // first integer in the sequence
  pid_t pid = fork();
  if (pid == 0) { // child process
    int current = start;
    while (current > 1) {
      printf("%d,", current);
      current = nextCollatz(current);
    }
    printf("%d\n", current);
  } else if (pid > 0) { // parent process
    wait(NULL); // wait for child to complete
  } else { // fork failed
    fprintf(stderr, "Fork Failed\n");
    return 2;
  }
  return 0;
}

int nextCollatz(int current) {
  if (current % 2 == 0)
    return current / 2;
  else
    return 3 * current + 1;
}
