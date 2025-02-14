#ifndef PATH_H_
#define PATH_H_

#define MAX_CWD_LEN 256

void init_path(void);
char *get_cwd(void);
int set_cwd(char *path);
char *find_in_path(const char *cmd_name);
char *get_home_dir(void);
void add_path_cmds(char *prefix, int prefix_len);

#endif
