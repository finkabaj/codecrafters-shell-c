#ifndef TRIE_H
#define TRIE_H

#include "cmds.h"
#include <stddef.h>
#include <stdint.h>

#define ALPHABET_SIZE 62

typedef struct TrieNode TrieNode;

struct TrieNode {
  TrieNode *children[ALPHABET_SIZE];
  uint8_t is_cmd;
  const Command *cmd;
};

int init_trie();
TrieNode *find_in_trie(const char *cmd_name);

#endif // TRIE_H
