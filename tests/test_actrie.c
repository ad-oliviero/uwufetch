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
 * unit tests for the actrie module. Built in debug mode, so
 * actrie_t_check_computed_links() (the invariant sweep) runs on every trie
 * for free.
 */

#include "../src/actrie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;
#define CHECK(cond)                                                   \
  do {                                                                \
    if (!(cond)) {                                                    \
      fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
      failures++;                                                     \
    }                                                                 \
  } while (0)

// ---------- contains ----------

static void test_contains(void) {
  struct actrie_t t;
  actrie_t_ctor(&t);
  actrie_t_add_pattern(&t, "linux", "LinUwU");
  actrie_t_add_pattern(&t, "-}", "alphabet edges"); // first and last char of the alphabet
  actrie_t_compute_links(&t);

  CHECK(actrie_t_contains_pattern(&t, "linux")); // hit
  CHECK(actrie_t_contains_pattern(&t, "LINUX")); // case insensitive
  CHECK(actrie_t_contains_pattern(&t, "LiNuX"));
  CHECK(!actrie_t_contains_pattern(&t, "linu"));      // miss (not a whole word)
  CHECK(!actrie_t_contains_pattern(&t, "linuxx"));    // miss (longer)
  CHECK(actrie_t_contains_pattern(&t, "-}"));         // alphabet edges
  CHECK(!actrie_t_contains_pattern(&t, ""));          // empty pattern (never added)
  CHECK(!actrie_t_contains_pattern(&t, "lin\xE2ux")); // 0xE2 is not in the alphabet
  actrie_t_dtor(&t);
}

// ---------- longest match (the uwufy table) ----------

static void test_longest_match(void) {
  struct actrie_t t;
  actrie_t_ctor(&t);
  actrie_t_add_pattern(&t, "linux", "LinUwU");
  actrie_t_add_pattern(&t, "linuxmint", "LinUwU Miwint");
  actrie_t_add_pattern(&t, "manjaro", "Myanjawo");
  actrie_t_add_pattern(&t, "manjaro-arm", "Myanjawo AWM");
  actrie_t_compute_links(&t);

  char s[64] = "linuxmint"; // currently "LinUwUmint" without the longest match rule
  actrie_t_replace_all_occurances(&t, s);
  CHECK(strcmp(s, "LinUwU Miwint") == 0);

  char m[64] = "manjaro-arm"; // currently "Myanjawo-arm" without the longest match rule
  actrie_t_replace_all_occurances(&t, m);
  CHECK(strcmp(m, "Myanjawo AWM") == 0);

  char m2[64] = "manjaro"; // prefix pattern, NOT "... AWM"
  actrie_t_replace_all_occurances(&t, m2);
  CHECK(strcmp(m2, "Myanjawo") == 0);

  char l[64] = "linux";
  actrie_t_replace_all_occurances(&t, l);
  CHECK(strcmp(l, "LinUwU") == 0);

  actrie_t_dtor(&t);
}

// ---------- replacement edge cases ----------

