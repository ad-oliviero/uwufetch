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

#include "fetch.h"
#include <stdlib.h>
#if defined(__DEBUG__)
	#define LOGGING_ENABLED
#endif
#include "logging.h"
#if defined(LOGGING_ENABLED)
void set_libfetch_log_level(int level) {
	SET_LOG_LEVEL(level, "libfetch");
}
#endif

#define MEM_SIZE (11 * 1024) // 1024 bytes of memory for 11 strings
static void* memory[MEM_SIZE];
long unsigned int memory_used = 0;
void* alloc(size_t size) {
	if (size > MEM_SIZE - memory_used) {
		LOG_E("Not enough memory");
		exit(1);
	}
	long unsigned int start = memory_used;
	memory_used += size;
	return (void*)(memory + start);
}

char* get_user_name() {
	char* user_name = alloc(1024);
#if SYSTEM_BASE == SYSTEM_LINUX
	sprintf(user_name, "%s", getenv("USER"));
#endif
	return user_name;
}

char* get_host_name() {
	char* r = alloc(1024);
	return r;
}

char* get_shell_name() {
	char* r = alloc(1024);
	return r;
}

char* get_model_name() {
	char* r = alloc(1024);
	return r;
}

char* get_kernel_name() {
	char* r = alloc(1024);
	return r;
}

char* get_os_name() {
	char* r = alloc(1024);
	return r;
}

char* get_cpu_model() {
	char* r = alloc(1024);
	return r;
}

char* get_pkgman_name() {
	char* r = alloc(1024);
	return r;
}

char* get_image_name() {
	char* r = alloc(1024);
	return r;
}

char* get_resolution() {
	char* r = alloc(1024);
	return r;
}

int get_screen_width() {
	return 0;
}

int get_screen_height() {
	return 0;
}

char* get_memory() {
	char* r = alloc(1024);
	return r;
}

int get_memory_total() {
	return 0;
}

int get_memory_used() {
	return 0;
}

int get_pkg_count() {
	return 0;
}

long get_uptime() {
	return 1270;
}
