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

#ifndef _ACTRIE_H_
#define _ACTRIE_H_

#include <assert.h>  // static_assert
#include <limits.h>  // CHAR_MAX
#include <stdbool.h> // bool, true
#include <stddef.h>  // size_t
#include <stdint.h>  // uint32_t
#include <stdlib.h>  // abort
#include <string.h>  // strlen

/// @brief A function called when memory allocation failed
inline static void out_of_memory_handler(void) {
  abort();
}

enum actrie_constants {
  ALPHABET_START             = '-',
  ALPHABET_END               = '}',
  ALPHABET_LENGTH            = ALPHABET_END - ALPHABET_START + 1,
  IS_FIND_IGNORECASE_MODE_ON = true,
};

static_assert(IS_FIND_IGNORECASE_MODE_ON == true || IS_FIND_IGNORECASE_MODE_ON == false, "IS_FIND_IGNORECASE_MODE_ON must be bool");
static_assert(ALPHABET_START <= 'a' && 'z' <= ALPHABET_END, "Symbols [a; z] must be in the alphabet");
static_assert(ALPHABET_START > '\0' && ALPHABET_END <= CHAR_MAX);

#if defined(__thiscall)
  #define actrie_thiscall __thiscall
#elif defined(__fastcall)
  #define actrie_thiscall __fastcall
#else
  #define actrie_thiscall
#endif

struct actrie_node_t {
  // Indexes in array of nodes
  uint32_t edges[ALPHABET_LENGTH];

  // Index in array of nodes
  uint32_t suffix_link;

  // Index in array of nodes
  uint32_t compressed_suffix_link;

  /* Index of the word in the Trie which ends on this node and replacement word in the Trie for word ends on this node
   * MISSING_SENTIEL if node is not terminal
   */
  uint32_t word_index;
};

struct string_t {
  const char* c_str;
  size_t size;
};

struct vector_actrie_node_t {
  struct actrie_node_t* data;
  size_t size;
  size_t capacity;
};

struct vector_uint32_t {
  uint32_t* data;
  size_t size;
  size_t capacity;
};

struct vector_string_t {
  struct string_t* data;
  size_t size;
  size_t capacity;
};

struct actrie_t {
  // vector<actrie_node_t>
  struct vector_actrie_node_t nodes;
  // vector<uint32_t>
  struct vector_uint32_t words_lengths;
  // vector<string_t>
  struct vector_string_t words_replacement;
  bool are_links_computed;
};

/// @brief actrie_t type constructor
/// @param this_ actrie
/// @return
void actrie_thiscall actrie_t_ctor(struct actrie_t* this_);

/// @brief actrie_t type destructor
/// @param this_ actrie
/// @return
void actrie_thiscall actrie_t_dtor(struct actrie_t* this_);

/// @brief Reserves actrie's internal vector with patterns for patterns_capacity patterns to add
/// @param this_ actrie
/// @param patterns_capacity amount of patterns that will be added
/// @return
void actrie_thiscall actrie_t_reserve_patterns(struct actrie_t* this_, size_t patterns_capacity);

/// @brief Adds pattern to the trie in O(pattern_size)
/// @param this_ actrie
/// @param pattern c string representing a pattern
/// @param pattern_size length of the pattern
/// @param replacer c string representing a replacer for the pattern
/// @param replacer_size length of the replacer
/// @return
void actrie_thiscall actrie_t_add_pattern_len(struct actrie_t* this_, const char* pattern, size_t pattern_size, const char* replacer, size_t replacer_size);

/// @brief Adds pattern to the trie in O(strlen(pattern))
/// @param this_ actrie
/// @param pattern c string representing a pattern
/// @param replacer c string representing a replacer for the pattern
/// @return
static inline void actrie_thiscall actrie_t_add_pattern(struct actrie_t* this_, const char* pattern, const char* replacer) {
  actrie_t_add_pattern_len(this_, pattern, strlen(pattern), replacer, strlen(replacer));
}

/// @brief Aho–Corasick deterministic finite-state machine is built on the ordinal trie so it can be used as ordinal trie
/// @param this_ actrie
/// @param pattern
/// @return
bool actrie_thiscall actrie_t_contains_pattern(const struct actrie_t* this_, const char* pattern);

/// @brief Computes compressed suffix links. This is necessary to do before find and/or replace any patterns in texts.
/// @param this_ actrie
/// @return
void actrie_thiscall actrie_t_compute_links(struct actrie_t* this_);

static inline bool actrie_t_is_ready(const struct actrie_t* this_) {
  return this_->are_links_computed;
}

static inline size_t actrie_t_patterns_size(const struct actrie_t* this_) {
  return this_->words_lengths.size;
}

static inline size_t actrie_t_nodes_size(const struct actrie_t* this_) {
  return this_->nodes.size;
}

/// @brief Callback that is called when the next occurrence is found in the
///        actrie_t_run_text(const struct actrie_t*, const char*, FindCallback)
typedef void (*FindCallback)(void* context, const char* found_word, size_t word_length, size_t start_index_in_original_text);

/// @brief Find all occurances of any pattern (defined in this ac trie) in the given text in O(strlen(text))
/// @param this_ actrie
/// @param text Text to search pattern occurances in
/// @param callback_context Context passed to the callback
/// @param find_callback Callback that is called when the next occurrence is found
/// @return
void actrie_thiscall actrie_t_run_text(const struct actrie_t* this_, const char* text, void* callback_context, FindCallback find_callback);

