#include "cmds.h"
#include "path.h"
#include "trie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const Command cmds[] = {
    {"exit", exit_handler, NULL}, {"echo", echo_handler, NULL},
    {"type", type_handler, NULL}, {"pwd", pwd_handler, NULL},
    {"cd", cd_handler, NULL},
};

const size_t cmds_count = sizeof(cmds) / sizeof(cmds[0]);

const Command *create_path_cmd(const char *name, const char *path) {
  Command *cmd = malloc(sizeof(Command));
  if (!cmd)
    return NULL;

  cmd->name = strdup(name);
  if (!cmd->name) {
    free(cmd);
    return NULL;
  }

  cmd->path = strdup(path);
  if (!cmd->path) {
    free(cmd->name);
    free(cmd);
    return NULL;
  }

  cmd->function = NULL;
  return cmd;
}

const Command *lookup_cmd(const char *cmd_name) {
  TrieNode *node = find_in_trie(cmd_name);

  if (node && node->cmd) {
    return node->cmd;
  }

  char *full_path = find_in_path(cmd_name);

  if (!full_path) {
    return NULL;
  }

  const Command *cmd = create_path_cmd(cmd_name, full_path);
  free(full_path);

  if (!cmd) {
    return NULL;
  }

  if (!insert_cmd(cmd)) {
    free(cmd->name);
    free(cmd->path);
    free((void *)cmd);
    return NULL;
  }

  return cmd;
}

void exit_handler(int argc, char **argv) {
  if (argc > 2) {
    fprintf(stderr, "usage: exit [exit_code]\n");
    return;
  }

  int exit_code = 0;

  if (argc == 2 && (sscanf(argv[1], "%d", &exit_code) != 1 || exit_code < 0)) {
    fprintf(stderr, "exit: exit_code must be a non-negative integer\n");
    return;
  }

  exit(exit_code);
}

void echo_handler(int argc, char **argv) {
  if (argc == 1) {
    fprintf(stderr, "usage echo [string]\n");
    return;
  }

  for (int i = 1; i < argc; i++) {
    printf("%s", argv[i]);
    if (i < argc - 1) {
      printf(" ");
    }
  }
  printf("\n");
}

void type_handler(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage type [cmd]\n");
    return;
  }

  const Command *cmd = lookup_cmd(argv[1]);

  if (!cmd) {
    fprintf(stderr, "%s: not found\n", argv[1]);
    return;
  }

  if (cmd->path) {
    printf("%s is %s\n", argv[1], cmd->path);
    return;
  }

  printf("%s is a shell builtin\n", argv[1]);
}

void pwd_handler(int argc, char **argv) {
  (void)argv;
  if (argc > 1) {
    fprintf(stderr, "pwd: too many arguments\n");
    return;
  }

  printf("%s\n", get_cwd());
}

void cd_handler(int argc, char **argv) {
  if (argc > 3) {
    fprintf(stderr, "cd: too many arguments\n");
  }

  if (argc == 2) {
    set_cwd(*argv[1] == '~' ? get_home_dir() : argv[1]);
  } else if (argc == 1) {
    set_cwd(get_home_dir());
  }
}
