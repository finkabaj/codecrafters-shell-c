#include "cmds.h"
#include "path.h"
#include "trie.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct TrieNode TrieNode;

int repl(char input[]);
int parse_input(char *input, char ***argv);
int execute_cmd(int argc, char **argv);

int main(int argc, char **argv) {
  if (!get_pwd()) {
    printf("shell: error geting pwd\n");
    return 1;
  }

  init_path();

  if (!init_trie()) {
    printf("shell: error while initalizing\n");
    return 1;
  }

  if (argc > 1) {
    return execute_cmd(argc - 1, argv + 1);
  }

  char input[1024];
  while (repl(input))
    ;

  return 0;
}

int repl(char input[]) {
  printf("$ ");
  fflush(stdout);

  if (!fgets(input, 1024, stdin)) {
    return 0;
  }

  input[strcspn(input, "\n")] = '\0';

  char **argv = NULL;

  int argc = parse_input(input, &argv);

  if (argc > 0) {
    int code = execute_cmd(argc, argv);
    if (!code) {
      fprintf(stderr, "%s: command not found\n", input);
    }
    free_ptr_to_str(argc, argv);
  } else if (argc == -1) {
    fprintf(stderr, "%s: error parsing command\n", input);
  }

  return 1;
}

int parse_input(char *input, char ***argv) {
  if (input[0] == '\0') {
    *argv = NULL;
    return 0;
  }

  int argc = 0;
  int cap = 5;
  char **args = malloc(cap * sizeof(char *));

  if (!args) {
    return -1;
  }

  char *token = strtok(input, " ");

  while (token) {
    if (argc >= cap) {
      cap *= 2;
      char **new_args = realloc(args, cap * sizeof(char *));
      if (!new_args) {
        free_ptr_to_str(argc, args);
        return -1;
      }
      args = new_args;
    }

    args[argc] = strdup(token);
    if (!args[argc]) {
      free_ptr_to_str(argc, args);
      return -1;
    }

    argc++;
    token = strtok(NULL, " ");
  }

  *argv = args;
  return argc;
}

int execute_cmd(int argc, char **argv) {
  const Command *cmd = lookup_cmd(argv[0]);

  if (!cmd) {
    return 0;
  }

  if (cmd->function) {
    cmd->function(argc, argv);
  } else {
    pid_t pid = fork();

    if (pid < 0) {
      perror("fork");
      return 0;
    }

    if (pid == 0) {
      char *new_argv[argc + 1];
      memcpy(new_argv, argv, argc * sizeof(char *));
      new_argv[argc] = NULL;
      execv(cmd->path, new_argv);
      perror("execv");
      exit(1);
    } else {
      waitpid(pid, 0, 0);
    }
  }

  return 1;
}
