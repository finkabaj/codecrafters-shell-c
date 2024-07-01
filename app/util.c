#include <stdlib.h>

void free_ptr_to_str(int strc, char **strs) {
  if (!strs) {
    return;
  }

  for (int i = 0; i < strc; i++) {
    if (strs[i]) {
      free(strs[i]);
      strs[i] = NULL;
    }
  }
  free(strs);
}
