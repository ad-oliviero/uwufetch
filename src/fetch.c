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
#if defined(SYSTEM_BASE_LINUX)
  #include <sys/sysinfo.h>
  #include <sys/utsname.h>
#elif defined(SYSTEM_BASE_FREEBSD)
  #include <sys/sysctl.h>
  #include <sys/time.h>
  #include <sys/types.h>
#endif
#include <unistd.h>
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

#if defined(SYSTEM_BASE_LINUX)
struct utsname GLOBAL_UTSNAME;
struct sysinfo GLOBAL_SYSINFO;
#endif
char PROC_MEMINFO[256];
char PROC_CPUINFO[256];
char FB0_VIRTUAL_SIZE[256];

#define BUFFER_SIZE 1024
#define MEM_SIZE 11 * BUFFER_SIZE // 11 strings
char memory_pool[MEM_SIZE]  = {0};
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

void libfetch_init(void) {
#if defined(SYSTEM_BASE_LINUX) || defined(SYSTEM_BASE_ANDROID)
  uname(&GLOBAL_UTSNAME);
  sysinfo(&GLOBAL_SYSINFO);

  FILE* proc_meminfo = fopen("/proc/meminfo", "r");
  if (proc_meminfo) {
    // reading only 256 bytes because every other line of the file is not really needed
    size_t len        = fread(PROC_MEMINFO, 1, 256, proc_meminfo) - 1;
    PROC_MEMINFO[len] = '\0';
    fclose(proc_meminfo);
  }

  FILE* cpu_info = fopen("/proc/cpuinfo", "r");
  if (cpu_info) {
    size_t len        = fread(PROC_CPUINFO, 1, 256, cpu_info) - 1;
    PROC_CPUINFO[len] = '\0';
    fclose(cpu_info);
  }
  #if !defined(SYSTEM_BASE_ANDROID)
  FILE* fb0_virtual_size = fopen("/sys/class/graphics/fb0/virtual_size", "r");
  if (fb0_virtual_size) {
    size_t len            = fread(FB0_VIRTUAL_SIZE, 1, 256, fb0_virtual_size) - 1;
    FB0_VIRTUAL_SIZE[len] = '\0';
    fclose(fb0_virtual_size);
  }
  #endif
#elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_MACOS)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not implemented");
#else
  LOG_E("System not supported or system base not specified");
#endif
}

char* get_user_name(void) {
  char* user_name = alloc(BUFFER_SIZE);
#if defined(SYSTEM_BASE_LINUX) || defined(SYSTEM_BASE_ANDROID) || defined(SYSTEM_BASE_FREEBSD)
  char* env = getenv("USER");
  if (env)
    snprintf(user_name, BUFFER_SIZE, "%s", env);
  else {
    FILE* pp = popen("whoami", "r");
    if (pp) {
      fscanf(pp, "%s", user_name);
      pclose(pp);
    }
  }
#elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_MACOS)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not implemented");
#else
  LOG_E("System not supported or system base not specified");
#endif
  return user_name;
}

char* get_host_name(void) {
  char* host_name = alloc(BUFFER_SIZE);
#if defined(SYSTEM_BASE_LINUX) || defined(SYSTEM_BASE_ANDROID) || defined(SYSTEM_BASE_FREEBSD)
  size_t len = 0;
  #if !defined(SYSTEM_BASE_FREEBSD)
  len = strlen(GLOBAL_UTSNAME.nodename);
  if (len > 0) {
    snprintf(host_name, BUFFER_SIZE, "%s", GLOBAL_UTSNAME.nodename);
  } else {
  #endif
    char* env = getenv("HOST");
    if (env)
      snprintf(host_name, BUFFER_SIZE, "%s", env);
    else {
      FILE* fp = fopen("/etc/hostname", "r");
      if (fp) {
        len = fread(host_name, 1, BUFFER_SIZE, fp) - 1;
        fclose(fp);
        if (host_name[len] == '\n') host_name[len] = '\0';
      } else {
        gethostname(host_name, BUFFER_SIZE);
      }
    }
  #if !defined(SYSTEM_BASE_FREEBSD)
  }
  #endif
#elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_MACOS)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not implemented");
#else
  LOG_E("System not supported or system base not specified");
#endif
  return host_name;
}

