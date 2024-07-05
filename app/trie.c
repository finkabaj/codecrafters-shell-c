#include "trie.h"
#include "cmds.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

TrieNode *root = NULL;

void free_trie(TrieNode *root) {
  if (!root) {
    return;
  }

  for (int i = 0; i < ALPHABET_SIZE; i++) {
    free_trie(root->children[i]);
    root->children[i] = NULL;
  }

  if (root->cmd) {
    free(root->cmd->name);
    if (root->cmd->path) {
      free(root->cmd->path);
    }
    free(root->cmd);
  }

  free(root);
}

int char_to_index(char c) { return (unsigned char)c; }

int _insert_cmd(TrieNode *root, Command *cmd) {
  assert(root);
  assert(cmd);
  assert(cmd->name);

  const char *current = cmd->name;

  while (*current != '\0') {
    int index = char_to_index(*current);
    if (index == -1) {
      return 0;
    }

    if (!root->children[index]) {
      root->children[index] = calloc(1, sizeof(TrieNode));
      if (!root->children[index]) {
        return 0;
      }
    }

    root = root->children[index];
    current++;
  }

  if (root->is_cmd) {
    return 0;
  }

  root->is_cmd = 1;
  root->cmd = cmd;
  return 1;
}

int insert_cmd(Command *cmd) { return _insert_cmd(root, cmd); }

int init_trie() {
  root = calloc(1, sizeof(TrieNode));

  if (!root) {
    return 0;
  }

  for (size_t i = 0; i < cmds_count; i++) {
    if (!_insert_cmd(root, &cmds[i])) {
      free_trie(root);
      root = NULL;
      return 0;
    }
  }

  return 1;
}

TrieNode *_find_in_trie(TrieNode *node, const char *cmd_name) {
  if (!node || !cmd_name) {
    return NULL;
  }

  while (*cmd_name != '\0') {
    int index = char_to_index(*cmd_name);
    if (index == -1) {
      return NULL;
    }
    if (!node->children[index]) {
      return NULL;
    }
    node = node->children[index];
    cmd_name++;
  }

  return node;
}

TrieNode *find_cmd_in_trie(const char *cmd_name) {
  TrieNode *node = _find_in_trie(root, cmd_name);
  return (node && node->is_cmd && node->cmd) ? node : NULL;
}

void _traverse_trie_prefix(TrieNode *node, char *prefix, int depth,
                           CommandNameList *list) {
  if (!node) {
    return;
  }

  if (node->is_cmd && node->cmd) {
    if (list->count >= list->capacity) {
      int new_capacity = list->capacity * 2;
      char **new_commands =
          realloc(list->cmd_names, new_capacity * sizeof(char *));
      if (!new_commands) {
        return;
      }
      list->cmd_names = new_commands;
      list->capacity = new_capacity;
    }

    list->cmd_names[list->count] = strdup(prefix);
    if (list->cmd_names[list->count]) {
      list->count++;
    }
  }

  for (int i = 0; i < ALPHABET_SIZE; i++) {
    if (node->children[i]) {
      prefix[depth] = i;
      prefix[depth + 1] = '\0';
      _traverse_trie_prefix(node->children[i], prefix, depth + 1, list);
    }
  }
  prefix[depth] = '\0';
}

CommandNameList *traverse_trie_prefix(const char *cmd_prefix) {
  TrieNode *node = _find_in_trie(root, cmd_prefix);
  if (!node) {
    return NULL;
  }

  CommandNameList *list = malloc(sizeof(CommandNameList));
  if (!list) {
    return NULL;
  }

  list->cmd_names = malloc(INITIAL_NAME_LIST_CAP * sizeof(char *));
  if (!list->cmd_names) {
    free(list);
    return NULL;
  }

  list->capacity = INITIAL_NAME_LIST_CAP;
  list->count = 0;

  char prefix[BUFSIZ];
  strncpy(prefix, cmd_prefix, sizeof(prefix) - 1);
  prefix[sizeof(prefix) - 1] = '\0';
  int prefix_len = strlen(prefix);

  _traverse_trie_prefix(node, prefix, prefix_len, list);

  return list;
}

void free_command_name_list(CommandNameList *list) {
  if (!list)
    return;
  for (int i = 0; i < list->count; i++) {
    free(list->cmd_names[i]);
  }
  free(list->cmd_names);
  free(list);
}
