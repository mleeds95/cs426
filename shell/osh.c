/*
 * This program is a very simple UNIX shell, osh, that can execute 
 * commands either as child processes or concurrently (using the & operator), 
 * and has a "history" command to show past executions.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE 80

int main() {
  // print /etc/motd if it exists
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

  // We will set should_run to 0 to exit the shell.
  int should_run = 1;

  // This while loop is the shell's prompt.
  while (should_run) {
    // Due to spaces betweens args, this array will fit them all.
    char* args[MAX_LINE/2 + 1] = {NULL};
    printf("osh>");
    fflush(stdout);
    char line[MAX_LINE];
    bool background = false; // if true, run process concurrently
    if (fgets(line, MAX_LINE, stdin)) {
      if (strchr(line, '\n') == NULL) {
        fprintf(stderr, "Maximum line length %d exceeded!\n", MAX_LINE);
        return 1;
      } else if (strchr(line, ' ') == NULL) { // just one arg
        args[0] = line;
      } else { // arguments need tokenization
        char* token = strtok(line, " ");
        int i;
        for (i = 0; i <= MAX_LINE/2; ++i) {
          if (token == NULL) {
            // chop off the newline character
            args[i-1][strlen(args[i-1])-1] = '\0';
            break;
          }
          args[i] = token;
          token = strtok(NULL, " ");
        }
        if (strcmp(args[i-1], "&") == 0) {
          background = true;
          args[i-1] = NULL;
        }
      }
      // Exceute the given command in a child process.
      pid_t pid = fork();
      if (pid == 0) { // child process
        execvp(args[0], args);
      } else if (pid > 0) { // parent process
        if (!background) wait(NULL);
      } else { // fork failed
        fprintf(stderr, "Fork failed!\n");
        return 2;
      }
    } else { // usually EOF, so exit
      printf("\n");
      should_run = 0;
    }
  }

  return 0;
}
