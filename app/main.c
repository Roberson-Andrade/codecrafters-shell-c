#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define EXIT_COMMAND "exit"
#define ECHO_COMMAND "echo"
#define TYPE_COMMAND "type"
#define PWD_COMMAND "pwd"
#define CD_COMMAND "cd"

#define BUILTIN_LENGTH 5
#define INITIAL_CAPACITY 5

const char *SHELL_COMMANDS[BUILTIN_LENGTH] = {EXIT_COMMAND, ECHO_COMMAND, TYPE_COMMAND, PWD_COMMAND, CD_COMMAND};

struct Command
{
  char *name;
  char **args;
  char *path;
  int argc;
};

void free_tokens(char **input)
{
  for (int i = 0; input[i] != NULL; i++)
  {
    free(input[i]);
  }

  free(input);
}

char **tokenize(char *input, int *counter)
{
  char *current = input;
  size_t tokens_capacity = INITIAL_CAPACITY;
  char **tokens = malloc(tokens_capacity * sizeof(char *));

  if (!tokens)
    return NULL;

  while (*current != '\0' && *current != '\n')
  {
    while (*current == ' ')
    {
      current++;
    };

    if (*counter >= (int)tokens_capacity)
    {
      tokens_capacity *= 2;
      tokens = realloc(tokens, tokens_capacity * sizeof(char *));

      if (!tokens)
        return NULL;
    }

    if (*current == '\'')
    {
      size_t quotes_capacity = INITIAL_CAPACITY;
      char *quotes = malloc(quotes_capacity * sizeof(char));

      int i = 0;
      current++; // skip '

      while (*current != '\'' && *current != '\0')
      {
        quotes[i++] = *current;
        current++;

        if (*current == '\0')
        {
          printf("Error: Non terminated quotes\n");
          return NULL;
        }

        if (i >= (int)quotes_capacity)
        {
          quotes_capacity *= 2;
          quotes = realloc(quotes, quotes_capacity * sizeof(char));
        }

        if (*current == '\'' && *(current + 1) == '\'')
        {
          current += 2;
        }
      }

      quotes[i] = '\0';
      tokens[(*counter)++] = quotes;
      current++;
    }
    else
    {
      size_t word_capacity = INITIAL_CAPACITY;
      char *word = malloc(word_capacity * sizeof(char));

      int i = 0;

      while (*current != ' ' && *current != '\0')
      {
        word[i++] = *current;
        current++;

        if (*current == '\0')
        {
          break;
        }

        if (i >= (int)word_capacity)
        {
          word_capacity *= 2;
          word = realloc(word, word_capacity * sizeof(char));
        }
      }

      word[i] = '\0';
      tokens[(*counter)++] = word;
    }

    if (*counter >= (int)tokens_capacity)
    {
      tokens_capacity *= 2;
      tokens = realloc(tokens, tokens_capacity * sizeof(char *));
    }
  }

  return tokens;
}

struct Command *parse_command(char *input)
{
  struct Command *cmd = malloc(sizeof(struct Command));

  int argc = 0;

  char **args = tokenize(input, &argc);

  cmd->name = args[0];
  cmd->args = args;
  cmd->argc = argc;

  return cmd;
}

void free_command(struct Command *cmd)
{
  if (cmd == NULL)
    return;

  free_tokens(cmd->args);
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

  for (int i = 0; i < BUILTIN_LENGTH; i++)
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
  if (cmd->argc < 2)
  {
    return;
  }

  char *command = cmd->args[1];
  char *command_path = search_for_command_in_path(command);

  if (is_shell_command(command) == 1)
  {
    printf("%s is a shell builtin\n", command);
  }
  else if (command_path != NULL)
  {
    printf("%s is %s\n", command, command_path);
  }
  else
  {
    printf("%s: not found\n", command);
  }
}

void handle_echo(struct Command *cmd)
{
  if (cmd->argc < 2)
  {
    return;
  }

  for (int i = 1; i < cmd->argc; i++)
  {
    if (i == 1)
      printf("%s", cmd->args[i]);
    else
      printf(" %s", cmd->args[i]);
  }

  printf("\n");
}

void handle_not_found(struct Command *cmd)
{
  printf("%s: command not found\n", cmd->name);
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

void handle_pwd()
{
  char *path = getcwd(NULL, 0);

  printf("%s\n", path);

  free(path);
}

void handle_cd(struct Command *cmd)
{
  if (cmd->argc < 2)
  {
    return;
  }
  char *path = cmd->args[1];

  if (!strcmp(path, "~"))
  {
    char *home = strdup(getenv("HOME"));

    if (home == NULL)
      return;

    path = home;
  }

  if (chdir(path) == -1)
  {
    printf("cd: %s: No such file or directory\n", path);
  }
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

    if (!strcmp(cmd->name, EXIT_COMMAND))
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
    else if (!strcmp(cmd->name, PWD_COMMAND))
    {
      handle_pwd();
    }
    else if (!strcmp(cmd->name, CD_COMMAND))
    {
      handle_cd(cmd);
    }
    else if (cmd->path != NULL)
    {
      handle_external_command(cmd);
    }
    else
    {
      handle_not_found(cmd);
    }

    free_command(cmd);

    printf("$ ");
  }

  return 0;
}
