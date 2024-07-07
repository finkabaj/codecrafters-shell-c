#include "cmds.h"
#include "path.h"
#include "trie.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <termcap.h>
#include <termios.h>
#include <unistd.h>

typedef struct TrieNode TrieNode;

int repl(char input[]);
int parse_input(char *input, char ***argv);
int execute_cmd(int argc, char **argv);
void handle_tab_completion(char *prefix);

int term_ln = 0;
int term_clm = 0;

void get_term_size(void) {
  static char termbuf[2048];
  char *termtype = getenv("TERM");

  if (tgetent(termbuf, termtype) < 0) {
    exit(EXIT_FAILURE);
  }

  term_ln = tgetnum("li");
  term_clm = tgetnum("co");
}

void configure_terminal(struct termios *old_term, struct termios *new_term) {
  tcgetattr(STDIN_FILENO, old_term);
  *new_term = *old_term;
  new_term->c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, new_term);
}

void restore_terminal(struct termios *old_term) {
  tcsetattr(STDIN_FILENO, TCSANOW, old_term);
}

// INFO: consider memory pool in future

// TODO: implement cmd completion on '\t'
// step 1) add all exe in PATH to trie **DONE**
// step 2) add trie traversal **DONE**
// step 3) add cmd selection when user click on '\t' **DONE**
// step 4) minimize memory overhead
int main(int argc, char **argv) {
  struct termios old_term, new_term;
  configure_terminal(&old_term, &new_term);
  get_term_size();

  if (!get_cwd()) {
    printf("shell: error geting pwd\n");
    return 1;
  }
  get_home_dir();

  init_path();

  if (!init_trie()) {
    printf("shell: error while initalizing\n");
    return 1;
  }

  add_path_cmds();

  if (argc > 1) {
    return execute_cmd(argc - 1, argv + 1);
  }

  char input[BUFSIZ];
  while (repl(input))
    ;

  restore_terminal(&old_term);

  return 0;
}

int repl(char input[]) {
  printf("$ ");
  fflush(stdout);

  int cursor = 0;
  while (1) {
    char c = getchar();
    if (c == '\n') {
      input[cursor] = '\0';
      putchar('\n');
      fflush(stdout);
      break;
    } else if (c == '\t') {
      input[cursor] = '\0';
      handle_tab_completion(input);
      cursor = strlen(input);
    } else if (c == 8 || c == 127) {
      if (cursor > 0) {
        cursor--;
        printf("\b \b");
        fflush(stdout);
      }
    } else {
      input[cursor++] = c;
      putchar(c);
      fflush(stdout);
    }
  }

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

void handle_tab_completion(char *prefix) {
  if (!prefix || *prefix == '\0' || *prefix == '\n') {
    putchar('\t');
    return;
  }

  int pre_len = strlen(prefix);
  CommandNameList *list = traverse_trie_prefix(prefix);
  if (list) {
    int max_width = 0;
    for (int i = 0; i < list->count; i++) {
      int len = strlen(list->cmd_names[i]);
      if (len > max_width) {
        max_width = len;
      }
    }

    int cols = (term_ln + 1) / (max_width + 2);
    int rows = (list->count + cols - 1) / cols;

    printf("\033[s\033[?25l");
    printf("\n\033[J");

    putchar('\n');
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        int index = i + j * rows;
        if (index < list->count) {
          printf("%-*s", max_width + 2, list->cmd_names[index]);
        }
      }
      putchar('\n');
    }

    int selected = 0;
    while (1) {
      printf("\033[u\033[B");

      for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
          int index = i + j * rows;
          if (index < list->count) {
            if (index == selected) {
              printf("\033[47m\033[30m%-*s\033[0m", max_width + 2,
                     list->cmd_names[index]);
            } else {
              printf("%-*s", max_width + 2, list->cmd_names[index]);
            }
          }
        }
        printf("\033[K");
        if (i < rows - 1) {
          printf("\n");
        }
      }

      printf("\033[J");

      char c = getchar();
      if (c == '\t') {
        selected = (selected + 1) % list->count;
      } else if (c == '\n') {
        strcpy(prefix, list->cmd_names[selected]);
        break;
      } else if (c == 27) {
        break;
      }
    }

    printf("\033[u\033[?25h");
    printf("\033[J");

    for (int i = 0; i < pre_len + 2; i++) {
      printf("\b");
    }

    printf("$ %s", prefix);
    fflush(stdout);

    free_command_name_list(list);
  }
}
