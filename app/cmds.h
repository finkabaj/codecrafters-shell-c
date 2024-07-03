#ifndef CMDS_H
#define CMDS_H

#include <stddef.h>

typedef struct Command {
  char *name;
  void (*function)(int argc, char **argv);
  char *path;
} Command;

const Command *create_path_cmd(const char *name, const char *path);
const Command *lookup_cmd(const char *cmd_name);

void exit_handler(int argc, char **argv);
void echo_handler(int argc, char **argv);
void type_handler(int argc, char **argv);
void pwd_handler(int argc, char **argv);
void cd_handler(int argc, char **argv);

extern const Command cmds[];
extern const size_t cmds_count;

#endif // CMDS_H
