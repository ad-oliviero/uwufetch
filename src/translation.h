#ifndef __TRANSLATION_H__
#define __TRANSLATION_H__
#include <stdint.h>
#include <stdlib.h>

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
