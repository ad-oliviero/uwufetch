/*
 *  UwUfetch is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/* NOTE:
 * simple random-loop fuzzer: no libFuzzer, the sanitizers (make test-asan)
 * do the actual bug detection.
 * usage: fuzz [seconds] [seed], the seed is printed so a crash can be
 * reproduced with the same arguments.
 */

#include "../src/actrie.h"
#include "../src/cache.h"
#include "../src/config.h"
#include "../src/uwufetch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

static unsigned long long rng_state;
static unsigned rnd(unsigned n) { // LCG, reproducible on any platform
  rng_state = rng_state * 6364136223846793005ULL + 1442695040888963407ULL;
  return (unsigned)((rng_state >> 33) % n);
}

static char home_dir[] = "/tmp/uwufetch_fuzz_XXXXXX";
static char config_path[512];
static char cache_path[512];

// the text alphabet includes a byte outside the actrie alphabet (\xE2), to
// exercise the match reset path
static const char text_alphabet[] = "abcxyz-}0\xE2";
// the patterns can only contain alphabet chars (the actrie rejects the others)
static const char pattern_alphabet[]   = "abcxyz-}";
static const char replacer_alphabet[]  = "xy}";
static const char* const config_keys[] = {
    "user_name", "os_name", "model", "kernel", "cpu", "gpu_list",
    "memory", "screen", "shell", "packages", "uptime", "colors"};

#define MAX_PATTERNS 8
#define MAX_PATTERN_LEN 10
#define MAX_REPLACER_LEN 8
#define MAX_TEXT_LEN 256
#define TEXT_BUFFER_CAP 4096

// random patterns and replacers into the actrie, then a replace into a canary buffer
static void fuzz_actrie(void) {
  struct actrie_t t;
  actrie_t_ctor(&t);
  size_t pattern_count = 1 + rnd(MAX_PATTERNS);
  actrie_t_reserve_patterns(&t, pattern_count);
  for (size_t i = 0; i < pattern_count; i++) {
    char pattern[MAX_PATTERN_LEN + 1]   = {0};
    char replacer[MAX_REPLACER_LEN + 1] = {0};
    size_t pattern_len                  = rnd(MAX_PATTERN_LEN + 1);
    size_t replacer_len                 = rnd(MAX_REPLACER_LEN + 1);
    for (size_t j = 0; j < pattern_len; j++) pattern[j] = pattern_alphabet[rnd(sizeof(pattern_alphabet) - 1)];
    for (size_t j = 0; j < replacer_len; j++) replacer[j] = replacer_alphabet[rnd(sizeof(replacer_alphabet) - 1)];
    actrie_t_add_pattern_len(&t, pattern, pattern_len, replacer, replacer_len);
  }
  actrie_t_compute_links(&t); // runs the invariant sweep in debug builds

  static char buffer[TEXT_BUFFER_CAP];
  memset(buffer, 0x5A, sizeof(buffer)); // canary
  size_t text_len = rnd(MAX_TEXT_LEN + 1);
  for (size_t i = 0; i < text_len; i++) buffer[i] = text_alphabet[rnd(sizeof(text_alphabet) - 1)];
  buffer[text_len] = '\0';

  size_t new_len = actrie_t_replace_all_occurances_len(&t, buffer, text_len);
  if (new_len >= TEXT_BUFFER_CAP) {
    fprintf(stderr, "FUZZ FAIL: replace_all returned %zu (buffer is %zu)\n", new_len, (size_t)TEXT_BUFFER_CAP);
    exit(1);
  }
  if (buffer[new_len] != '\0') {
    fprintf(stderr, "FUZZ FAIL: replace_all did not NUL terminate\n");
    exit(1);
  }

  char probe[MAX_PATTERN_LEN + 1] = {0};
  size_t probe_len                = rnd(MAX_PATTERN_LEN + 1);
  for (size_t i = 0; i < probe_len; i++) probe[i] = pattern_alphabet[rnd(sizeof(pattern_alphabet) - 1)];
  actrie_t_contains_pattern(&t, probe);

  actrie_t_dtor(&t);
}

