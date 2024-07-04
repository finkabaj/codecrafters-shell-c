#include "trie.h"
#include "cmds.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  return (node->is_cmd && node->cmd) ? node : NULL;
}

TrieNode *find_in_trie(const char *cmd_name) {
  return _find_in_trie(root, cmd_name);
}
