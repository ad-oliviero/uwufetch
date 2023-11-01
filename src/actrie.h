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
#endif