/// @brief Replace first occurance of any pattern (defined in this ac trie) found in the given string
///        in O(length + |replacement_length - first_occurance_length|)
/// @param c_string string where first occurance should be replaced
/// @param length length of the string
/// @return new length of the string (length changes if occurance is found and strlen(occurance_pattern) != strlen(replacement))
///         !!! It is caller's responsobility to ensure that c_string buffer will not overflow if length growths
size_t actrie_thiscall actrie_t_replace_first_occurance_len(const struct actrie_t* this_, char* c_string, size_t length);

/// @brief Replace first occurance of any pattern (defined in this ac trie) found in the given string
///        in O(strlen(c_string) + |replacement_length - first_occurance_length|)
/// @param c_string string where first occurance should be replaced
/// @return new length of the string (length changes if occurance is found and strlen(occurance_pattern) != strlen(replacement))
///         !!! It is caller's responsobility to ensure that c_string buffer will not overflow if length growths
static inline size_t actrie_thiscall actrie_t_replace_first_occurance(const struct actrie_t* this_, char* c_string) {
  return actrie_t_replace_first_occurance_len(this_, c_string, strlen(c_string));
}

/// @brief Replace all occurances of any pattern (defined in this ac trie) found in the given string
///        in O(length + sum( |replacement_length - occurance_length| for each pattern occurance) )
/// @param c_string string where first occurance should be replaced
/// @param length length of the string
/// @return new length of the string (length changes if for any pattern strlen(pattern[i]) != strlen(replacement[i]))
///         !!! It is caller's responsobility to ensure that c_string buffer will not overflow if length growths
size_t actrie_thiscall actrie_t_replace_all_occurances_len(const struct actrie_t* this_, char* c_string, size_t length);

/// @brief Replace all occurances of any pattern (defined in this ac trie) found in the given string
///        in O(strlen(c_string) + sum( |replacement_length - occurance_length| for each pattern occurance) )
/// @param c_string string where first occurance should be replaced
/// @return new length of the string (length changes if for any pattern strlen(pattern[i]) != strlen(replacement[i]))
///         !!! It is caller's responsobility to ensure that c_string buffer will not overflow if length growths
static inline size_t actrie_thiscall actrie_t_replace_all_occurances(const struct actrie_t* this_, char* c_string) {
  return actrie_t_replace_all_occurances_len(this_, c_string, strlen(c_string));
}

#include <assert.h> // assert, static_assert
#include <stdlib.h> // malloc, free
#include <string.h> // memset

#include "actrie.h"

#if defined(__DEBUG__)
  #define actrie_assert(Expr) assert(Expr)
#else
  #define actrie_assert(Expr)
#endif

#if defined(__GNUC__)
  #define actrie_noinline __attribute__((__noinline__))
#elif defined(_MSC_VER)
  #define actrie_noinline __declspec(noinline)
#else
  #define actrie_noinline
#endif

#if defined(__GNUC__)
  #pragma GCC diagnostic ignored "-Wpedantic" // MISSING_SENTIEL and DEFAULT_VECTOR_CAPACITY are not int
#endif

enum actrie_constants_internal {
  FAKE_PREROOT_INDEX      = 1,
  ROOT_INDEX              = 2,
  NULL_NODE_INDEX         = 0,
  MISSING_SENTIEL         = (uint32_t)(-1),
  DEFAULT_VECTOR_CAPACITY = (size_t)(16)
};

#if defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif

static inline void actrie_thiscall actrie_node_t_ctor(struct actrie_node_t* this_) {
  memset((void*)this_->edges, (int)NULL_NODE_INDEX, sizeof(this_->edges));
  this_->suffix_link            = NULL_NODE_INDEX;
  this_->compressed_suffix_link = NULL_NODE_INDEX;
  this_->word_index             = MISSING_SENTIEL;
}

static inline bool actrie_thiscall actrie_node_t_is_terminal(const struct actrie_node_t* this_) {
  return this_->word_index != MISSING_SENTIEL;
}

static inline void actrie_thiscall string_t_ctor1(struct string_t* this_, const char* c_str) {
  this_->size = 0;
  if (c_str == NULL) {
    this_->c_str = NULL;
    return;
  }

  size_t size  = strlen(c_str);
  char* data   = (char*)malloc(size * sizeof(char));
  this_->c_str = data;
  if (data == NULL) {
    out_of_memory_handler();
    return;
  }

  strcpy(data, c_str);
  this_->size = size;
}

static inline void actrie_thiscall string_t_ctor2(struct string_t* this_, const char* c_str, size_t size) {
  this_->size = 0;
  if (c_str == NULL) {
    this_->c_str = NULL;
    return;
  }

  char* data   = (char*)malloc(size * sizeof(char));
  this_->c_str = data;
  if (data == NULL) {
    out_of_memory_handler();
    return;
  }

  strncpy(data, c_str, size);
  this_->size = size;
}

static inline void actrie_thiscall string_t_dtor(struct string_t* this_) {
  this_->size = 0;
  if (this_->c_str != NULL) {
    free((void*)this_->c_str);
    this_->c_str = NULL;
  }
}

static inline void actrie_thiscall vector_actrie_node_t_ctor1(struct vector_actrie_node_t* this_, size_t initial_size) {
  size_t capacity            = ((size_t)DEFAULT_VECTOR_CAPACITY >= initial_size) ? (size_t)DEFAULT_VECTOR_CAPACITY : initial_size;
  struct actrie_node_t* data = this_->data = (struct actrie_node_t*)malloc(capacity * sizeof(struct actrie_node_t));
  if (data == NULL) {
    out_of_memory_handler();
    return;
  }

  for (const struct actrie_node_t* data_end = data + initial_size; data != data_end; ++data) {
    actrie_node_t_ctor(data);
  }

  this_->size     = initial_size;
  this_->capacity = capacity;
}

