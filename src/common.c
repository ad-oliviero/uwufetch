#include "libfetch/logging.h"
#include <stdint.h>
#include <stdlib.h>

// calculate the id of the distro logo, https://en.wikipedia.org/wiki/Jenkins_hash_function
uint32_t str2id(const char* key, int length) {
  uint32_t hash = 0;
  for (int i = 0; i < length; ++i) {
    hash += (uint8_t)key[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);

  LOG_V(hash);
  return hash;
}

#ifdef _WIN32
// windows sucks and hasn't a strstep, so I copied one from https://stackoverflow.com/questions/8512958/is-there-a-windows-variant-of-strsep-function
char* strsep(char** stringp, const char* delim) {
  char* start = *stringp;
  char* p;
  p = (start != NULL) ? strpbrk(start, delim) : NULL;
  if (p == NULL)
    *stringp = NULL;
  else {
    *p       = '\0';
    *stringp = p + 1;
  }
  return start;
}
#endif
