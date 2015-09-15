/*
 * This program is a very simple UNIX shell, osh, that can execute 
 * commands either as child processes or concurrently (using the & operator), 
 * and has a "history" command to show past executions.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

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

  // Due to spaces betweens args, this array will fit them all.
  char *args[MAX_LINE/2 + 1];
  // We will set should_run to 0 to exit the shell.
  int should_run = 1;

  // This while loop is the shell's prompt.
  while (should_run) {
    printf("osh>");
    fflush(stdout);
    char line[MAX_LINE];
    if (fgets(line, MAX_LINE, stdin)) {
      printf("%s",line);
    } else { // usually EOF, so exit
      printf("\n");
      should_run = 0;
    }
  }

  return 0;
}