char* get_shell(void) {
  char* shell_name = alloc(BUFFER_SIZE);
#if defined(SYSTEM_BASE_LINUX) || defined(SYSTEM_BASE_ANDROID) || defined(SYSTEM_BASE_FREEBSD)
  char* env = getenv("SHELL");
  if (env) snprintf(shell_name, BUFFER_SIZE, "%s", env);
#elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_MACOS)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not implemented");
#else
  LOG_E("System not supported or system base not specified");
#endif
  return shell_name;
}

char* get_model(void) {
  char* model = alloc(BUFFER_SIZE);
#if defined(SYSTEM_BASE_LINUX)
  FILE* model_fp;
  char* model_filename[4] = {
      "/sys/devices/virtual/dmi/id/product_version",
      "/sys/devices/virtual/dmi/id/product_name",
      "/sys/devices/virtual/dmi/id/board_name",
  };

  char tmp_model[4][BUFFER_SIZE] = {0}; // temporary variable to store the contents of all 3 files
  int longest_model = 0, best_len = 0, currentlen = 0;
  for (int i = 0; i < 4; i++) {
    model_fp = fopen(model_filename[i], "r");
    if (model_fp) {
      fgets(tmp_model[i], BUFFER_SIZE, model_fp);
      fclose(model_fp);
    }
    currentlen = strlen(tmp_model[i]);
    if (currentlen > best_len) {
      best_len      = currentlen;
      longest_model = i;
    }
  }
  snprintf(model, BUFFER_SIZE, "%s", tmp_model[longest_model]);
  if (model[best_len - 1] == '\n') model[best_len - 1] = '\0';
#elif defined(SYSTEM_BASE_ANDROID)
  FILE* marketname = popen("getprop ro.product.marketname", "r");
  if (marketname) {
    fgets(model, BUFFER_SIZE, marketname);
    pclose(marketname);
    size_t len = strlen(model);
    if (model[len - 1] == '\n') model[len - 1] = '\0';
  }
#elif defined(SYSTEM_BASE_FREEBSD)
  char buf[BUFFER_SIZE] = {0};
  size_t len            = sizeof(buf);
  sysctlbyname("hw.hv_vendor", &buf, &len, NULL, 0);
  strcpy(model, buf);
#elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_MACOS)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not implemented");
#else
  LOG_E("System not supported or system base not specified");
#endif
  return model;
}

char* get_kernel(void) {
  char* kernel_name = alloc(BUFFER_SIZE);
#if defined(SYSTEM_BASE_LINUX) || defined(SYSTEM_BASE_ANDROID)
  char* p    = kernel_name;
  size_t len = 0;
  if (strlen(GLOBAL_UTSNAME.sysname) > 0) {
    p += snprintf(p, BUFFER_SIZE, "%s ", GLOBAL_UTSNAME.sysname);
    len = p - kernel_name;
  }
  if (strlen(GLOBAL_UTSNAME.release) > 0) {
    p += snprintf(p, BUFFER_SIZE - len, "%s ", GLOBAL_UTSNAME.release);
    len = p - kernel_name;
  }
  if (strlen(GLOBAL_UTSNAME.machine) > 0)
    p += snprintf(p, BUFFER_SIZE - len, "%s", GLOBAL_UTSNAME.machine);
#elif defined(SYSTEM_BASE_FREEBSD)
  char buf[BUFFER_SIZE] = {0};
  size_t len            = sizeof(buf);
  sysctlbyname("kern.version", &buf, &len, NULL, 0);
  strcpy(kernel_name, buf);
  len = strlen(kernel_name) - 1;
  if (kernel_name[len] == '\n') kernel_name[len] = '\0';
#elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_MACOS)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not implemented");
#else
  LOG_E("System not supported or system base not specified");
#endif
  return kernel_name;
}

