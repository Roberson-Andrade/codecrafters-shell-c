#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_CODE "exit 0"

int main()
{
  // Flush after every printf
  setbuf(stdout, NULL);

  // Uncomment this block to pass the first stage
  printf("$ ");

  // Wait for user input
  char input[100];

  while (fgets(input, 100, stdin) != NULL)
  {
    input[strlen(input) - 1] = '\0';

    if(strcmp(input, EXIT_CODE) == 0) {
      exit(0);
      break;
    }

    printf("%s: command not found\n", input);
    printf("$ ");
  }

  return 0;
}
