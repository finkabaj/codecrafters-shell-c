#include "trie.h"
#include "cmds.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TrieNode *root = NULL;

void free_trie(TrieNode *root) {
  if (root == NULL) {
    return;
  }

  for (int i = 0; i < ALPHABET_SIZE; i++) {
    free_trie(root->children[i]);
    root->children[i] = NULL;
  }

  free(root);
}

int char_to_index(char c) {
  if (islower(c)) {
    return c - 'a';
  } else if (isupper(c)) {
    return c - 'A' + 26;
  } else if (isdigit(c)) {
    return c - '0' + 52;
  }
  return -1;
}

int insert_cmd(TrieNode *root, const Command *cmd) {
  assert(root != NULL);
  assert(cmd != NULL);
  assert(cmd->name != NULL);

  const char *current = cmd->name;

  while (*current != '\0') {
    int index = char_to_index(*current);
    if (index == -1) {
      return 0;
    }

    if (root->children[index] == NULL) {
      root->children[index] = calloc(1, sizeof(TrieNode));
      if (root->children[index] == NULL) {
        return 0;
      }
    }

    root = root->children[index];
    current++;
  }

  root->is_cmd = 1;
  root->cmd = cmd;
  return 1;
}

int init_trie() {
  root = calloc(1, sizeof(TrieNode));

  if (root == NULL) {
    return 0;
  }

  for (size_t i = 0; i < cmds_count; i++) {
    if (!insert_cmd(root, &cmds[i])) {
      free_trie(root);
      root = NULL;
      return 0;
    }
  }

  return 1;
}

TrieNode *_find_in_trie(TrieNode *node, const char *cmd_name) {
  if (node == NULL || cmd_name == NULL) {
    return NULL;
  }

  while (*cmd_name != '\0') {
    int index = char_to_index(*cmd_name);
    if (index == -1) {
      return NULL;
    }
    if (node->children[index] == NULL) {
      return NULL;
    }
    node = node->children[index];
    cmd_name++;
  }

  return (node->is_cmd && node->cmd != NULL) ? node : NULL;
}

TrieNode *find_in_trie(const char *cmd_name) {
  return _find_in_trie(root, cmd_name);
}