static inline void actrie_thiscall vector_actrie_node_t_dtor(struct vector_actrie_node_t* this_) {
  this_->size     = 0;
  this_->capacity = 0;
  if (this_->data != NULL) {
    free(this_->data);
    this_->data = NULL;
  }
}

static void actrie_thiscall vector_actrie_node_t_reserve(struct vector_actrie_node_t* this_, size_t new_capacity) {
  if (this_->capacity >= new_capacity) {
    return;
  }

  struct actrie_node_t* new_data = (struct actrie_node_t*)malloc(new_capacity * sizeof(struct actrie_node_t));
  if (new_data == NULL) {
    out_of_memory_handler();
    return;
  }

  memcpy(new_data, this_->data, this_->size * sizeof(struct actrie_node_t));
  free(this_->data);
  this_->data     = new_data;
  this_->capacity = new_capacity;
}

static void actrie_thiscall actrie_noinline vector_actrie_node_t_emplace_back_with_resize(struct vector_actrie_node_t* this_) {
  size_t size     = this_->size;
  size_t capacity = this_->capacity;
  static_assert(DEFAULT_VECTOR_CAPACITY > 0);
  capacity *= 2; // Must not be 0, because capacity >= DEFAULT_VECTOR_CAPACITY > 0
  struct actrie_node_t* new_data = (struct actrie_node_t*)malloc(sizeof(struct actrie_node_t) * capacity);
  if (new_data == NULL) {
    out_of_memory_handler();
    return;
  }

  memcpy(new_data, this_->data, size * sizeof(struct actrie_node_t));
  actrie_node_t_ctor(new_data + size);

  free(this_->data);
  this_->data     = new_data;
  this_->size     = size + 1;
  this_->capacity = capacity;
}

static inline void actrie_thiscall vector_actrie_node_t_emplace_back(struct vector_actrie_node_t* this_) {
  size_t size     = this_->size;
  size_t capacity = this_->capacity;
  if (size < capacity) {
    actrie_node_t_ctor(this_->data + size);
    this_->size = size + 1;
    return;
  }

  // Rare case
  vector_actrie_node_t_emplace_back_with_resize(this_);
}

static inline void actrie_thiscall vector_uint32_t_ctor(struct vector_uint32_t* this_) {
  this_->data     = (uint32_t*)malloc(DEFAULT_VECTOR_CAPACITY * sizeof(uint32_t));
  this_->size     = 0;
  this_->capacity = DEFAULT_VECTOR_CAPACITY;
  if (this_->data == NULL) {
    this_->capacity = 0;
    out_of_memory_handler();
  }
}

static inline void actrie_thiscall vector_uint32_t_dtor(struct vector_uint32_t* this_) {
  this_->size     = 0;
  this_->capacity = 0;
  if (this_->data != NULL) {
    free(this_->data);
    this_->data = NULL;
  }
}

static inline void actrie_thiscall vector_uint32_t_reserve(struct vector_uint32_t* this_, size_t new_capacity) {
  if (this_->capacity >= new_capacity) {
    return;
  }

  uint32_t* new_data = (uint32_t*)malloc(new_capacity * sizeof(uint32_t));
  if (new_data == NULL) {
    out_of_memory_handler();
    return;
  }

  memcpy(new_data, this_->data, this_->size * sizeof(uint32_t));
  free(this_->data);
  this_->data     = new_data;
  this_->capacity = new_capacity;
}

static void actrie_thiscall actrie_noinline vector_uint32_t_emplace_back1_with_resize(struct vector_uint32_t* this_, uint32_t arg0) {
  size_t size     = this_->size;
  size_t capacity = this_->capacity;

  static_assert(DEFAULT_VECTOR_CAPACITY > 0);
  actrie_assert(capacity >= DEFAULT_VECTOR_CAPACITY);
  capacity *= 2; // Must not be 0, because capacity >= DEFAULT_VECTOR_CAPACITY > 0
  uint32_t* new_data = (uint32_t*)malloc(sizeof(uint32_t) * capacity);
  if (new_data == NULL) {
    out_of_memory_handler();
    return;
  }

  memcpy(new_data, this_->data, size * sizeof(uint32_t));
  *(new_data + size) = arg0;

  free(this_->data);
  this_->data     = new_data;
  this_->size     = size + 1;
  this_->capacity = capacity;
}

static inline void actrie_thiscall vector_uint32_t_emplace_back1(struct vector_uint32_t* this_, uint32_t arg0) {
  size_t size     = this_->size;
  size_t capacity = this_->capacity;
  if (size < capacity) {
    *(this_->data + size) = arg0;
    this_->size           = size + 1;
    return;
  }

  // Rare case
  vector_uint32_t_emplace_back1_with_resize(this_, arg0);
}

static inline void actrie_thiscall vector_string_t_ctor(struct vector_string_t* this_) {
  this_->data     = (struct string_t*)malloc(DEFAULT_VECTOR_CAPACITY * sizeof(struct string_t));
  this_->size     = 0;
  this_->capacity = DEFAULT_VECTOR_CAPACITY;
  if (this_->data == NULL) {
    this_->capacity = 0;
    out_of_memory_handler();
  }
}

static inline void actrie_thiscall vector_string_t_dtor(struct vector_string_t* this_) {
  struct string_t* data = this_->data;
  if (data != NULL) {
    for (const struct string_t* data_end = data + (this_->size); data != data_end; ++data) {
      string_t_dtor(data);
    }

    free(this_->data);
    this_->data = NULL;
  }

  this_->size = this_->capacity = 0;
}

