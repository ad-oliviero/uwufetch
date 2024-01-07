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

#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <errno.h>
#include <stdio.h>
#include <string.h>

#if defined(__DEBUG__)
  #define LOGGING_ENABLED
#endif

enum LOG_LEVELS {
  LEVEL_DISABLE,
  LEVEL_ERROR,
  LEVEL_WARNING,
  LEVEL_INFO,
  LEVEL_VAR,
  LEVEL_MAX = LEVEL_VAR
};

#if defined(LOGGING_ENABLED)
  #define LOG_BUF_SIZE 2048

static int logging_level       = 0;
static int logging_error_count = 0;
static void escapeColors(char* buf);

  #define SET_LOG_LEVEL(level, additional_info) set_logging_level(level, additional_info)
  #define LOG_I(format, ...) \
    if (logging_level >= LEVEL_INFO) LOG(LEVEL_INFO, format, ##__VA_ARGS__)
  #define LOG_W(format, ...) \
    if (logging_level >= LEVEL_WARNING) LOG(LEVEL_WARNING, format, ##__VA_ARGS__)
  #define LOG_E(format, ...)                   \
    if (logging_level >= LEVEL_ERROR) {        \
      LOG(LEVEL_ERROR, format, ##__VA_ARGS__); \
      logging_error_count++;                   \
    }
  #define LOG_V(var)                                                                    \
    if (logging_level >= LEVEL_VAR) {                                                   \
      static char format[1024] = "";                                                    \
      sprintf(format, "%s = %s", #var,                                                  \
              _Generic((var), int: "%d", float: "%f", char*: "\"%s\"", default: "%p")); \
      LOG(LEVEL_VAR, format, var);                                                      \
    }
  #define LOG(type, format, ...)                                  \
    {                                                             \
      static char buf[LOG_BUF_SIZE] = "";                         \
      if (sizeof(#__VA_ARGS__) == sizeof(""))                     \
        sprintf(buf, "%s", format);                               \
      else {                                                      \
        sprintf(buf, format, ##__VA_ARGS__);                      \
        escapeColors(buf);                                        \
      }                                                           \
      fprintf(stderr, "[%s]: %s in %s:%d: %s\n",                  \
              type == LEVEL_INFO      ? "\033[32mINFO\033[0m"     \
              : type == LEVEL_WARNING ? "\033[33mWARNING\033[0m"  \
              : type == LEVEL_ERROR   ? "\033[31mERROR\033[0m"    \
              : type == LEVEL_VAR     ? "\033[37mVARIABLE\033[0m" \
                                      : "",                           \
              __func__, __FILE__, __LINE__, buf);                 \
    }
  #define CHECK_FUNC(fn, err_val) \
    if (fn == err_val) LOG_E("%s returned %s: %s", #fn, #err_val, strerror(errno))
  #define CHECK_FN_NEG(fn) \
    if (fn < 0) LOG_E("%s failed: %s", #fn, strerror(errno))
static __attribute__((unused)) void set_logging_level(int level, char* additional_info) {
  if (level < LEVEL_DISABLE || level > LEVEL_MAX) {
    logging_level = LEVEL_ERROR;
    LOG_E("%s; invalid logging level: %d", additional_info, level);
    return;
  }
  logging_level = level;
  LOG(LEVEL_INFO, "%s; logging level set to %d", additional_info, level);
}
static void escapeColors(char* str) {
  const char* color_strings[] = {"<r>", "<g>", "<b>", "</>"};
  const char* colors[]        = {"\033[31m", "\033[32m", "\033[33m", "\033[0m"};
  for (int i = 0; i < 4; i++) {
    char* pos;
    size_t slen = strlen(color_strings[i]);
    size_t rlen = strlen(colors[i]);
    while ((pos = strstr(str, color_strings[i])) != NULL) {
      if (strlen(str) + rlen < LOG_BUF_SIZE) {
        memmove(pos + rlen, pos + slen, strlen(pos + slen) + 1);
        strncpy(pos, colors[i], rlen);
      }
    }
  }
}
#else
  #define SET_LOG_LEVEL(level, additional_info)
  #define LOG_I(format, ...)
  #define LOG_W(format, ...)
  #define LOG_E(format, ...)
  #define LOG_V(var)
  #define LOG(type, format, ...) printf(format "\n", ##__VA_ARGS__);
  #define CHECK_FUNC(fn, err_val) fn;
  #define CHECK_FN_NEG(fn) fn;
#endif // LOGGING_ENABLED

#endif // _LOGGING_H_
