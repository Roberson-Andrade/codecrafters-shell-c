#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

char *search_for_command_in_path(const char *command)
{
  char *path = strdup(getenv("PATH"));

  int count = 0;

  char *token = strtok(path, ":");

  do
  {
    char *full_path = malloc(strlen(token) + strlen(command) + 2);

    strcpy(full_path, token);
    strcat(full_path, "/");
    strcat(full_path, command);

    int is_executable = access(full_path, X_OK);

    if (is_executable == 0)
    {
      return full_path;
    }
  } while ((token = strtok(NULL, ":")) != NULL);

  return NULL;
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
      char *command = input + strlen(TYPE_COMMAND) + 1;

      if (is_shell_command(command) == 1)
      {
        printf("%s is a shell builtin", command);
      }
      else
      {
        char *command_path = search_for_command_in_path(command);

        if (command_path != NULL)
        {
          printf("%s is %s", command, command_path);
        }
        else
        {

          printf("%s: command not found", command);
        }
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
