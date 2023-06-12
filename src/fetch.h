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
#include <stdbool.h>

enum SUPPORTED_SYSTEM_BASE {
	SYSTEM_LINUX,
	SYSTEM_ANDROID,
	SYSTEM_FREEBSD,
	SYSTEM_OPENBSD,
	SYSTEM_MACOS,
	SYSTEM_WINDOWS
};

#if defined(__linux__)
	#define SYSTEM_BASE LINUX
#elif defined(__ANDROID__)
	#define SYSTEM_BASE ANDROID
#elif defined(__FreeBSD__)
	#define SYSTEM_BASE FREEBSD
#elif defined(__OpenBSD__)
	#define SYSTEM_BASE OPENBSD
#elif defined(__APPLE__)
	#define SYSTEM_BASE MACOS
#elif defined(_WIN32)
	#define SYSTEM_BASE WINDOWS
#else
	#error "System Base not specified!"
#endif

#ifdef __DEBUG__
void set_libfetch_log_level(int level);
	#define SET_LIBFETCH_LOG_LEVEL(level) set_libfetch_log_level(level)
#else
	#define SET_LIBFETCH_LOG_LEVEL(level)
#endif

char* get_user_name();
char* get_host_name();
char* get_shell_name();
char* get_model_name();
char* get_kernel_name();
char* get_os_name();
char* get_cpu_model();
char* get_pkgman_name();
char* get_image_name();
char* get_resolution();
int get_screen_width();
int get_screen_height();
char* get_memory();
int get_memory_total();
int get_memory_used();
int get_pkg_count();
long get_uptime();

#endif // _FETCH_H_