char* get_os_name(void) {
  char* os_name = alloc(BUFFER_SIZE);
#if defined(SYSTEM_BASE_LINUX) || defined(SYSTEM_BASE_FREEBSD)
  char buffer[BUFFER_SIZE];
  FILE* fp = fopen("/etc/os-release", "r");
  if (fp) {
    while (fgets(buffer, BUFFER_SIZE, fp) &&
           !(sscanf(buffer, "\nID=\"%s\"", os_name) ||
             sscanf(buffer, "\nID=%s", os_name)))
      ;
    fclose(fp);
  }
#elif defined(SYSTEM_BASE_ANDROID)
  sprintf(os_name, "android");
#elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_MACOS)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not implemented");
#else
  LOG_E("System not supported or system base not specified");
#endif
  return os_name;
}

char* get_cpu_model(void) {
  char* cpu_model = alloc(BUFFER_SIZE);
#if defined(SYSTEM_BASE_LINUX) || defined(SYSTEM_BASE_ANDROID)
  char* p = PROC_CPUINFO - 1;
  do {
    p++;
    sscanf(p, "model name%*[ |	]: %[^\n]", cpu_model);
  } while ((p = strchr(p, '\n')));
#elif defined(SYSTEM_BASE_FREEBSD)
  char buf[BUFFER_SIZE] = {0};
  size_t len            = sizeof(buf);
  sysctlbyname("hw.model", &buf, &len, NULL, 0);
  strcpy(cpu_model, buf);
#elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_MACOS)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not implemented");
#else
  LOG_E("System not supported or system base not specified");
#endif
  return cpu_model;
}

char* get_packages(void) {
  char* packages = alloc(BUFFER_SIZE);
#if defined(SYSTEM_BASE_ANDROID)
  #define PKGPATH "/data/data/com.termux/files/usr/bin/"
#elif defined(SYSTEM_BASE_MACOS)
  #define PKGPATH "/usr/local/bin/"
#elif defined(SYSTEM_BASE_FREEBSD)
  #define PKGPATH "/usr/sbin/"
#else // Linux, OpenBSD, FreeBSD
  #define PKGPATH "/usr/bin/"
#endif
  struct pkgcmd {
    char* path;
    char* command;
    char* name;
    size_t count;
  };
#define CMD_COUNT 16
  struct pkgcmd cmds[CMD_COUNT] = {
      {PKGPATH "apt", "apt list --installed 2> /dev/null | wc -l", "(apt)", 0},
      {PKGPATH "apk", "apk info 2> /dev/null | wc -l", "(apk)", 0},
      {PKGPATH "qlist", "qlist -I 2> /dev/null | wc -l", "(emerge)", 0},
      {PKGPATH "flatpak", "flatpak list 2> /dev/null | wc -l", "(flatpak)", 0},
      {PKGPATH "snap", "snap list 2> /dev/null | wc -l", "(snap)", 0},
      {PKGPATH "guix", "guix package --list-installed 2> /dev/null | wc -l", "(guix)", 0},
      {"/run/current-system/sw/bin/nix-store", "nix-store -q --requisites /run/current-system/sw 2> /dev/null | wc -l", "(nix)", 0},
      {PKGPATH "pacman", "pacman -Qq 2> /dev/null | wc -l", "(pacman)", 0},
      {PKGPATH "pkg", "pkg info 2>/dev/null | wc -l", "(pkg)", 0},
      {PKGPATH "pkg_info", "pkg_info 2>/dev/null | wc -l | sed \"s/ //g\"", "(pkg)", 0},
      {PKGPATH "port", "port installed 2> /dev/null | tail -n +2 | wc -l", "(port)", 0},
      {PKGPATH "brew", "find $(brew --cellar 2>/dev/stdout) -maxdepth 1 -type d 2> /dev/null | wc -l | awk '{print $1, 0}'", "(brew-cellar)", 0},
      {PKGPATH "brew", "find $(brew --caskroom 2>/dev/stdout) -maxdepth 1 -type d 2> /dev/null | wc -l | awk '{print $1, 0}'", "(brew-cask)", 0},
      {PKGPATH "rpm", "rpm -qa --last 2> /dev/null | wc -l", "(rpm)", 0},
      {PKGPATH "xbps-query", "xbps-query -l 2> /dev/null | wc -l", "(xbps)", 0},
      {PKGPATH "zypper", "zypper -q se --installed-only 2> /dev/null | wc -l", "(zypper)", 0},
  };
  size_t total   = 0;
  int last_valid = 0;
  for (int i = 0; i < CMD_COUNT; i++)
    if (access(cmds[i].path, F_OK) != -1) {
      FILE* fp = popen(cmds[i].command, "r");
      if (fscanf(fp, "%lu", &cmds[i].count) == 3)
        continue;
      else {
        last_valid = cmds[i].count > 0 ? i : last_valid;
        total += cmds[i].count;
      }
      pclose(fp);
    }
  char* p = packages + sprintf(packages, "%lu: ", total);
  for (int i = 0; i < CMD_COUNT; i++)
    if (cmds[i].count > 0)
      p += snprintf(p, BUFFER_SIZE - strlen(packages), "%lu %s%s", cmds[i].count, cmds[i].name, i == last_valid ? "" : ", ");
  return packages;
#undef PKGPATH
}

