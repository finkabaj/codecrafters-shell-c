#ifndef PATH_H_
#define PATH_H_

#define MAX_PWD_LEN 256

void init_path(void);
char *get_pwd(void);
char *find_in_path(const char *cmd_name);

#endif
