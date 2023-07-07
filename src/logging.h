#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <stdio.h>
#include <string.h>

#ifdef LOGGING_ENABLED
  #define LOG_BUF_SIZE 2048

static void escapeColors(char* buf);

enum LOG_LEVELS {
  LEVEL_DISABLE,
  LEVEL_ERROR,
  LEVEL_WARNING,
  LEVEL_INFO,
  LEVEL_VAR,
  LEVEL_MAX = LEVEL_VAR
};
  #define SET_LOG_LEVEL(level, additional_info) set_logging_level(level, additional_info)
  #define LOG_I(format, ...) \
    if (logging_level >= LEVEL_INFO) LOG(LEVEL_INFO, format, ##__VA_ARGS__)
  #define LOG_W(format, ...) \
    if (logging_level >= LEVEL_WARNING) LOG(LEVEL_WARNING, format, ##__VA_ARGS__)
  #define LOG_E(format, ...) \
    if (logging_level >= LEVEL_ERROR) LOG(LEVEL_ERROR, format, ##__VA_ARGS__)
  #define LOG_V(var)                       \
    if (logging_level >= LEVEL_VAR) {      \
      static char format[1024] = "";       \
      sprintf(format, "%s = %s", #var,     \
              _Generic((var), int          \
                       : "%d", float       \
                       : "%f", char*       \
                       : "\"%s\"", default \
                       : "%p"));           \
      LOG(LEVEL_VAR, format, var);         \
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
  #define CHECK_FN_NULL(fn) \
    if (fn == NULL) LOG_E("%s returned NULL: %s", #fn, strerror(errno))
  #define CHECK_FN_NEG(fn) \
    if (fn < 0) LOG_E("%s failed: %s", #fn, strerror(errno))
static int logging_level = 0;
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
  char* color_strings[] = {"<r>", "<g>", "<b>", "</>"};
  char* colors[]        = {"\033[31m", "\033[32m", "\033[33m", "\033[0m"};
  for (int i = 0; i < 4; i++) {
    char* pos;
    int slen = strlen(color_strings[i]);
    int rlen = strlen(colors[i]);
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
  #define LOG_E(format, ...)
  #define LOG_V(var)
  #define LOG(type, format, ...)
  #define CHECK_FN_NULL(fn)
  #define CHECK_FN_NEG(fn)
#endif // LOGGING_ENABLED

#endif // _LOGGING_H_
