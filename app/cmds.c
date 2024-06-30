#include "cmds.h"
#include <stdio.h>
#include <stdlib.h>

const Command cmds[] = {
    {"exit", exit_handler},
    {"echo", echo_handler},
};

const size_t cmds_count = sizeof(cmds) / sizeof(cmds[0]);

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