static void test_replace_edges(void) {
  struct actrie_t t;
  actrie_t_ctor(&t);
  actrie_t_add_pattern(&t, "a", "xxxx"); // replacement grows
  actrie_t_add_pattern(&t, "bbbb", "y"); // replacement shrinks
  actrie_t_add_pattern(&t, "ab", "AB");  // same size
  actrie_t_add_pattern(&t, "", "empty"); // empty patterns never match a text scan
  actrie_t_compute_links(&t);

  char buf[64];
  memset(buf, 0xAA, sizeof(buf)); // canary
  strcpy(buf, "a bbbb a");
  size_t len = actrie_t_replace_all_occurances(&t, buf);
  CHECK(len == strlen("xxxx y xxxx"));
  CHECK(strcmp(buf, "xxxx y xxxx") == 0);                                     // growth and shrink together
  for (size_t i = len + 1; i < sizeof(buf); i++) CHECK(buf[i] == (char)0xAA); // canary intact
  CHECK(buf[len] == '\0');                                                    // NUL terminated within bounds

  memset(buf, 0xAA, sizeof(buf));
  strcpy(buf, "ab"); // "ab" must win over "a" (same start, longer)
  len = actrie_t_replace_all_occurances(&t, buf);
  CHECK(strcmp(buf, "AB") == 0);

  memset(buf, 0xAA, sizeof(buf));
  strcpy(buf, "xabcy"); // at position 1 "ab" (longest) wins over "a"
  actrie_t_replace_all_occurances(&t, buf);
  CHECK(strcmp(buf, "xABcy") == 0);

  memset(buf, 0xAA, sizeof(buf));
  strcpy(buf, "a\xE2"
              "a"); // 0xE2 is not in the alphabet: it must break any ongoing match
  actrie_t_replace_all_occurances(&t, buf);
  CHECK(strcmp(buf, "xxxx\xE2"
                    "xxxx") == 0);

  memset(buf, 0xAA, sizeof(buf));
  buf[0] = '\0'; // empty text
  len    = actrie_t_replace_all_occurances(&t, buf);
  CHECK(len == 0);

  actrie_t_dtor(&t);
}

static void test_overlapping_patterns(void) {
  struct actrie_t t;
  actrie_t_ctor(&t);
  actrie_t_add_pattern(&t, "ab", "X");
  actrie_t_add_pattern(&t, "abc", "YZ");
  actrie_t_add_pattern(&t, "bcd", "W");
  actrie_t_add_pattern(&t, "b", "b!"); // substring of both "ab" and "bcd"
  actrie_t_compute_links(&t);

  char s[64] = "zabcy"; // leftmost-longest: "abc" wins over "ab" and the inner "b"
  actrie_t_replace_all_occurances(&t, s);
  CHECK(strcmp(s, "zYZy") == 0);

  char s2[64] = "xbcx"; // "bcd" can not match: at position 1 the longest is "bc"... only "b" is a pattern here
  actrie_t_replace_all_occurances(&t, s2);
  CHECK(strcmp(s2, "xb!cx") == 0);

  actrie_t_dtor(&t);
}

// ---------- property tests ----------

// simple LCG: rand() is not portable, the tests must be reproducible
static unsigned rng_state = 0x12345678u;
static unsigned rnd(unsigned n) {
  rng_state = rng_state * 1103515245u + 12345u;
  return (rng_state >> 16) % n;
}

#define MAX_PATTERNS 6
#define MAX_PATTERN_LEN 8
#define MAX_REPLACER_LEN 6
#define MAX_TEXT_LEN 40
#define BUFFER_CAP 512
// lowercase letters only, so that the oracle can compare without case handling
static const char pattern_alphabet[]  = "abc-}";
static const char replacer_alphabet[] = "xy";

struct found_match {
  size_t start, length;
};
static struct found_match found[BUFFER_CAP];
static size_t found_count;

static void find_cb(void* context, const char* word, size_t word_length, size_t start) {
  (void)context;
  (void)word;
  found[found_count].start  = start;
  found[found_count].length = word_length;
  found_count++;
}

