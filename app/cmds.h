#ifndef CMDS_H
#define CMDS_H

#include <stddef.h>

typedef struct Command {
  const char *name;
  void (*function)(int argc, char **argv);
} Command;

void exit_handler(int argc, char **argv);

extern const Command cmds[];
extern const size_t cmds_count;

#endif // CMDS_H