static inline void actrie_thiscall vector_string_t_reserve(struct vector_string_t* this_, size_t new_capacity) {
  if (this_->capacity >= new_capacity) {
    return;
  }

  struct string_t* new_data = (struct string_t*)malloc(new_capacity * sizeof(struct string_t));
  if (new_data == NULL) {
    out_of_memory_handler();
    return;
  }

  memcpy(new_data, this_->data, this_->size * sizeof(struct string_t));
  free(this_->data);
  this_->data     = new_data;
  this_->capacity = new_capacity;
}

static void actrie_thiscall actrie_noinline vector_string_t_emplace_back2_with_resize(struct vector_string_t* this_, const char* c_string, size_t c_string_size) {
  size_t size     = this_->size;
  size_t capacity = this_->capacity;

  static_assert(DEFAULT_VECTOR_CAPACITY > 0);
  actrie_assert(capacity >= DEFAULT_VECTOR_CAPACITY);
  capacity *= 2; // Must not be 0, because capacity >= DEFAULT_VECTOR_CAPACITY > 0
  struct string_t* new_data = (struct string_t*)malloc(sizeof(struct string_t) * capacity);
  if (new_data == NULL) {
    out_of_memory_handler();
    return;
  }

  memcpy(new_data, this_->data, size * sizeof(struct string_t));
  string_t_ctor2(new_data + size, c_string, c_string_size);

  free(this_->data);
  this_->data     = new_data;
  this_->size     = size + 1;
  this_->capacity = capacity;
}

static inline void actrie_thiscall vector_string_t_emplace_back2(struct vector_string_t* this_, const char* c_string, size_t c_string_size) {
  size_t size = this_->size;
  if (size < this_->capacity) {
    string_t_ctor2(this_->data + size, c_string, c_string_size);
    this_->size = size + 1;
    return;
  }

  vector_string_t_emplace_back2_with_resize(this_, c_string, c_string_size);
}

struct replacement_info_t {
  uint32_t word_l_index_in_text;
  uint32_t word_index;
};

struct vector_replacement_info_t {
  struct replacement_info_t* data;
  size_t size;
  size_t capacity;
};

static inline void actrie_thiscall vector_replacement_info_t_ctor(struct vector_replacement_info_t* this_) {
  this_->data     = (struct replacement_info_t*)malloc(DEFAULT_VECTOR_CAPACITY * sizeof(struct replacement_info_t));
  this_->size     = 0;
  this_->capacity = DEFAULT_VECTOR_CAPACITY;
  if (this_->data == NULL) {
    this_->capacity = 0;
    out_of_memory_handler();
  }
}

static inline void actrie_thiscall vector_replacement_info_t_dtor(struct vector_replacement_info_t* this_) {
  struct replacement_info_t* data = this_->data;
  if (data != NULL) {
    free(data);
    this_->data = NULL;
  }

  this_->size = this_->capacity = 0;
}

static void actrie_thiscall actrie_noinline vector_replacement_info_t_emplace_back2_with_resize(struct vector_replacement_info_t* this_, uint32_t word_l_index_in_text, uint32_t word_index) {
  size_t size     = this_->size;
  size_t capacity = this_->capacity;

  static_assert(DEFAULT_VECTOR_CAPACITY > 0);
  actrie_assert(capacity >= DEFAULT_VECTOR_CAPACITY);
  capacity *= 2; // Must not be 0, because capacity >= DEFAULT_VECTOR_CAPACITY > 0
  struct replacement_info_t* new_data = (struct replacement_info_t*)malloc(sizeof(struct replacement_info_t) * capacity);
  if (new_data == NULL) {
    out_of_memory_handler();
    return;
  }

  memcpy(new_data, this_->data, size * sizeof(struct string_t));
  new_data[size].word_l_index_in_text = word_l_index_in_text;
  new_data[size].word_index           = word_index;

  free(this_->data);
  this_->data     = new_data;
  this_->size     = size + 1;
  this_->capacity = capacity;
}

static inline void actrie_thiscall vector_replacement_info_t_emplace_back2(struct vector_replacement_info_t* this_, uint32_t word_l_index_in_text, uint32_t word_index) {
  size_t size = this_->size;
  if (size < this_->capacity) {
    (this_->data + size)->word_l_index_in_text = word_l_index_in_text;
    (this_->data + size)->word_index           = word_index;
    this_->size                                = size + 1;
    return;
  }

  // Rare case
  vector_replacement_info_t_emplace_back2_with_resize(this_, word_l_index_in_text, word_index);
}

static inline bool is_in_alphabet(char c) {
  return (uint32_t)(c)-ALPHABET_START <= ALPHABET_END - ALPHABET_START;
}

static inline bool is_upper(char c) {
  return (uint32_t)(c) - 'A' <= 'Z' - 'A';
}

static inline bool is_lower(char c) {
  return (uint32_t)(c) - 'a' <= 'z' - 'a';
}

static inline char to_upper(char c) {
  return (char)((c) - (('a' - 'A') * is_lower(c)));
}

static inline char to_lower(char c) {
  return (char)((c) | (('a' - 'A') * is_upper(c)));
}

static inline size_t char_to_edge_index(char c) {
  return (size_t)(uint8_t)(c)-ALPHABET_START;
}

#define constexpr_is_upper(c) ((uint32_t)(c) - 'A' <= 'Z' - 'A')
#define constexpr_is_lower(c) ((uint32_t)(c) - 'a' <= 'z' - 'a')
#define constexpr_to_upper(c) (char)((c) - (('a' - 'A') * constexpr_is_lower(c)))
#define constexpr_to_lower(c) (char)((c) | (('a' - 'A') * constexpr_is_upper(c)))
#define constexpr_char_to_edge_index(c) ((size_t)(uint8_t)(c)-ALPHABET_START)

