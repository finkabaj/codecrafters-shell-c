#ifndef CMDS_H
#define CMDS_H

#include <stddef.h>

#define INITIAL_NAME_LIST_CAP 128

typedef struct {
  char *name;
  void (*function)(int argc, char **argv);
  char *path;
} Command;

typedef struct {
  char **cmd_names;
  int count;
  int capacity;
} CommandNameList;

Command *create_path_cmd(const char *name, const char *path);
const Command *lookup_cmd(const char *cmd_name);

void exit_handler(int argc, char **argv);
void echo_handler(int argc, char **argv);
void type_handler(int argc, char **argv);
void pwd_handler(int argc, char **argv);
void cd_handler(int argc, char **argv);

extern Command cmds[];
extern const size_t cmds_count;

#endif // CMDS_H
