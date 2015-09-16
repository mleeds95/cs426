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
#define HIST_LENGTH 10

typedef struct {
  unsigned int commandNumber;
  char* line;
  struct historyNode* next;
} historyNode;

typedef struct {
  struct historyNode* head;
  struct historyNode* tail;
  unsigned int numEntries;
} history;

void print_motd();
bool process_line(char* line, char** processed_args);

int main() {
  print_motd();

  // use a singly linked list for command history
  history h;
  h.numEntries = 0;
  h.head = h.tail = NULL;
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
      } else if (strlen(line) == 1) { // just newline character
        continue;
      } else {
        background = process_line(line, args);
      }
      // Check if it's a shell command or a system one.
      if (strcmp(args[0], "exit") == 0) {
        should_run = 0;
        break;
      } else if (strcmp(args[0], "history") == 0) {
        //TODO print history from most to least recent
      } else if (args[0][0] == '!') {
        //TODO execute a command from the history
      } else {
        //TODO add command to history
        historyNode hN;
        hN.next = NULL;
        struct historyNode* hTail = h.tail;
        hN.commandNumber = (h.numEntries > 0 ? ++(hTail->commandNumber) : 1);
        hN.line = line;
        if (h.head == NULL) h.head = &hN;
        else h.tail->next = &hN;
        h.tail = &hN;
        if (++h.numEntries > HIST_LENGTH) {
          h.head = h.head->next;
          --h.numEntries;
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
bool process_line(char* line, char** args) {
  if (strchr(line, ' ') == NULL) { // just one arg
    args[0] = line;
    args[0][strlen(args[0])-1] = '\0'; // chop off newline character
  } else { // arguments need tokenization
    char* token = strtok(line, " ");
    int i;
    for (i = 0; i <= MAX_LINE/2; ++i) {
      if (token == NULL) {
        args[i-1][strlen(args[i-1])-1] = '\0'; // chop off newline character
        break;
      }
      args[i] = token;
      token = strtok(NULL, " ");
    }
    if (strcmp(args[i-1], "&") == 0) {
      args[i-1] = NULL;
      return true;
    }
  }
  return false;
}
