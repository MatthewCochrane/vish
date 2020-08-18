/*
 * vish
 */
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include "sys/types.h"
#include "stdio.h"
#include "unistd.h"

//char PATH[] = "/usr/bin /usr/sbin /usr/local/bin /usr/local/sbin /bin"
char PATH[] = "/bin";

void process_input(char *input);

int read_input(char *input, size_t nbytes) {
  int bytes_read = read(STDIN_FILENO, input, nbytes);
  input[bytes_read-1] = '\0';
  return bytes_read > 0 ? 0 : -1;
}

char** str_split(char* a_str, const char a_delim)
{
  char** result    = 0;
  size_t count     = 0;
  char* tmp        = a_str;
  char* last_comma = 0;
  char delim[2];
  delim[0] = a_delim;
  delim[1] = 0;

  /* Count how many elements will be extracted. */
  while (*tmp)
  {
    if (a_delim == *tmp)
    {
        count++;
        last_comma = tmp;
    }
    tmp++;
  }

  /* Add space for trailing token. */
  count += last_comma < (a_str + strlen(a_str) - 1);

  /* Add space for terminating null string so caller
     knows where the list of returned strings ends. */
  count++;

  result = malloc(sizeof(char*) * count);

  if (result)
  {
      size_t idx  = 0;
      char* token = strtok(a_str, delim);

      while (token)
      {
          assert(idx < count);
          *(result + idx++) = strdup(token);
          token = strtok(0, delim);
      }
      assert(idx == count - 1);
      *(result + idx) = 0;
  }

  return result;
}

void free_split_str(char** split_str) {
  int i;
  for (i = 0; *(split_str + i); i++)
  {
    free(*(split_str + i));
  }
  free(split_str);
}

void process_input(char* input) {
  pid_t cpid, w;
  int wstatus;

  if (strncmp(input, "exit", 4) == 0) {
    exit(EXIT_SUCCESS);
  }

  char cmd[1029];
  snprintf(cmd, sizeof cmd, "%s/%s", PATH, input);
//    printf("running: %s\n", cmd);

  char** args = str_split(cmd, ' ');

  if (!args) return;

  cpid = fork();
  if (cpid == -1) {
    perror("Failed to fork.");
    exit(EXIT_FAILURE);
  }

  if (cpid == 0) {
    // Child path after fork
    execv(args[0], args);
    exit(EXIT_SUCCESS);
  } else {
    // Parent path after fork
    w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
    if (w == -1) {
      perror("waitpid failed.");
      exit(EXIT_FAILURE);
    }
  }

  free_split_str(args);
}

int main(int argc, char** argv) {
  char input[1024];
  int read_success;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
  while (1) {
    write(STDOUT_FILENO, "> ", 2);
    read_success = read_input(input, sizeof input);
    if (read_success == -1) {
      printf("Error reading input\n");
    }
    process_input(input);
  };
#pragma clang diagnostic pop
}