static_assert(constexpr_to_upper('\0') == '\0');
static_assert(constexpr_to_upper('0') == '0');
static_assert(constexpr_to_upper('9') == '9');
static_assert(constexpr_to_upper('A') == 'A');
static_assert(constexpr_to_upper('Z') == 'Z');
static_assert(constexpr_to_upper('a') == 'A');
static_assert(constexpr_to_upper('z') == 'Z');
static_assert(constexpr_to_upper('~') == '~');
static_assert(constexpr_to_upper('{') == '{');
static_assert(constexpr_to_upper('}') == '}');

static_assert(constexpr_to_lower('\0') == '\0');
static_assert(constexpr_to_lower('0') == '0');
static_assert(constexpr_to_lower('9') == '9');
static_assert(constexpr_to_lower('A') == 'a');
static_assert(constexpr_to_lower('Z') == 'z');
static_assert(constexpr_to_lower('a') == 'a');
static_assert(constexpr_to_lower('z') == 'z');
static_assert(constexpr_to_lower('~') == '~');
static_assert(constexpr_to_lower('{') == '{');
static_assert(constexpr_to_lower('}') == '}');

static_assert(constexpr_char_to_edge_index(constexpr_to_upper('A')) == constexpr_char_to_edge_index('A'));
static_assert(constexpr_char_to_edge_index(constexpr_to_upper('Z')) == constexpr_char_to_edge_index('Z'));
static_assert(constexpr_char_to_edge_index(constexpr_to_upper('a')) == constexpr_char_to_edge_index('A'));
static_assert(constexpr_char_to_edge_index(constexpr_to_upper('z')) == constexpr_char_to_edge_index('Z'));
static_assert(constexpr_char_to_edge_index(constexpr_to_upper('~')) == constexpr_char_to_edge_index('~'));
static_assert(constexpr_char_to_edge_index(constexpr_to_upper('{')) == constexpr_char_to_edge_index('{'));
static_assert(constexpr_char_to_edge_index(constexpr_to_upper('}')) == constexpr_char_to_edge_index('}'));

static_assert(constexpr_char_to_edge_index(constexpr_to_lower('A')) == constexpr_char_to_edge_index('a'));
static_assert(constexpr_char_to_edge_index(constexpr_to_lower('Z')) == constexpr_char_to_edge_index('z'));
static_assert(constexpr_char_to_edge_index(constexpr_to_lower('a')) == constexpr_char_to_edge_index('a'));
static_assert(constexpr_char_to_edge_index(constexpr_to_lower('z')) == constexpr_char_to_edge_index('z'));
static_assert(constexpr_char_to_edge_index(constexpr_to_lower('~')) == constexpr_char_to_edge_index('~'));
static_assert(constexpr_char_to_edge_index(constexpr_to_lower('{')) == constexpr_char_to_edge_index('{'));
static_assert(constexpr_char_to_edge_index(constexpr_to_lower('}')) == constexpr_char_to_edge_index('}'));

#undef constexpr_char_to_edge_index
#undef constexpr_to_lower
#undef constexpr_to_upper
#undef constexpr_is_lower
#undef constexpr_is_upper

void actrie_thiscall actrie_t_ctor(struct actrie_t* this_) {
  vector_actrie_node_t_ctor1(&this_->nodes, 3);
  vector_uint32_t_ctor(&this_->words_lengths);
  vector_string_t_ctor(&this_->words_replacement);

  /* link(root) = fake_vertex;
   * For all chars from the alphabet: fake_vertex ---char--> root
   */

  struct actrie_node_t* root   = &this_->nodes.data[ROOT_INDEX];
  root->suffix_link            = FAKE_PREROOT_INDEX;
  root->compressed_suffix_link = ROOT_INDEX;

  struct actrie_node_t* fake_preroot = &this_->nodes.data[FAKE_PREROOT_INDEX];
  for (uint32_t *iter = fake_preroot->edges, *end = iter + ALPHABET_LENGTH; iter != end; ++iter) {
    *iter = ROOT_INDEX;
  }

  this_->are_links_computed = false;
}

void actrie_thiscall actrie_t_dtor(struct actrie_t* this_) {
  vector_actrie_node_t_dtor(&this_->nodes);
  vector_uint32_t_dtor(&this_->words_lengths);
  vector_string_t_dtor(&this_->words_replacement);
  this_->are_links_computed = false;
}

void actrie_thiscall actrie_t_reserve_patterns(struct actrie_t* this_, size_t patterns_capacity) {
  vector_uint32_t_reserve(&this_->words_lengths, this_->words_lengths.size + patterns_capacity);
  vector_string_t_reserve(&this_->words_replacement, this_->words_replacement.size + patterns_capacity);
}

