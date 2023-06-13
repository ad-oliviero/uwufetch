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
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#if defined(__DEBUG__)
	#define LOGGING_ENABLED
#endif
#include "logging.h"
#if defined(LOGGING_ENABLED)
void set_libfetch_log_level(int level) {
	// the logging_level variable used in logging.h is static, a new variable is not needed
	SET_LOG_LEVEL(level, "libfetch");
}
#endif

struct utsname GLOBAL_UTSNAME;
struct sysinfo GLOBAL_SYSINFO;
char PROC_MEMINFO[256];
char PROC_CPUINFO[256];

#define BUFFER_SIZE 1024
#define MEM_SIZE 11 * BUFFER_SIZE // 11 strings
char memory_pool[MEM_SIZE]	= {0};
long unsigned int pool_used = 0;
void* alloc(size_t size) {
	if (size > MEM_SIZE - pool_used) {
		LOG_E("Not enough memory");
		abort();
	}
	long unsigned int start = pool_used;
	pool_used += size;
	return (void*)(memory_pool + start);
}

void libfetch_init() {
#if defined(LOGGING_ENABLED)
	set_libfetch_log_level(LEVEL_MAX);
#endif
#if SYSTEM_BASE == SYSTEM_LINUX
	uname(&GLOBAL_UTSNAME);
	sysinfo(&GLOBAL_SYSINFO);

	FILE* proc_meminfo = fopen("/proc/meminfo", "r");
	if (proc_meminfo) {
		// reading only 256 bytes because every other line of the file is not really needed
		size_t len				= fread(PROC_MEMINFO, 1, 256, proc_meminfo) - 1;
		PROC_MEMINFO[len] = '\0';
		fclose(proc_meminfo);
	}

	FILE* cpu_info = fopen("/proc/cpuinfo", "r");
	if (cpu_info) {
		size_t len				= fread(PROC_CPUINFO, 1, 256, cpu_info) - 1;
		PROC_CPUINFO[len] = '\0';
		fclose(cpu_info);
	}
#endif

#if defined(LOGGING_ENABLED)
	set_libfetch_log_level(LEVEL_DISABLE);
#endif
}

char* get_user_name() {
	char* user_name = alloc(BUFFER_SIZE);
#if SYSTEM_BASE == SYSTEM_LINUX
	char* env = getenv("USER");
	if (env) {
		size_t len = strlen(env);
		memcpy(user_name, env, len < BUFFER_SIZE ? len : BUFFER_SIZE - 1);
	} else {
		FILE* pp = popen("whoami", "r");
		if (pp) {
			fscanf(pp, "%s", user_name);
			pclose(pp);
		}
	}
#endif
	return user_name;
}

char* get_host_name() {
	char* host_name = alloc(BUFFER_SIZE);
#if SYSTEM_BASE == SYSTEM_LINUX
	size_t len = strlen(GLOBAL_UTSNAME.nodename);
	if (len > 0) {
		memcpy(host_name, GLOBAL_UTSNAME.nodename, len < BUFFER_SIZE ? len : BUFFER_SIZE - 1);
	} else {
		char* env = getenv("HOSTNAME");
		if (env) {
			len = strlen(env);
			memcpy(host_name, env, len < BUFFER_SIZE ? len : BUFFER_SIZE - 1);
		} else {
			FILE* fp = fopen("/etc/hostname", "r");
			if (fp) {
				len = fread(host_name, 1, BUFFER_SIZE, fp) - 1;
				fclose(fp);
				if (host_name[len] == '\n') host_name[len] = '\0';
			}
		}
	}
#endif
	return host_name;
}

char* get_shell() {
	char* shell_name = alloc(BUFFER_SIZE);
#if SYSTEM_BASE == SYSTEM_LINUX
	char* env = getenv("SHELL");
	if (env) {
		size_t len = strlen(env);
		memcpy(shell_name, env, len < BUFFER_SIZE ? len : BUFFER_SIZE - 1);
	}
#endif
	return shell_name;
}