static void test_properties(void) {
  char patterns[MAX_PATTERNS][MAX_PATTERN_LEN + 1];
  size_t pattern_lens[MAX_PATTERNS];
  char replacers[MAX_PATTERNS][MAX_REPLACER_LEN + 1];
  size_t replacer_lens[MAX_PATTERNS];
  char text[BUFFER_CAP];
  char expected[BUFFER_CAP];

  for (unsigned round = 0; round < 500; round++) {
    size_t pattern_count = 1 + rnd(MAX_PATTERNS);
    struct actrie_t t;
    actrie_t_ctor(&t);
    actrie_t_reserve_patterns(&t, pattern_count);
    for (size_t i = 0; i < pattern_count; i++) {
      pattern_lens[i] = 1 + rnd(MAX_PATTERN_LEN);
      for (size_t j = 0; j < pattern_lens[i]; j++)
        patterns[i][j] = pattern_alphabet[rnd(sizeof(pattern_alphabet) - 1)];
      patterns[i][pattern_lens[i]] = '\0';
      replacer_lens[i]             = rnd(MAX_REPLACER_LEN + 1); // may be empty
      for (size_t j = 0; j < replacer_lens[i]; j++)
        replacers[i][j] = replacer_alphabet[rnd(sizeof(replacer_alphabet) - 1)];
      replacers[i][replacer_lens[i]] = '\0';
      actrie_t_add_pattern_len(&t, patterns[i], pattern_lens[i], replacers[i], replacer_lens[i]);
    }
    actrie_t_compute_links(&t); // runs the invariant sweep in debug builds

    size_t text_len = rnd(MAX_TEXT_LEN + 1);
    for (size_t i = 0; i < text_len; i++) {
      unsigned r = rnd(16);
      // 0xE2 (a byte outside the alphabet) from time to time, to test the reset path
      text[i] = r == 0 ? '\xE2' : pattern_alphabet[rnd(sizeof(pattern_alphabet) - 1)];
    }
    text[text_len] = '\0';

    // property 1: the match reported by run_text at each end position is the
    // longest pattern ending there
    found_count = 0;
    actrie_t_run_text(&t, text, NULL, find_cb);
    for (size_t end = 1; end <= text_len; end++) {
      size_t expected_len = 0; // longest pattern ending at "end" (brute force oracle)
      for (size_t i = 0; i < pattern_count; i++)
        if (pattern_lens[i] <= end && strncmp(text + end - pattern_lens[i], patterns[i], pattern_lens[i]) == 0)
          expected_len = expected_len < pattern_lens[i] ? pattern_lens[i] : expected_len;
      size_t reported_len = 0;
      for (size_t i = 0; i < found_count; i++)
        if (found[i].start + found[i].length == end) reported_len = found[i].length > reported_len ? found[i].length : reported_len;
      CHECK(reported_len == expected_len);
    }

    // property 2: replace_all must be deterministic, NUL terminated and
    // equal to the leftmost-longest replacement of the original text
    char copy1[BUFFER_CAP], copy2[BUFFER_CAP];
    memset(copy1, 0x5A, sizeof(copy1));
    memset(copy2, 0x5A, sizeof(copy2));
    memcpy(copy1, text, text_len + 1);
    memcpy(copy2, text, text_len + 1);
    size_t len1 = actrie_t_replace_all_occurances(&t, copy1);
    size_t len2 = actrie_t_replace_all_occurances(&t, copy2);
    CHECK(len1 == len2); // deterministic
    CHECK(memcmp(copy1, copy2, BUFFER_CAP) == 0);
    CHECK(copy1[len1] == '\0'); // NUL terminated within bounds
    // canary: only the bytes below max(old, new) length may change
    size_t used = len1 > text_len ? len1 : text_len;
    for (size_t i = used + 1; i < BUFFER_CAP; i++) CHECK(copy1[i] == (char)0x5A);

    // oracle: greedy leftmost-longest scan of the original text
    size_t o = 0, ti = 0;
    while (ti < text_len) {
      size_t best   = 0;
      size_t best_i = 0;
      for (size_t i = 0; i < pattern_count; i++) // the latest added replacer wins ties
        if (pattern_lens[i] > 0 && pattern_lens[i] <= text_len - ti &&
            strncmp(text + ti, patterns[i], pattern_lens[i]) == 0 && pattern_lens[i] >= best) {
          best   = pattern_lens[i];
          best_i = i;
        }
      if (best != 0) {
        memcpy(expected + o, replacers[best_i], replacer_lens[best_i]);
        o += replacer_lens[best_i];
        ti += best;
      } else {
        expected[o++] = text[ti++];
      }
    }
    expected[o] = '\0';
    CHECK(len1 == o); // length == input + sum(deltas)
    CHECK(memcmp(copy1, expected, o + 1) == 0);

    actrie_t_dtor(&t);
  }
}

int main(void) {
  test_contains();
  test_longest_match();
  test_replace_edges();
  test_overlapping_patterns();
  test_properties();
  if (failures == 0) fprintf(stderr, "test_actrie: all tests passed\n");
  return failures != 0;
}