// random key=value (and garbage) lines into parse_config
static void fuzz_config(void) {
  FILE* fp = fopen(config_path, "w");
  if (fp == NULL) return;
  unsigned lines = rnd(32);
  for (unsigned l = 0; l < lines; l++) {
    switch (rnd(8)) {
    case 0: // valid key, unquoted value
      fprintf(fp, "%s=%s\n", config_keys[rnd(sizeof(config_keys) / sizeof(config_keys[0]))],
              rnd(2) ? "true" : "false");
      break;
    case 1: // valid key, quoted value
      fprintf(fp, "%s=\"%s\"\n", config_keys[rnd(sizeof(config_keys) / sizeof(config_keys[0]))],
              rnd(2) ? "true" : "false");
      break;
    case 2: // logo line with a random value
      fprintf(fp, "logo=");
      for (unsigned i = 0; i < rnd(64); i++) fputc('a' + (int)rnd(26), fp);
      fputc('\n', fp);
      break;
    case 3: // binary garbage line
      for (unsigned i = 0; i < rnd(128); i++) fputc((int)(1 + rnd(255)), fp);
      fputc('\n', fp);
      break;
    default: // random junk
      for (unsigned i = 0; i < rnd(256); i++) fputc(' ' + (int)rnd(94), fp);
      fputc('\n', fp);
      break;
    }
  }
  fclose(fp);

  struct configuration configuration = {0};
  parse_config(&configuration, config_path);
  free(configuration.logo_name); // may have been set by a logo line
}

// random (or semi-valid) cache files into read_cache
static void fuzz_cache(void) {
  FILE* fp = fopen(cache_path, "wb");
  if (fp == NULL) return;
  unsigned long body_size = rnd(2048);
  switch (rnd(3)) {
  case 0: { // fully random bytes
    unsigned long file_size = 4 + body_size;
    for (unsigned long i = 0; i < file_size; i++) fputc((int)rnd(256), fp);
    break;
  }
  case 1: { // consistent header, random body: reaches the deep parsing code
    fputc((int)((4 + body_size) & 0xFF), fp);
    fputc((int)(((4 + body_size) >> 8) & 0xFF), fp);
    fputc((int)(((4 + body_size) >> 16) & 0xFF), fp);
    fputc((int)(((4 + body_size) >> 24) & 0xFF), fp);
    for (unsigned long i = 0; i < body_size; i++) fputc(1 + (int)rnd(255), fp);
    break;
  }
  default: { // header claims more than the file has: truncation path
    unsigned long file_size = 4 + body_size;
    for (unsigned long i = 0; i < file_size; i++) fputc((int)rnd(256), fp);
    unsigned long claimed = file_size + 1 + rnd(100);
    fseek(fp, 0, SEEK_SET);
    fputc((int)(claimed & 0xFF), fp);
    fputc((int)((claimed >> 8) & 0xFF), fp);
    fputc((int)((claimed >> 16) & 0xFF), fp);
    fputc((int)((claimed >> 24) & 0xFF), fp);
    break;
  }
  }
  fclose(fp);

  struct info user_info = {0};
  char* content         = read_cache(&user_info);
  if (content != NULL) { // anything returned must be freeable
    free(user_info.gpu_list);
    free(content);
  }
}

int main(int argc, char** argv) {
  unsigned seconds = argc > 1 ? (unsigned)atoi(argv[1]) : 5;
  rng_state        = argc > 2 ? strtoull(argv[2], NULL, 10) : (unsigned long long)time(NULL);

  if (mkdtemp(home_dir) == NULL) return 1;
  char cache_dir[512];
  snprintf(cache_dir, sizeof(cache_dir), "%s/.cache", home_dir);
  mkdir(cache_dir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  snprintf(config_path, sizeof(config_path), "%s/config", home_dir);
  snprintf(cache_path, sizeof(cache_path), "%s/uwufetch.cache", cache_dir);
  setenv("HOME", home_dir, 1);

  printf("fuzzing for %us (seed %llu)...\n", seconds, rng_state);
  time_t deadline               = time(NULL) + seconds;
  unsigned long long iterations = 0;
  while (time(NULL) < deadline) {
    fuzz_actrie();
    fuzz_config();
    fuzz_cache();
    iterations++;
  }
  printf("done: %llu iterations\n", iterations);

  unlink(cache_path);
  unlink(config_path);
  rmdir(cache_dir);
  rmdir(home_dir);
  return 0;
}
