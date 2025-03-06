#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define EXIT_COMMAND "exit"
#define ECHO_COMMAND "echo"
#define TYPE_COMMAND "type"

const char *SHELL_COMMANDS[3] = {EXIT_COMMAND, ECHO_COMMAND, TYPE_COMMAND};

struct Command
{
  char *name;
  char **args;
  int argc;
};

struct Command *parse_command(char *input)
{
  struct Command *cmd = malloc(sizeof(struct Command));

  int capacity = 5;
  int argc = 0;
  int is_command_name = 1;

  char **args = malloc(capacity * sizeof(char *));

  char *token = strtok(input, " ");

  while (token != NULL)
  {
    if (argc == capacity)
    {
      capacity *= 2;
      args = realloc(args, capacity * sizeof(char *));
    }

    char *token_ptr = malloc(strlen(token) + 1);
    strcpy(token_ptr, token);

    if (is_command_name == 1)
    {
      cmd->name = token_ptr;
      token = strtok(NULL, " ");
      is_command_name = 0;

      continue;
    }

    args[argc] = token_ptr;

    argc++;

    token = strtok(NULL, " ");
  }

  cmd->args = args;
  cmd->argc = argc;

  return cmd;
}

void printCommand(struct Command *cmd)
{
  printf("name: %s\n", cmd->name);
  printf("argc: %d\n", cmd->argc);

  for (int i = 0; i < cmd->argc; i++)
  {
    printf("arg: %s\n", cmd->args[i]);
  }
}

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

void handleExit()
{
  exit(0);
}

void handle_type(struct Command *cmd)
{
  if (cmd->argc == 0)
  {
    return;
  }

  char *command = cmd->args[0];

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

      free(command_path);
    }
    else
    {

      printf("%s: not found", command);
    }
  }
}

void handle_echo(struct Command *cmd)
{
  if (cmd->argc == 0)
  {
    return;
  }

  for (int i = 0; i < cmd->argc; i++)
  {
    char formatter[4];

    if (i == 0)
    {
      strcpy(formatter, "%s ");
    }
    else
    {
      strcpy(formatter, " %s");
    }

    printf(formatter, cmd->args[i]);
  }

  printf("\n");
}

void handle_not_found(struct Command *cmd)
{
  printf("%s: command not found", cmd->name);
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

    struct Command *cmd = parse_command(input);

    if (!strcmp(cmd->name, EXIT_COMMAND))
    {
      handleExit();
    }
    else if (!strcmp(cmd->name, TYPE_COMMAND))
    {
      handle_type(cmd);
    }
    else if (!strcmp(cmd->name, ECHO_COMMAND))
    {
      handle_echo(cmd);
    }
    else
    {
      handle_not_found(cmd);
    }

    printf("\n$ ");
  }

  return 0;
}