void actrie_thiscall actrie_t_add_pattern_len(struct actrie_t* this_, const char* pattern, size_t pattern_size, const char* replacer, size_t replacer_size) {
  uint32_t current_node_index   = ROOT_INDEX;
  const char* pattern_iter      = pattern;
  const char* const pattern_end = pattern + pattern_size;

  for (; pattern_iter != pattern_end; ++pattern_iter) {
    char c = IS_FIND_IGNORECASE_MODE_ON ? to_lower(*pattern_iter) : *pattern_iter;
    if (!is_in_alphabet(c)) {
      actrie_assert(!"char in pattern is not in alphabet!!!");
      continue;
    }

    uint32_t next_node_index = this_->nodes.data[current_node_index].edges[char_to_edge_index(c)];
    if (next_node_index != NULL_NODE_INDEX) {
      current_node_index = next_node_index;
    } else {
      break;
    }
  }

  size_t lasted_max_length = (size_t)(pattern_end - pattern_iter);
  vector_actrie_node_t_reserve(&this_->nodes, this_->nodes.size + lasted_max_length);

  /* Inserts substring [i..length - 1] of pattern if i < length (<=> i != length)
   * If i == length, then for cycle will no execute
   */
  for (; pattern_iter != pattern_end; ++pattern_iter) {
    char c = IS_FIND_IGNORECASE_MODE_ON ? to_lower(*pattern_iter) : *pattern_iter;
    if (!is_in_alphabet(c)) {
      actrie_assert(!"char in pattern is not in alphabet!!!");
      continue;
    }

    uint32_t new_node_index = (uint32_t)this_->nodes.size;
    vector_actrie_node_t_emplace_back(&this_->nodes);
    this_->nodes.data[current_node_index].edges[char_to_edge_index(c)] = new_node_index;
    current_node_index                                                 = new_node_index; // Actually new_node_index = current_node_index + 1
  }

  uint32_t word_index                              = (uint32_t)(this_->words_lengths.size);
  this_->nodes.data[current_node_index].word_index = word_index;
  vector_uint32_t_emplace_back1(&this_->words_lengths, (uint32_t)pattern_size);
  vector_string_t_emplace_back2(&this_->words_replacement, replacer, replacer_size);
}

bool actrie_thiscall actrie_t_contains_pattern(const struct actrie_t* this_, const char* pattern) {
  uint32_t current_node_index         = ROOT_INDEX;
  const struct actrie_node_t* m_nodes = this_->nodes.data;

  for (char sigma = IS_FIND_IGNORECASE_MODE_ON ? to_lower(*pattern) : *pattern;
       sigma != '\0';
       ++pattern, sigma = IS_FIND_IGNORECASE_MODE_ON ? to_lower(*pattern) : *pattern) {
    if (!is_in_alphabet(sigma)) {
      return false;
    }

    uint32_t next_node_index = m_nodes[current_node_index].edges[char_to_edge_index(sigma)];
    if (next_node_index != NULL_NODE_INDEX) {
      current_node_index = next_node_index;
    } else {
      return false;
    }
  }

  return actrie_node_t_is_terminal(&m_nodes[current_node_index]);
}

#if defined(__DEBUG__)
static inline void actrie_thiscall actrie_t_check_computed_links(struct actrie_t* this_) {
  const struct actrie_node_t* iter     = this_->nodes.data;
  const struct actrie_node_t* iter_end = iter + this_->nodes.size;

  uint32_t max_node_index_excl = (uint32_t)(this_->nodes.size);
  actrie_assert(max_node_index_excl >= 3);
  uint32_t max_word_end_index_excl = (uint32_t)(this_->words_lengths.size);
  actrie_assert(this_->words_replacement.size == max_word_end_index_excl);

  ++iter;
  // Now iter points to fake preroot node
  // fake preroot node does not have suffix_link_index and compressed_suffix_link
  // all children point to root
  for (const uint32_t *child_start = iter->edges, *child_end = child_start + sizeof(iter->edges) / sizeof(iter->edges[0]); child_start != child_end; ++child_start) {
    actrie_assert(*child_start == ROOT_INDEX);
  }

  for (++iter; iter != iter_end; ++iter) {
    for (const uint32_t *child_start = iter->edges, *child_end = child_start + sizeof(iter->edges) / sizeof(iter->edges[0]); child_start != child_end; ++child_start) {
      uint32_t child = *child_start;
      actrie_assert(child >= FAKE_PREROOT_INDEX && child < max_node_index_excl);
    }

    uint32_t suffix_link_index = iter->suffix_link;
    actrie_assert(suffix_link_index >= FAKE_PREROOT_INDEX && suffix_link_index < max_node_index_excl);

    uint32_t compressed_suffix_link_index = iter->compressed_suffix_link;
    actrie_assert(compressed_suffix_link_index >= FAKE_PREROOT_INDEX && compressed_suffix_link_index < max_node_index_excl);

    actrie_assert(actrie_node_t_is_terminal(iter) == (iter->word_index < max_word_end_index_excl));
  }

  this_->are_links_computed = true;
}
#endif

