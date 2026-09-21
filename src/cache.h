#ifndef CACHE_H
#define CACHE_H

#include "uwufetch.h"

void write_cache(struct info* user_info);
// on failure NULL is returned and errno is set
char* read_cache(struct info* user_info);

#endif // CACHE_H