char* get_image_name(void) {
  char* r = alloc(BUFFER_SIZE);
#if defined(SYSTEM_BASE_LINUX)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_ANDROID)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_FREEBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_MACOS)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not implemented");
#else
  LOG_E("System not supported or system base not specified");
#endif
  return r;
}

int get_screen_width(void) {
  int screen_width = 0;
#if defined(SYSTEM_BASE_LINUX)
  sscanf(FB0_VIRTUAL_SIZE, "%d,%*d", &screen_width);
#elif defined(SYSTEM_BASE_ANDROID)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_FREEBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_MACOS)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not implemented");
#else
  LOG_E("System not supported or system base not specified");
#endif
  return screen_width;
}

int get_screen_height(void) {
  int screen_height = 0;
#if defined(SYSTEM_BASE_LINUX)
  sscanf(FB0_VIRTUAL_SIZE, "%*d,%d", &screen_height);
#elif defined(SYSTEM_BASE_ANDROID)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_FREEBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_MACOS)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not implemented");
#else
  LOG_E("System not supported or system base not specified");
#endif
  return screen_height;
}

unsigned long get_memory_total(void) {
  unsigned long memory_total = 0;
#if defined(SYSTEM_BASE_LINUX) || defined(SYSTEM_BASE_ANDROID)
  memory_total = GLOBAL_SYSINFO.totalram;
#elif defined(SYSTEM_BASE_FREEBSD)
  size_t len = sizeof(memory_total);
  sysctlbyname("vm.kmem_size", &memory_total, &len, NULL, 0);
#elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_MACOS)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not implemented");
#else
  LOG_E("System not supported or system base not specified");
#endif
  memory_total /= 1048576;
  return memory_total;
}

unsigned long get_memory_used(void) {
  unsigned long memory_used = 0;
#if defined(SYSTEM_BASE_LINUX) || defined(SYSTEM_BASE_ANDROID)
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
#elif defined(SYSTEM_BASE_FREEBSD)
  unsigned long kmem_size        = 0;
  unsigned long pagesize         = 0;
  unsigned long v_free_count     = 0;
  unsigned long v_inactive_count = 0;
  size_t len                     = sizeof(unsigned long);
  sysctlbyname("vm.kmem_size", &kmem_size, &len, NULL, 0);
  sysctlbyname("hw.pagesize", &pagesize, &len, NULL, 0);
  sysctlbyname("vm.stats.vm.v_free_count", &v_free_count, &len, NULL, 0);
  sysctlbyname("vm.stats.vm.v_inactive_count", &v_inactive_count, &len, NULL, 0);
  memory_used = (kmem_size - (pagesize * (v_free_count + v_inactive_count))) / 1048576;
#elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_MACOS)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not implemented");
#else
  LOG_E("System not supported or system base not specified");
#endif
  return memory_used;
}

long get_uptime(void) {
  long uptime = 0;
#if defined(SYSTEM_BASE_LINUX) || defined(SYSTEM_BASE_ANDROID)
  uptime = GLOBAL_SYSINFO.uptime;
#elif defined(SYSTEM_BASE_FREEBSD)
  struct timeval boottime;
  size_t len = sizeof(boottime);
  sysctlbyname("kern.boottime", &boottime, &len, NULL, 0);
  time_t current_time;
  time(&current_time);
  uptime = current_time - boottime.tv_sec;
#elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_MACOS)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not implemented");
#else
  LOG_E("System not supported or system base not specified");
#endif
  return uptime;
}