void actrie_thiscall actrie_t_compute_links(struct actrie_t* this_) {
  actrie_assert(!this_->are_links_computed);

  struct actrie_node_t* m_nodes = this_->nodes.data;

  /*
   * See MIPT lecture https://youtu.be/MEFrIcGsw1o for more info
   *
   * For each char (marked as sigma) in the Alphabet:
   *   v := root_eges[sigma] <=> to((root, sigma))
   *
   *   root_edges[c] = root_edges[c] ? root_edegs[c] : root
   *   <=>
   *   to((root, sigma)) = to((root, sigma)) if (root, sigma) in rng(to) else root
   *
   *   link(v) = root (if v aka to((root, sigma)) exists)
   *
   *   rood_edges[sigma].compressed_suffix_link = root
   */

  // Run BFS through all nodes.
  // BFS queue with known max size
  uint32_t* const bfs_queue = (uint32_t*)malloc(sizeof(uint32_t) * (this_->nodes.size + 1));
  if (bfs_queue == NULL) {
    out_of_memory_handler();
    return;
  }

  size_t queue_head_index       = 0;
  size_t queue_tail_index       = 0;
  bfs_queue[queue_tail_index++] = ROOT_INDEX;

  while (queue_head_index < queue_tail_index) {
    uint32_t vertex_index = bfs_queue[queue_head_index++];

    // to(v, sigma) === vertex.edges[sigma]
    uint32_t* vertex_edges = m_nodes[vertex_index].edges;

    actrie_assert(m_nodes[vertex_index].suffix_link != NULL_NODE_INDEX);

    // to((link(v), sigma)) === m_nodes[vertex.suffix_link].edges[sigma]
    const uint32_t* vertex_suffix_link_edges = m_nodes[m_nodes[vertex_index].suffix_link].edges;

    // v --sigma-> u is a path from node u to node v via the char sigma
    // For each char (sigma) in the Alphabet vertex_edges[sigma] is the child such: v --sigma--> child
    for (size_t sigma = 0; sigma != ALPHABET_LENGTH; sigma++) {
      actrie_assert(vertex_suffix_link_edges[sigma] != NULL_NODE_INDEX);

      // child = to(v, sigma)
      uint32_t child_index = vertex_edges[sigma];

      // to((v, sigma)) = to((v, sigma)) if (v, sigma) in the rng(to) else to((link(v), sigma))
      // rng(to) is a range of function 'to'
      if (child_index != NULL_NODE_INDEX) {
        // link(to(v, sigma)) = to((link(v), sigma)) if (v, sigma) in the rng(to)
        uint32_t child_link_v_index = vertex_suffix_link_edges[sigma];

        struct actrie_node_t* child = &m_nodes[child_index];
        child->suffix_link          = child_link_v_index;

        actrie_assert(m_nodes[child_link_v_index].compressed_suffix_link != NULL_NODE_INDEX);

        // comp(v) = link(v) if (link(v) is terminal or root) else comp(link(v))
        child->compressed_suffix_link =
            ((!actrie_node_t_is_terminal(&m_nodes[child_link_v_index])) & (child_link_v_index != ROOT_INDEX))
                ? m_nodes[child_link_v_index].compressed_suffix_link
                : child_link_v_index;

        bfs_queue[queue_tail_index++] = child_index;
      } else {
        vertex_edges[sigma] = vertex_suffix_link_edges[sigma];
      }
    }
  }

  free(bfs_queue);

#if defined(__DEBUG__)
  actrie_t_check_computed_links(this_);
#endif
}

void actrie_thiscall actrie_t_run_text(const struct actrie_t* this_, const char* text, void* callback_context, FindCallback find_callback) {
  actrie_assert(find_callback != NULL);
  actrie_assert(actrie_t_is_ready(this_));

  const struct actrie_node_t* m_nodes = this_->nodes.data;
  const uint32_t* m_words_lengths     = this_->words_lengths.data;

  uint32_t current_node_index = ROOT_INDEX;
  size_t i                    = 0;
  char c;
  for (const char* text_iter = text;
       (c = IS_FIND_IGNORECASE_MODE_ON ? to_lower(*text_iter) : *text_iter) != '\0';
       ++i, ++text_iter) {
    if (!is_in_alphabet(c)) {
      current_node_index = ROOT_INDEX;
      continue;
    }

    current_node_index = m_nodes[current_node_index].edges[char_to_edge_index(c)];
    actrie_assert(current_node_index != NULL_NODE_INDEX);
    const struct actrie_node_t* current_node = m_nodes + current_node_index;
    if (actrie_node_t_is_terminal(current_node)) {
      actrie_assert(current_node->word_index < this_->words_lengths.size);
      size_t word_length = m_words_lengths[current_node->word_index];
      size_t l           = i + 1 - word_length;
      find_callback(callback_context, text + l, word_length, l);
    }

    // Jump up through compressed suffix links
    for (uint32_t tmp_node_index = current_node->compressed_suffix_link; tmp_node_index != ROOT_INDEX; tmp_node_index = m_nodes[tmp_node_index].compressed_suffix_link) {
      actrie_assert(tmp_node_index != NULL_NODE_INDEX);
      actrie_assert(actrie_node_t_is_terminal(&m_nodes[tmp_node_index]));
      size_t word_index = m_nodes[tmp_node_index].word_index;
      actrie_assert(word_index < this_->words_lengths.size);
      size_t word_length = m_words_lengths[word_index];
      size_t l           = i + 1 - word_length;
      find_callback(callback_context, text + l, word_length, l);
    }
  }
}

size_t actrie_thiscall actrie_t_replace_first_occurance_len(const struct actrie_t* this_, char* c_string, size_t length) {
  actrie_assert(actrie_t_is_ready(this_));

  uint32_t r_index_in_current_substring = 0;
  uint32_t word_index                   = 0;

  uint32_t current_node_index         = ROOT_INDEX;
  const struct actrie_node_t* m_nodes = this_->nodes.data;
  char c;
  for (const char* iter = c_string;
       (c = IS_FIND_IGNORECASE_MODE_ON ? to_lower(*iter) : *iter) != '\0';
       ++iter) {
    if (!is_in_alphabet(c)) {
      current_node_index = ROOT_INDEX;
      continue;
    }

    current_node_index = m_nodes[current_node_index].edges[char_to_edge_index(c)];
    actrie_assert(current_node_index != NULL_NODE_INDEX);

    const struct actrie_node_t* current_node = m_nodes + current_node_index;
    bool current_node_is_terminal            = actrie_node_t_is_terminal(current_node);
    size_t compressed_suffix_link            = current_node->compressed_suffix_link;

    if (current_node_is_terminal | (compressed_suffix_link != ROOT_INDEX)) {
      if (current_node_is_terminal) {
        word_index = current_node->word_index;
      } else {
        actrie_assert(actrie_node_t_is_terminal(&m_nodes[compressed_suffix_link]));
        word_index = m_nodes[compressed_suffix_link].word_index;
      }

      actrie_assert(word_index < this_->words_lengths.size);
      r_index_in_current_substring = (uint32_t)(iter - c_string);

      uint32_t word_length = this_->words_lengths.data[word_index];

      // Matched pattern is c_string[l; r]
      uint32_t l                = r_index_in_current_substring + 1 - word_length;
      size_t replacement_length = this_->words_replacement.data[word_index].size;

      if (replacement_length != word_length) {
        // In both cases: replacement_length > word_length || replacement_length < word_length code is the same
        memmove(c_string + l + replacement_length, c_string + r_index_in_current_substring + 1, length - 1 - r_index_in_current_substring);
        length += (replacement_length - word_length);
        c_string[length] = '\0';
      }

      memcpy(c_string + l, this_->words_replacement.data[word_index].c_str, replacement_length);
      break;
    }
  }

  return length;
}

