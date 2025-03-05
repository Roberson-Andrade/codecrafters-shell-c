#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_COMMAND "exit"
#define ECHO_COMMAND "echo"
#define TYPE_COMMAND "type"

const char *SHELL_COMMANDS[3] = {EXIT_COMMAND, ECHO_COMMAND, TYPE_COMMAND};

int is_shell_command(const char *input)
{
  int result = 0;

  for (int i = 0; i < 3; i++)
  {
    if (!strncmp(input, SHELL_COMMANDS[i], strlen(SHELL_COMMANDS[i])))
    {
      result = 1;
      break;
    }
  }

  return result;
}

int main()
{
  // Flush after every printf
  setbuf(stdout, NULL);

  printf("$ ");

  // Wait for user input
  char input[100];

  while (fgets(input, 100, stdin) != NULL)
  {
    input[strlen(input) - 1] = '\0';

    if (!strncmp(input, EXIT_COMMAND, strlen(EXIT_COMMAND)))
    {
      exit(0);
      break;
    }

    if (!strncmp(input, TYPE_COMMAND, strlen(TYPE_COMMAND)))
    {
      if (is_shell_command(input + strlen(TYPE_COMMAND) + 1) == 1)
      {
        printf("%s is a shell builtin", input + strlen(TYPE_COMMAND) + 1);
      }
      else
      {
        printf("%s: not found", input + strlen(TYPE_COMMAND) + 1);
      }
    }
    else if (!strncmp(input, ECHO_COMMAND, strlen(ECHO_COMMAND)))
    {
      printf("%s", input + strlen(ECHO_COMMAND) + 1);
    }
    else
    {
      printf("%s: command not found", input);
    }

    printf("\n$ ");
  }

  return 0;
}
