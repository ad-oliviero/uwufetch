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

#ifndef _FETCH_H_
#define _FETCH_H_
#include "logging.h"
#include <stdbool.h>

#if defined(__linux__)
  #if defined(__ANDROID__)
    #define SYSTEM_BASE_ANDROID
  #else
    #define SYSTEM_BASE_LINUX
  #endif
#elif defined(__FreeBSD__)
  #define SYSTEM_BASE_FREEBSD
#elif defined(__OpenBSD__)
  #define SYSTEM_BASE_OPENBSD
#elif defined(__APPLE__)
  #define SYSTEM_BASE_MACOS
#elif defined(_WIN32)
  #define SYSTEM_BASE_WINDOWS
#else
  #error "System Base not specified!"
#endif

#if defined(SYSTEM_BASE_WINDOWS)
struct winsize {
  unsigned short ws_col;
  unsigned short ws_row;
  unsigned short ws_xpixel;
  unsigned short ws_ypixel;
};
#endif

#ifdef LOGGING_ENABLED
void set_libfetch_log_level(int level);
  #define SET_LIBFETCH_LOG_LEVEL(level) set_libfetch_log_level(level)
#else
  #define SET_LIBFETCH_LOG_LEVEL(level)
#endif

void libfetch_init(void);
void libfetch_cleanup(void);
char* get_user_name(void);
char* get_host_name(void);
char* get_shell(void);
char* get_model(void);
char* get_kernel(void);
char* get_os_name(void);
char* get_cpu(void);
char** get_gpu_list(void);
char* get_packages(void);
int get_screen_width(void);
int get_screen_height(void);
unsigned long long get_memory_total(void);
unsigned long long get_memory_used(void);
long get_uptime(void);
struct winsize get_terminal_size(void);

#endif // _FETCH_H_
