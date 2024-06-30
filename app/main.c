#include "trie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TrieNode TrieNode;

int repl(char input[]);
int parse_input(char *input, char ***argv);
void free_argv(int argc, char **argv);
int execute_cmd(int argc, char **argv);

int main(void) {
  char input[100];

  if (!init_trie()) {
    return 1;
  }

  while (repl(input))
    ;

  return 0;
}

int repl(char input[]) {
  printf("$ ");
  fflush(stdout);

  if (fgets(input, 100, stdin) == NULL) {
    return 0;
  }

  input[strlen(input) - 1] = '\0';

  char **argv = NULL;

  int argc = parse_input(input, &argv);

  if (argc > 0) {
    int code = execute_cmd(argc, argv);
    if (!code) {
      fprintf(stderr, "%s: command not found\n", input);
    }
    free_argv(argc, argv);
  } else if (argc == -1) {
    fprintf(stderr, "%s: error parsing command\n", input);
  }

  return 1;
}

int parse_input(char *input, char ***argv) {
  int argc = 0;
  int cap = 5;
  char **args = malloc(cap * sizeof(char *));

  if (!args) {
    return -1;
  }

  char *token = strtok(input, " ");

  while (token != NULL) {
    if (argc >= cap) {
      cap *= 2;
      char **new_args = realloc(args, cap * sizeof(char *));
      if (!new_args) {
        free_argv(argc, args);
        return -1;
      }
      args = new_args;
    }

    args[argc] = strdup(token);
    if (!args[argc]) {
      free_argv(argc, args);
      return -1;
    }

    argc++;
    token = strtok(NULL, " ");
  }

  *argv = args;
  return argc;
}

int execute_cmd(int argc, char **argv) {
  TrieNode *node = find_in_trie(argv[0]);
  if (node == NULL) {
    return 0;
  }
  node->cmd->function(argc, argv);
  return 1;
}

void free_argv(int argc, char **argv) {
  if (argv == NULL) {
    return;
  }

  for (int i = 0; i < argc; i++) {
    free(argv[i]);
  }
  free(argv);
}
