#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define EXIT_COMMAND "exit"
#define ECHO_COMMAND "echo"
#define TYPE_COMMAND "type"

const char *SHELL_COMMANDS[3] = {EXIT_COMMAND, ECHO_COMMAND, TYPE_COMMAND};

struct Command
{
  char *name;
  char **args;
  char *path;
  int argc;
};

struct Command *parse_command(char *input)
{
  struct Command *cmd = malloc(sizeof(struct Command));

  size_t capacity = 5;
  int argc = 0;

  char **args = malloc(capacity * sizeof(char *));

  char *token = strtok(input, " ");

  while (token != NULL)
  {
    if (argc == (int)capacity)
    {
      capacity *= 2;
      args = realloc(args, capacity * sizeof(char *));
    }

    char *token_ptr = malloc(strlen(token) + 1);
    strcpy(token_ptr, token);

    if (argc == 0)
    {
      cmd->name = token_ptr;
    }

    args[argc] = token_ptr;

    argc++;

    token = strtok(NULL, " ");
  }

  args[argc] = NULL;

  cmd->args = args;
  cmd->argc = argc;

  return cmd;
}

void free_command(struct Command *cmd)
{
  if (cmd == NULL)
    return;

  for (int i = 0; i < cmd->argc; i++)
  {
    free(cmd->args[i]);
  }

  free(cmd->path);

  free(cmd);
}

void printCommand(struct Command *cmd)
{
  printf("name: %s\n", cmd->name);
  printf("path: %s\n", cmd->path);
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

    free(full_path);
  } while ((token = strtok(NULL, ":")) != NULL);

  return NULL;
}

void handle_exit()
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
    if (cmd->path != NULL)
    {
      printf("%s is %s", command, cmd->path);
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
    if (i == 0)
      printf("%s ", cmd->args[i]);
    else
      printf(" %s", cmd->args[i]);
  }

  printf("\n");
}

void handle_not_found(struct Command *cmd)
{
  printf("%s: command not found", cmd->name);
}

void handle_external_command(struct Command *cmd)
{
  int pid = fork();

  if (pid < 0)
  {
    printf("Something wrong happened...");
    return;
  }

  if (pid == 0)
  {
    execv(cmd->path, cmd->args);
  }

  waitpid(pid, NULL, 0);
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

    cmd->path = search_for_command_in_path(cmd->name);

    // printCommand(cmd);

    if (cmd->path != NULL)
    {
      handle_external_command(cmd);
    }
    else if (!strcmp(cmd->name, EXIT_COMMAND))
    {
      handle_exit();
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

    free_command(cmd);
    printf("\n$ ");
  }

  return 0;
}