char* get_model() {
	char* r = alloc(BUFFER_SIZE);
	return r;
}

char* get_kernel() {
	char* kernel_name = alloc(BUFFER_SIZE);
#if SYSTEM_BASE == SYSTEM_LINUX
	size_t name_len = strlen(GLOBAL_UTSNAME.sysname);
	size_t len			= name_len;
	if (name_len > 0) {
		memcpy(kernel_name, GLOBAL_UTSNAME.sysname, len < BUFFER_SIZE ? name_len : BUFFER_SIZE - 1);
		if (name_len < BUFFER_SIZE - 2) {
			kernel_name[name_len++] = ' ';
			len++;
		}
	}
	size_t release_len = strlen(GLOBAL_UTSNAME.release);
	len += release_len;
	if (release_len > 0) {
		memcpy(kernel_name + name_len, GLOBAL_UTSNAME.release, len < BUFFER_SIZE ? release_len : BUFFER_SIZE - name_len - 1);
		if (len < BUFFER_SIZE - 2) {
			kernel_name[len++] = ' ';
			release_len++;
		}
	}
	size_t machine_len = strlen(GLOBAL_UTSNAME.machine);
	len += machine_len;
	if (machine_len > 0) {
		memcpy(kernel_name + name_len + release_len, GLOBAL_UTSNAME.machine, len < BUFFER_SIZE ? machine_len : BUFFER_SIZE - (name_len + release_len) - 1);
	}
#endif
	return kernel_name;
}

char* get_os_name() {
	char* os_name = alloc(BUFFER_SIZE);
	char buffer[BUFFER_SIZE];
#if SYSTEM_BASE == SYSTEM_LINUX
	FILE* fp = fopen("/etc/os-release", "r");
	if (fp) {
		while (fgets(buffer, BUFFER_SIZE, fp) &&
					 !(sscanf(buffer, "\nID=\"%s\"", os_name) ||
						 sscanf(buffer, "\nID=%s", os_name)))
			;
		fclose(fp);
	}
#endif
	return os_name;
}

char* get_cpu_model() {
	char* cpu_model = alloc(BUFFER_SIZE);
#if SYSTEM_BASE == SYSTEM_LINUX
	char* p = PROC_CPUINFO - 1;
	do {
		p++;
		sscanf(p, "model name%*[ |	]: %[^\n]", cpu_model);
	} while ((p = strchr(p, '\n')));
#endif
	return cpu_model;
}

char* get_packages() {
	char* r = alloc(BUFFER_SIZE);
	return r;
}

char* get_image_name() {
	char* r = alloc(BUFFER_SIZE);
	return r;
}

int get_screen_width() {
	return 0;
}

int get_screen_height() {
	return 0;
}

unsigned long get_memory_total() {
	unsigned long memory_total = 0;
#if SYSTEM_BASE == SYSTEM_LINUX
	memory_total = GLOBAL_SYSINFO.totalram;
	memory_total /= 1048576;
#endif
	return memory_total;
}

unsigned long get_memory_used() {
	unsigned long memory_used = 0;
#if SYSTEM_BASE == SYSTEM_LINUX
	unsigned long memtotal = 0, memfree = 0, buffers = 0, cached = 0;
	char* p = PROC_MEMINFO - 1;
	do {
		p++;
		sscanf(p, "MemTotal:%*[^0-9]%lu", &memtotal);
		sscanf(p, "MemFree:%*[^0-9]%lu", &memfree);
		sscanf(p, "Buffers:%*[^0-9]%lu", &buffers);
		sscanf(p, "Cached:%*[^0-9]%lu", &cached);
	} while ((p = strchr(p, '\n')));
	memory_used = (memtotal - (memfree + buffers + cached)) / 1024;
#endif
	return memory_used;
}

long get_uptime() {
	long uptime = 0;
#if SYSTEM_BASE == SYSTEM_LINUX
	uptime = GLOBAL_SYSINFO.uptime;
#endif
	return uptime;
}