size_t actrie_thiscall actrie_t_replace_all_occurances_len(const struct actrie_t* this_, char* c_string, size_t length) {
  actrie_assert(actrie_t_is_ready(this_));
#if defined(__DEBUG__)
  size_t total_copied = 0;
#endif

  struct vector_replacement_info_t queue;
  vector_replacement_info_t_ctor(&queue);

  const struct actrie_node_t* m_nodes        = this_->nodes.data;
  const uint32_t* m_words_lengths            = this_->words_lengths.data;
  const struct string_t* m_words_replacement = this_->words_replacement.data;

  size_t new_length = length;

  uint32_t current_node_index = ROOT_INDEX;
  char c;
  for (char *current_c_string = c_string, *iter = current_c_string;
       (c = IS_FIND_IGNORECASE_MODE_ON ? to_lower(*iter) : *iter) != '\0';
       ++iter) {
    if (!is_in_alphabet(c)) {
      current_node_index = ROOT_INDEX;
      continue;
    }

    current_node_index = m_nodes[current_node_index].edges[char_to_edge_index(c)];
    actrie_assert(current_node_index != NULL_NODE_INDEX);

    const struct actrie_node_t* current_node = m_nodes + current_node_index;
    bool current_node_is_terminal            = actrie_node_t_is_terminal(current_node);
    size_t compressed_suffix_link            = current_node->compressed_suffix_link;

    if (current_node_is_terminal | (compressed_suffix_link != ROOT_INDEX)) {
      actrie_assert(current_node_is_terminal || actrie_node_t_is_terminal(&m_nodes[compressed_suffix_link]));
      uint32_t word_index = current_node_is_terminal ? current_node->word_index : m_nodes[compressed_suffix_link].word_index;

      actrie_assert(word_index < this_->words_lengths.size);
      uint32_t r_index_in_current_substring = (uint32_t)(iter - current_c_string);
      uint32_t word_length                  = m_words_lengths[word_index];
      // Matched pattern is current_c_string[l; r]
      uint32_t l_index_in_current_substring = r_index_in_current_substring + 1 - word_length;
      size_t replacement_length             = m_words_replacement[word_index].size;

      if ((queue.size == 0) & (word_length == replacement_length)) {
        char* dst_address = current_c_string + l_index_in_current_substring;
        actrie_assert(c_string <= dst_address && dst_address + word_length <= c_string + length);
        memcpy(dst_address, m_words_replacement[word_index].c_str, word_length);
#if defined(__DEBUG__)
        total_copied += word_length;
#endif
      } else {
        uint32_t word_l_index_in_text = (uint32_t)(current_c_string + l_index_in_current_substring - c_string);
        vector_replacement_info_t_emplace_back2(&queue, word_l_index_in_text, word_index);
        new_length += (replacement_length - word_length);
      }

      current_c_string   = iter + 1;
      current_node_index = ROOT_INDEX;
    }
  }

  size_t right_offset = 0;
  for (const struct replacement_info_t *iter_end = queue.data - 1, *iter = iter_end + queue.size; iter != iter_end; --iter) {
    uint32_t word_index           = iter->word_index;
    uint32_t word_l_index_in_text = iter->word_l_index_in_text;
    uint32_t word_length          = m_words_lengths[word_index];
    size_t moved_part_length      = length - (word_l_index_in_text + word_length);

    char* dst_address       = c_string + (new_length - right_offset - moved_part_length);
    const char* src_address = c_string + word_l_index_in_text + word_length;

    if (dst_address != src_address) {
      // If pattern length != replacement length, worth checking
      actrie_assert((c_string <= dst_address) & (dst_address + moved_part_length <= c_string + (new_length >= length ? new_length : length)));
      actrie_assert((c_string <= src_address) & (src_address + moved_part_length <= c_string + (new_length >= length ? new_length : length)));
      memmove(dst_address, src_address, moved_part_length);
    }

    size_t replacement_length = m_words_replacement[word_index].size;
    dst_address -= replacement_length;

    actrie_assert((c_string <= dst_address) & (dst_address + replacement_length <= c_string + (new_length >= length ? new_length : length)));
    memcpy(dst_address, m_words_replacement[word_index].c_str, replacement_length);

    length = word_l_index_in_text;
    right_offset += moved_part_length + replacement_length;
#if defined(__DEBUG__)
    total_copied += moved_part_length + replacement_length;
#endif
  }

  vector_replacement_info_t_dtor(&queue);

#if defined(__DEBUG__)
  // Check for O(length + sum( |replacement_length - occurance_length| for each pattern occurance) ) complexity
  actrie_assert(total_copied <= new_length);
#endif

  c_string[new_length] = '\0';
  return new_length;
}

#if defined(actrie_noinline)
  #undef actrie_noinline
#endif

#if defined(actrie_assert)
  #undef actrie_assert
#endif

#endif
