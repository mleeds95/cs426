/*
 * Matthew Leeds, CS 426, 2015-9-16, OS Concepts Ch. 3 Project 1
 * This program is a very simple UNIX shell, osh, that can execute 
 * commands either as child processes or concurrently (using the & operator), 
 * and has a "history" command to show past executions.
 *
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE 80
#define HIST_LENGTH 10

void print_motd();
bool process_line(char* line, char** processed_args, char** line_copy_ptr);

int main() {
  // message of the day
  print_motd();

  // store the past ten commands in an array
  char* history[HIST_LENGTH] = {NULL};
  int historyLength = 0;

  // We will set should_run to false to exit the shell.
  bool should_run = true;
  
  // allocate space to hold address malloc'd by process_line
  char** line_copy_ptr = malloc(sizeof(char *));
  *line_copy_ptr = NULL;
  char* line = NULL; // for each line of input
  bool real_command = false; // true if a system command is executed, not a shell one

  // This while loop is the shell's prompt.
  while (should_run) {
    // free memory from last loop
    free(*line_copy_ptr);
    if (!real_command) free(line);

    real_command = false;
    line = malloc(MAX_LINE); // for next inputted line
    // Due to spaces betweens args, this array will fit them all.
    char* args[MAX_LINE/2 + 1] = {(char *) NULL};
    bool background = false; // if true, run process concurrently

    printf("osh>");
    fflush(stdout);

    if (fgets(line, MAX_LINE, stdin)) {
      if (strchr(line, '\n') == NULL) {
        fprintf(stderr, "Maximum line length %d exceeded\n", MAX_LINE);
        continue;
      } else if (strlen(line) == 1) { // just newline character
        continue;
      } else { // parse the arguments
        background = process_line(line, args, line_copy_ptr);
      }

      // Check if it's a shell command or a system one.
      if (strcmp(args[0], "exit") == 0) {
        should_run = false;
        break;
      } else if (strcmp(args[0], "history") == 0) {
        // print history from most to least recent
        int i;
        int lowerLimit = (historyLength - 10) >= 0 ? (historyLength - 10) : 0;
        for (i = historyLength - 1; i >= lowerLimit; --i) {
          printf("%d %s", i+1, history[i % 10]);
        }
        continue;
      } else if (args[0][0] == '!') {
        // execute a command from the history
        if (strcmp(args[0], "!!") == 0) {// load last command into args
          if (historyLength == 0) {
            printf("Error: no commands in history\n");
            continue;
          } else {
            line = history[(historyLength - 1) % 10];
            printf("%s", line);
            free(*line_copy_ptr);
            background = process_line(line, args, line_copy_ptr);
          }
        } else { // command is !<number>
          args[0][0] = '0'; // overwrite !
          int n = atoi(args[0]);
          int min = (historyLength <= 10) ? 1 : (historyLength - 9);
          if (n < min || n > historyLength) {
            printf("Error: %d outside of history range\n", n);
            continue;
          } else {
            line = history[(n - 1) % 10];
            printf("%s", line);
            free(*line_copy_ptr);
            background = process_line(line, args, line_copy_ptr);
          }
        }
      }

      real_command = true; // line won't be freed on the next loop
      // add command to history
      if (historyLength >= HIST_LENGTH) free(history[historyLength % HIST_LENGTH]);
      history[historyLength++ % HIST_LENGTH] = line;

      // Exceute the given command in a child process.
      pid_t pid = fork();
      if (pid == 0) { // child process
        execvp(args[0], args);
        // execvp replaces the process image, so if we make it past that the command failed
        printf("%s: command not found\n", args[0]);
        return -1; // exit child process w/ non-zero code
      } else if (pid > 0) { // parent process
        if (!background) wait(NULL);
      } else { // fork failed
        fprintf(stderr, "Fork failed!\n");
        return 2;
      }
    } else { // fgets failed, so exit (usually EOF)
      printf("\n");
      should_run = false;
    }
  } // end while

  // free memory
  int i;
  for (i = 0; i < HIST_LENGTH; ++i) {
    free(history[i]);
  }
  free(line_copy_ptr);

  return 0;
}

// print /etc/motd if it exists
void print_motd() {
  struct stat sb;
  if (stat("/etc/motd", &sb) == 0 && S_ISREG(sb.st_mode)) {
    FILE* motd = fopen("/etc/motd", "r");
    int c;
    if (motd) {
      while ((c = fgetc(motd)) != EOF) {
        fputc(c, stdout);
      }
      fclose(motd);
    }
  }
}

// parse a line of input into arguments
bool process_line(char* line, char** args, char** line_copy_ptr) {
  char* line_copy = malloc(MAX_LINE);
  *line_copy_ptr = line_copy;
  strcpy(line_copy, line);
  char* token = strtok(line_copy, " ");
  int i = 0;
  for (; i <= MAX_LINE/2; ++i) {
    if (token == NULL) {
      args[i-1][strlen(args[i-1])-1] = '\0'; // chop off newline character
      break;
    }
    args[i] = token;
    token = strtok(NULL, " ");
  }
  if (strcmp(args[i-1], "&") == 0) { // will be a background process
    args[i-1] = NULL;
    return true;
  }
  return false;
}
