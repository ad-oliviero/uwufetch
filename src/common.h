#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdint.h>
// calculate the id of the distro logo, https://en.wikipedia.org/wiki/Jenkins_hash_function
uint32_t str2id(const char*, int);
#ifdef _WIN32
// windows sucks and hasn't a strstep, so I copied one from https://stackoverflow.com/questions/8512958/is-there-a-windows-variant-of-strsep-function
char* strsep(char**, const char*);
#endif
#endif // __COMMON_H__
