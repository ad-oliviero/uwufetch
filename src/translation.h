#ifndef __TRANSLATION_H__
#define __TRANSLATION_H__
#include <stdint.h>
#include <stdlib.h>

// calculate the id of the distro logo, taken from wikipedia
uint64_t jenkins_hash(const char*, size_t);
// Replaces all terms in a string with another term.
void replace(char*, const char*, const char*);
// Replaces all terms in a string with another term, case insensitive
void replace_ignorecase(char*, const char*, const char*);
#ifdef _WIN32
// windows sucks and hasn't a strstep, so I copied one from https://stackoverflow.com/questions/8512958/is-there-a-windows-variant-of-strsep-function
char* strsep(char**, const char*);
#endif
// uwufies distro name
void uwu_name(const struct actrie_t*, struct info*);
// uwufies kernel name
void uwu_kernel(const struct actrie_t*, char*);
// uwufies hardware names
void uwu_hw(const struct actrie_t*, char*);
// uwufies package manager names
void uwu_pkgman(const struct actrie_t*, char*);
// uwufies everything
void uwufy_all(struct info*);
#endif
