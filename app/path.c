#include "path.h"
#include "cmds.h"
#include "trie.h"
#include "util.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char **path = NULL;
char cwd[MAX_CWD_LEN] = {0};
char *home_dir = NULL;
int pathc = 0;

void init_path(void) {
  char *or_path = getenv("PATH");

  if (!or_path) {
    fprintf(stderr, "error: PATH environment variable is not set\n");
    return;
  }

  char *path_copy = strdup(or_path);

  if (!path_copy) {
    return;
  }

  char *token = strtok(path_copy, ":");
  path = malloc(1 * sizeof(char *));

  if (!path) {
    return;
  }
  pathc = 0;

  while (token) {
    if (pathc >= 1) {
      char **new_path = realloc(path, (pathc + 1) * sizeof(char *));
      if (!new_path) {
        free(path_copy);
        free_ptr_to_str(pathc, path);
        path = NULL;
        pathc = 0;
      }
      path = new_path;
    }

    path[pathc] = strdup(token);
    if (!path[pathc]) {
      free(path_copy);
      free_ptr_to_str(pathc, path);
      path = NULL;
      pathc = 0;
      return;
    }

    pathc++;
    token = strtok(NULL, ":");
  }

  free(path_copy);
}

void add_path_cmds(void) {
  for (int i = 0; i < pathc; i++) {
    DIR *d;
    d = opendir(path[i]);
    if (!d) {
      continue;
    }
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
      if (dir->d_type != DT_REG) {
        continue;
      }
      char path_buff[BUFSIZ];
      snprintf(path_buff, BUFSIZ, "%s/%s", path[i], dir->d_name);
      if (access(path_buff, X_OK) != 0) {
        continue;
      }
      Command *cmd = create_path_cmd(dir->d_name, path_buff);
      if (cmd && !insert_cmd(cmd)) {
        free(cmd->name);
        free(cmd->path);
        free(cmd);
      }
    }
    closedir(d);
  }
}

char *get_home_dir(void) {
  if (!home_dir) {
    home_dir = getenv("HOME");

    if (!home_dir) {
      fprintf(stderr, "error: HOME environment variable not set\n");
      return NULL;
    }
  }

  return home_dir;
}

void _get_cwd(char *cwd) {
  if (cwd) {
    getcwd(cwd, MAX_CWD_LEN);
  }
}

char *get_cwd() {
  if (*cwd == '\0') {
    _get_cwd(cwd);
  }

  return cwd;
}

int set_cwd(char *path) {
  if (chdir(path) == 0) {
    _get_cwd(cwd);
    return 1;
  }

  perror(path);
  return 0;
}

char *find_in_path(const char *cmd_name) {
  if (!cmd_name) {
    return NULL;
  }

  size_t cmd_len = strlen(cmd_name);
  char *fullpath = NULL;
  size_t max_path_len = 0;

  for (int i = 0; i < pathc; i++) {
    size_t path_len = strlen(path[i]);
    if (path_len > max_path_len) {
      max_path_len = path_len;
    }
  }

  size_t fullpath_len = max_path_len + 1 + cmd_len + 1;
  fullpath = malloc(fullpath_len);
  if (!fullpath) {
    return NULL;
  }

  for (int i = 0; i < pathc; i++) {
    snprintf(fullpath, fullpath_len, "%s/%s", path[i], cmd_name);

    if (access(fullpath, X_OK) == 0) {
      return fullpath;
    }
  }

  free(fullpath);
  return NULL;
}
