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

#ifdef __APPLE__
  #include <TargetConditionals.h> // for checking iOS
#endif
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if defined(__APPLE__) || defined(__BSD__)
  #include <sys/sysctl.h>
  #if defined(__OPENBSD__)
    #include <sys/time.h>
  #else
    #include <time.h>
  #endif // defined(__OPENBSD__)
#else    // defined(__APPLE__) || defined(__BSD__)
  #ifdef __BSD__
  #else // defined(__BSD__) || defined(_WIN32)
    #ifndef _WIN32
      #ifndef __OPENBSD__
        #include <sys/sysinfo.h>
      #else  // __OPENBSD__
      #endif // __OPENBSD__
    #else    // _WIN32
      #include <sysinfoapi.h>
    #endif // _WIN32
  #endif   // defined(__BSD__) || defined(_WIN32)
#endif     // defined(__APPLE__) || defined(__BSD__)
#ifndef _WIN32
  #include <pthread.h> // linux only right now
  #include <sys/ioctl.h>
  #include <sys/utsname.h>
#else // _WIN32
  #include <windows.h>
CONSOLE_SCREEN_BUFFER_INFO csbi;
#endif // _WIN32

#define LIBFETCH_INTERNAL // to do certain things only when included from the library itself
#include "fetch.h"
#define BUFFER_SIZE 256
#ifdef __DEBUG__
static bool verbose_enabled = false;
bool* get_verbose_handle() { return &verbose_enabled; }
#endif

#ifndef PKGPATH
  #ifdef __APPLE__
    #define PKGPATH "/usr/local/bin/"
  #else
    #define PKGPATH "/usr/bin/"
  #endif
#endif

#ifdef __APPLE__
// buffers where data fetched from sysctl are stored
  #define CPUBUFFERLEN 128

char cpu_buffer[CPUBUFFERLEN];
size_t cpu_buffer_len = CPUBUFFERLEN;

// Installed RAM
int64_t mem_buffer    = 0;
size_t mem_buffer_len = sizeof(mem_buffer);

// uptime
struct timeval time_buffer;
size_t time_buffer_len = sizeof(time_buffer);
#endif // __APPLE__

struct package_manager {
  char* command_path;
  char* command_string; // command to get number of packages installed
  char* pkgman_name;    // name of the package manager
};

// truncates the given string
void truncate_str(char* string, int target_width) {
  char arr[target_width];
  for (int i = 0; i < target_width; i++) arr[i] = string[i];
  string = arr;
}

// remove square brackets (for gpu names)
void remove_brackets(char* str) {
  int i = 0, j = 0;
  while (i < (int)strlen(str))
    if (str[i] == '[' || str[i] == ']')
      for (j = i; j < (int)strlen(str); j++) str[j] = str[j + 1];
    else
      i++;
}

void get_twidth(struct info* user_info) {
  LOG_I("getting terminal width");
  // get terminal width used to truncate long names
#ifndef _WIN32
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &user_info->win);
  user_info->target_width = user_info->win.ws_col - 30;
  LOG_V(user_info->target_width);
#else  // _WIN32
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  user_info->ws_col  = csbi.srWindow.Right - csbi.srWindow.Left - 29;
  user_info->ws_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
  LOG_V(user_info->ws_col);
  LOG_V(user_info->ws_rows);
#endif // _WIN32
}

void get_sys(struct info* user_info) {
  LOG_I("getting sys_var struct");
#ifndef _WIN32
  uname(&user_info->sys_var);
#endif // _WIN32
#ifndef __APPLE__
  #ifndef __BSD__
    #ifndef _WIN32
  sysinfo(&user_info->sys);
    #else
  GetSystemInfo(&user_info->sys);
    #endif
  #endif
#endif
}

// tries to get cpu name
void* get_cpu(void* argp) {
  if (!((struct thread_varg*)argp)->thread_flags[0]) return 0;
  char* buffer           = ((struct thread_varg*)argp)->buffer;
  struct info* user_info = ((struct thread_varg*)argp)->user_info;
  FILE* cpuinfo          = ((struct thread_varg*)argp)->cpuinfo;
  LOG_I("getting cpu name");
  if (cpuinfo) {
    while (fgets(buffer, BUFFER_SIZE, cpuinfo)) {
#ifdef __BSD__
      if (sscanf(buffer, "hw.model"
  #ifdef __FREEBSD__
                         ": "
  #elif defined(__OPENBSD__)
                         "="
  #endif
                         "%[^\n]",
                 user_info->cpu_model))
        break;
#else
      if (sscanf(buffer, "model name    : %[^\n]", user_info->cpu_model)) break;
#endif // __BSD__
    }
  }
  if (strlen(user_info->cpu_model) == 0) {
    LOG_E("failed to get cpu name");
    rewind(cpuinfo);
    char cores[4] = "";
    while (fgets(buffer, BUFFER_SIZE, cpuinfo)) // get the last core number
      sscanf(buffer, "processor%*[    |	]: %[^\n]", cores);
    cores[strlen(cores) - 1] += 1; // should be a number
    sprintf(user_info->cpu_model, "%s Cores", cores);
  }
  LOG_V(user_info->cpu_model);
  return 0;
}

// tries to get memory usage
void* get_ram(void* argp) {
  if (!((struct thread_varg*)argp)->thread_flags[1]) return 0;
  LOG_I("getting ram");
  struct info* user_info = ((struct thread_varg*)argp)->user_info;
#ifndef __APPLE__
  #ifdef _WIN32
  FILE* mem_used_fp      = popen("wmic os get freevirtualmemory", "r");      // free memory
  FILE* mem_total_fp     = popen("wmic os get totalvirtualmemorysize", "r"); // total memory
  char mem_used_ch[2137] = {0}, mem_total_ch[2137] = {0};

  while (fgets(mem_total_ch, sizeof(mem_total_ch), mem_total_fp) != NULL) {
    if (strstr(mem_total_ch, "TotalVirtualMemorySize") != 0)
      continue;
    else if (strstr(mem_total_ch, "  ") == 0)
      continue;
    else
      user_info->ram_total = atoi(mem_total_ch) / 1024;
  }
  LOG_V(user_info->ram_total);
  while (fgets(mem_used_ch, sizeof(mem_used_ch), mem_used_fp) != NULL) {
    if (strstr(mem_used_ch, "FreeVirtualMemory") != 0)
      continue;
    else if (strstr(mem_used_ch, "  ") == 0)
      continue;
    else
      user_info->ram_used = user_info->ram_total - (atoi(mem_used_ch) / 1024);
  }
  LOG_V(user_info->ram_used);
  pclose(mem_used_fp);
  pclose(mem_total_fp);
  #else // if not _WIN32
  char* buffer = ((struct thread_varg*)argp)->buffer;
  FILE* meminfo;

    #ifdef __BSD__
      #ifndef __OPENBSD__
  meminfo = popen("LANG=EN_us freecolor -om 2> /dev/null", "r"); // free alternative for freebsd
      #else
  meminfo = popen("LANG=EN_us vmstat 2> /dev/null | grep -v 'procs' | grep -v 'r' | awk '{print $3 "
                  "\" / \" $4}'",
                  "r"); // free alternative for openbsd
      #endif
    #else
  // getting memory info from /proc/meminfo: https://github.com/KittyKatt/screenFetch/issues/386#issuecomment-249312716
  meminfo = fopen("/proc/meminfo",
                  "r"); // popen("LANG=EN_us free -m 2> /dev/null", "r"); // get ram info with free
    #endif
  // brackets are here to restrict the access to this int variables, which are temporary
  {
    #ifndef __OPENBSD__
    int memtotal = 0, shmem = 0, memfree = 0, buffers = 0, cached = 0, sreclaimable = 0;
    #endif
    while (fgets(buffer, BUFFER_SIZE, meminfo)) {
    #ifndef __OPENBSD__
      sscanf(buffer, "MemTotal:       %d", &memtotal);
      sscanf(buffer, "Shmem:             %d", &shmem);
      sscanf(buffer, "MemFree:        %d", &memfree);
      sscanf(buffer, "Buffers:          %d", &buffers);
      sscanf(buffer, "Cached:          %d", &cached);
      sscanf(buffer, "SReclaimable:     %d", &sreclaimable);
    #else
      sscanf(buffer, "%dM / %dM", &user_info->ram_used, &user_info->ram_total);
    #endif
    }
    #ifndef __OPENBSD__
    user_info->ram_total = memtotal / 1024;
    user_info->ram_used = ((memtotal + shmem) - (memfree + buffers + cached + sreclaimable)) / 1024;
    #endif
    LOG_V(user_info->ram_total);
    LOG_V(user_info->ram_used);
  }

  fclose(meminfo);
  #endif
#else // if __APPLE__
  // Used
  FILE *mem_wired_fp, *mem_active_fp, *mem_compressed_fp;
  mem_wired_fp      = popen("vm_stat | awk '/wired/ { printf $4 }' | cut -d '.' -f 1", "r");
  mem_active_fp     = popen("vm_stat | awk '/active/ { printf $3 }' | cut -d '.' -f 1", "r");
  mem_compressed_fp = popen("vm_stat | awk '/occupied/ { printf $5 }' | cut -d '.' -f 1", "r");
  char mem_wired_ch[2137], mem_active_ch[2137], mem_compressed_ch[2137];
  while (fgets(mem_wired_ch, sizeof(mem_wired_ch), mem_wired_fp) != NULL)
    while (fgets(mem_active_ch, sizeof(mem_active_ch), mem_active_fp) != NULL)
      while (fgets(mem_compressed_ch, sizeof(mem_compressed_ch), mem_compressed_fp) != NULL)
        ;

  pclose(mem_wired_fp);
  pclose(mem_active_fp);
  pclose(mem_compressed_fp);

  int mem_wired      = atoi(mem_wired_ch);
  int mem_active     = atoi(mem_active_ch);
  int mem_compressed = atoi(mem_compressed_ch);

  // Total
  sysctlbyname("hw.memsize", &mem_buffer, &mem_buffer_len, NULL, 0);
  user_info->ram_used  = ((mem_wired + mem_active + mem_compressed) * 4 / 1024);
  user_info->ram_total = mem_buffer / 1024 / 1024;
  LOG_V(user_info->ram_total);
  LOG_V(user_info->ram_used);
#endif
  return 0;
}

// tries to get installed gpu(s)
void* get_gpu(void* argp) {
  if (!((struct thread_varg*)argp)->thread_flags[2]) return 0;
  LOG_I("getting gpu(s)");
  char* buffer           = ((struct thread_varg*)argp)->buffer;
  struct info* user_info = ((struct thread_varg*)argp)->user_info;
  int gpuc               = 0; // gpu counter
#ifndef _WIN32
  setenv("LANG", "en_US", 1); // force language to english
#endif
  FILE* gpu;
#ifndef _WIN32
  LOG_I("getting gpus with lshw");
  gpu = popen("lshw -class display 2> /dev/null", "r");

  // add all gpus to the array gpu_model
  while (fgets(buffer, BUFFER_SIZE, gpu))
    if (sscanf(buffer, "    product: %[^\n]", user_info->gpu_model[gpuc])) gpuc++;
#endif

  if (strlen(user_info->gpu_model[0]) < 2) {
    // get gpus with lspci command
    if (strcmp(user_info->os_name, "android") != 0) {
#ifndef __APPLE__
  #ifdef _WIN32
      gpu = popen("wmic PATH Win32_VideoController GET Name", "r");
  #else
      gpu = popen("lspci -mm 2> /dev/null | grep \"VGA\" | awk -F '\"' '{print $4 $5 $6}'", "r");
  #endif
#else
      gpu = popen(
          "system_profiler SPDisplaysDataType | awk -F ': ' '/Chipset Model: /{ print $2 }'", "r");
#endif
    } else
      gpu = popen("getprop ro.hardware.vulkan 2> /dev/null", "r"); // for android
  }

  // get all the gpus
  while (fgets(buffer, BUFFER_SIZE, gpu)) {
    // windows
    if (strstr(buffer, "Name") || (strlen(buffer) == 2))
      continue;
    else if (sscanf(buffer, "%[^\n]", user_info->gpu_model[gpuc]))
      gpuc++;
  }
  fclose(gpu);

  // format gpu names
  for (int i = 0; i < gpuc; i++) {
    remove_brackets(user_info->gpu_model[i]);
    truncate_str(user_info->gpu_model[i], user_info->target_width);
    LOG_V(user_info->gpu_model[i]);
  }
  return 0;
}

// tries to get screen resolution
#ifndef _WIN32
void* get_res(void* argp) {
  if (!((struct thread_varg*)argp)->thread_flags[3]) return 0;
  LOG_I("getting resolution");
  char* buffer           = ((struct thread_varg*)argp)->buffer;
  struct info* user_info = ((struct thread_varg*)argp)->user_info;
  FILE* resolution       = popen("xwininfo -root 2> /dev/null | grep -E 'Width|Height'", "r");
  while (fgets(buffer, BUFFER_SIZE, resolution)) {
    sscanf(buffer, "  Width: %d", &user_info->screen_width);
    sscanf(buffer, "  Height: %d", &user_info->screen_height);
  }
  LOG_V(user_info->screen_width);
  LOG_V(user_info->screen_height);
#else
void* get_res() {
  // TODO: get resolution on windows
#endif
  return 0;
}

// tries to get the installed package count and package managers name
void* get_pkg(void* argp) { // this is just a function that returns the total of installed packages
  if (!((struct thread_varg*)argp)->thread_flags[4]) return 0;
  LOG_I("getting pkgs");
  struct info* user_info = ((struct thread_varg*)argp)->user_info;
  user_info->pkgs        = 0;
#ifndef __APPLE__
  #ifndef _WIN32
  // all supported package managers
  struct package_manager pkgmans[] = {
      {PKGPATH "apt", "apt list --installed 2> /dev/null | wc -l", "(apt)"},
      {PKGPATH "apk", "apk info 2> /dev/null | wc -l", "(apk)"},
      // {PKGPATH"dnf","dnf list installed 2> /dev/null | wc -l", "(dnf)"}, // according to https://stackoverflow.com/questions/48570019/advantages-of-dnf-vs-rpm-on-fedora, dnf and rpm return the same number of packages
      {PKGPATH "qlist", "qlist -I 2> /dev/null | wc -l", "(emerge)"},
      {PKGPATH "flatpak", "flatpak list 2> /dev/null | wc -l", "(flatpak)"},
      {PKGPATH "snap", "snap list 2> /dev/null | wc -l", "(snap)"},
      {PKGPATH "guix", "guix package --list-installed 2> /dev/null | wc -l", "(guix)"},
      {PKGPATH "nix-store", "nix-store -q --requisites /run/current-system/sw 2> /dev/null | wc -l", "(nix)"},
      {PKGPATH "pacman", "pacman -Qq 2> /dev/null | wc -l", "(pacman)"},
      {PKGPATH "pkg", "pkg info 2>/dev/null | wc -l", "(pkg)"},
      {PKGPATH "pkg_info", "pkg_info 2>/dev/null | wc -l | sed \"s/ //g\"", "(pkg)"},
      {PKGPATH "port", "port installed 2> /dev/null | tail -n +2 | wc -l", "(port)"},
      {PKGPATH "brew", "find $(brew --cellar 2>/dev/stdout) -maxdepth 1 -type d 2> /dev/null | wc -l | awk '{print $1}'", "(brew-cellar)"},
      {PKGPATH "brew", "find $(brew --caskroom 2>/dev/stdout) -maxdepth 1 -type d 2> /dev/null | wc -l | awk '{print $1}'", "(brew-cask)"},
      {PKGPATH "rpm", "rpm -qa --last 2> /dev/null | wc -l", "(rpm)"},
      {PKGPATH "xbps-query", "xbps-query -l 2> /dev/null | wc -l", "(xbps)"}};
  #endif
#else
  struct package_manager pkgmans[] = {{"/usr/local/bin/brew", "find $(brew --cellar 2>/dev/stdout) -maxdepth 1 -type d 2> /dev/null | wc -l | awk '{print $1}' > /tmp/uwufetch_brew_tmp", "(brew-cellar)"},
                                      {"/usr/local/bin/brew", "find $(brew --caskroom 2>/dev/stdout) -maxdepth 1 -type d 2> /dev/null | wc -l | awk '{print $1}' > /tmp/uwufetch_brew_tmp", "(brew-cask)"}};
#endif
#ifndef _WIN32
  const int pkgman_count = sizeof(pkgmans) / sizeof(pkgmans[0]); // number of package managers
  int comma_separator    = 0;
  for (int i = 0; i < pkgman_count; i++) {
    struct package_manager* current = &pkgmans[i]; // pointer to current package manager

    unsigned int pkg_count = 0;
    LOG_I("trying pkgman %d: %s", i, current->pkgman_name);
    LOG_V(current->command_path);
    if (access(current->command_path, F_OK) != -1) {
  #ifndef __APPLE__
      FILE* fp = popen(current->command_string, "r"); // trying current package manager
  #else
      system(current->command_string); // writes to a temporary file: for some reason popen() does not intercept the stdout, so i have to read from a temporary file
      FILE* fp = fopen("/tmp/uwufetch_brew_tmp", "r");
  #endif
      if (fscanf(fp, "%u", &pkg_count) == 3) continue;

  #ifndef __APPLE__
      pclose(fp);
  #else
      // remove("/tmp/uwufetch_brew_tmp");
      fclose(fp);
  #endif
    }
  #ifdef __DEBUG__
    else
      LOG_W("pkgman %s executable not found!", current->pkgman_name);
  #endif

    // adding a package manager with its package count to user_info->pkgman_name
    user_info->pkgs += pkg_count;
    if (pkg_count > 0) {
      if (comma_separator++) strcat(user_info->pkgman_name, ", ");
      char spkg_count[16];
      sprintf(spkg_count, "%u", pkg_count);
      strcat(user_info->pkgman_name, spkg_count);
      strcat(user_info->pkgman_name, " ");
      strcat(user_info->pkgman_name, current->pkgman_name);
      LOG_V(user_info->pkgman_name);
    }
  }
#else  // _WIN32
  // chocolatey for windows
  FILE* fp = popen("choco list -l --no-color 2> nul", "r");
  unsigned int pkg_count;
  char buffer[7562] = {0};
  while (fgets(buffer, BUFFER_SIZE, fp)) {
    sscanf(buffer, "%u packages installed.", &pkg_count);
  }
  if (fp) pclose(fp);

  user_info->pkgs = pkg_count;
  char spkg_count[16];
  sprintf(spkg_count, "%u", pkg_count);
  strcat(user_info->pkgman_name, spkg_count);
  strcat(user_info->pkgman_name, " ");
  strcat(user_info->pkgman_name, "(chocolatey)");
  LOG_V(user_info->pkgman_name);
#endif // _WIN32
  return 0;
}

void* get_model(void* argp) {
  if (!((struct thread_varg*)argp)->thread_flags[5]) return 0;
  LOG_I("getting model");
  struct info* user_info = ((struct thread_varg*)argp)->user_info;
  char* buffer           = ((struct thread_varg*)argp)->buffer;
  FILE* model_fp;
#ifdef _WIN32
  // all the previous files obviously did not exist on windows
  model_fp = popen("wmic computersystem get model", "r");
  while (fgets(buffer, BUFFER_SIZE, model_fp)) {
    if (strstr(buffer, "Model") != 0)
      continue;
    else {
      sprintf(user_info->model, "%s", buffer);
      user_info->model[strlen(user_info->model) - 2] = '\0';
      break;
    }
  }
#elif defined(__BSD__) || defined(__APPLE__)
  #if defined(__BSD__) && !defined(__OPENBSD__)
    #define HOSTCTL "hw.hv_vendor"
  #elif defined(__APPLE__)
    #define HOSTCTL "hw.model"
  #elif defined(__OPENBSD__)
    #define HOSTCTL "hw.product"
  #endif
  model_fp = popen("sysctl " HOSTCTL, "r");
  while (fgets(buffer, BUFFER_SIZE, model_fp))
    if (sscanf(buffer,
               HOSTCTL
  #ifdef __OPENBSD__
               "="
  #else
               ": "
  #endif
               "%[^\n]",
               user_info->model))
      break;
  pclose(model_fp);
#else
  char model_filename[4][256] = {
      "/sys/devices/virtual/dmi/id/product_version",
      "/sys/devices/virtual/dmi/id/product_name",
      "/sys/devices/virtual/dmi/id/board_name",
      "getprop ro.product.vendor.marketname 2>/dev/null",
  };

  char tmp_model[4][BUFFER_SIZE] = {0}; // temporary variable to store the contents of all 3 files
  int longest_model = 0, best_len = 0, currentlen = 0;
  FILE* (*tocall[])(const char*, const char*) = {fopen, fopen, fopen, popen}; // open a process or a file, depending on the model_filename
  int (*tocall_close[])(FILE*)                = {fclose, fclose, fclose, pclose};
  for (int i = 0; i < 4; i++) {
    // read file
    model_fp = tocall[i](model_filename[i], "r");
    if (model_fp) {
      fgets(tmp_model[i], BUFFER_SIZE, model_fp);
      tmp_model[i][strlen(tmp_model[i]) - 1] = '\0';
      tocall_close[i](model_fp);
    }
    LOG_V(tmp_model[i]);
    // choose the file with the longest name
    currentlen = strlen(tmp_model[i]);
    if (currentlen > best_len) {
      best_len      = currentlen;
      longest_model = i;
    }
  }
  if (strlen(tmp_model[longest_model]) == 0) {
    model_fp = popen("lscpu 2>/dev/null", "r");
    while (fgets(buffer, BUFFER_SIZE, model_fp))
      if (sscanf(buffer, "Model name:%*[           |		]%[^\n]", tmp_model[longest_model]) == 1) break;
    pclose(model_fp);
    LOG_V(tmp_model[longest_model]);
    if (strcmp(tmp_model[longest_model], "Icestorm") == 0) sprintf(tmp_model[longest_model], "Apple MacBook Air (M1)");
  }
  sprintf(user_info->model, "%s", tmp_model[longest_model]);
  LOG_V(user_info->model);
#endif
  return 0;
}

void* get_ker(void* argp) {
  if (!((struct thread_varg*)argp)->thread_flags[6]) return 0;
  LOG_I("getting kernel");
  struct info* user_info = ((struct thread_varg*)argp)->user_info;

#ifndef _WIN32
  truncate_str(user_info->sys_var.release, user_info->target_width);
  sprintf(user_info->kernel, "%s %s %s", user_info->sys_var.sysname, user_info->sys_var.release, user_info->sys_var.machine); // kernel name
  truncate_str(user_info->kernel, user_info->target_width);
  LOG_V(user_info->kernel);
#else  // _WIN32
  // windows version
  FILE* kernel_fp = popen("wmic computersystem get systemtype", "r");
  char* buffer    = ((struct thread_varg*)argp)->buffer;
  while (fgets(buffer, BUFFER_SIZE, kernel_fp)) {
    if (strstr(buffer, "SystemType") != 0)
      continue;
    else {
      sprintf(user_info->kernel, "%s", buffer);
      user_info->kernel[strlen(user_info->kernel) - 2] = '\0';
      break;
    }
  }
  if (kernel_fp) pclose(kernel_fp);
#endif // _WIN32
  return 0;
}

void* get_upt(void* argp) {
  LOG_V(((struct thread_varg*)argp)->thread_flags[7]);
  if (!((struct thread_varg*)argp)->thread_flags[7]) return 0;
  LOG_I("getting uptime");
  struct info* user_info = ((struct thread_varg*)argp)->user_info;
#ifdef __APPLE__
  int mib[2] = {CTL_KERN, KERN_BOOTTIME};
  sysctl(mib, 2, &time_buffer, &time_buffer_len, NULL, 0);

  time_t bsec = time_buffer.tv_sec;
  time_t csec = time(NULL);

  user_info->uptime = difftime(csec, bsec);
#else
  #ifdef __BSD__
  // https://github.com/coreutils/coreutils/blob/master/src/uptime.c
  int boot_time         = 0;
  static int request[2] = {CTL_KERN, KERN_BOOTTIME};
  struct timeval result;
  size_t result_len = sizeof result;

  if (sysctl(request, 2, &result, &result_len, NULL, 0) >= 0) boot_time = result.tv_sec;
  int time_now      = time(NULL);
  user_info->uptime = time_now - boot_time;
  #else
    #ifdef _WIN32
  user_info->uptime = GetTickCount() / 1000;
    #else  // _WIN32
  user_info->uptime = user_info->sys.uptime;
    #endif // _WIN32
  #endif
#endif
  LOG_V(user_info->uptime);
  return 0;
}

// Retrieves system information
void get_info(struct flags flags, struct info* user_info) {
  char buffer[BUFFER_SIZE]; // line buffer
  get_twidth(user_info);
  // os version, cpu and board info
#ifdef __OPENBSD__
  FILE* os_release = popen("echo ID=openbsd", "r"); // os-release does not exist in OpenBSD
#else
  FILE* os_release  = fopen("/etc/os-release", "r"); // os name file
#endif
#ifndef __BSD__
  FILE* cpuinfo = fopen("/proc/cpuinfo", "r"); // cpu name file for not-freebsd systems
#else
  FILE* cpuinfo     = popen("sysctl hw.model", "r"); // cpu name command for freebsd
#endif
  // trying to get some kind of information about the name of the computer (hopefully a product full name)
  if (os_release) { // get normal vars if os_release exists
    if (flags.os) {
      LOG_I("getting os name from /etc/os-release");
      while (fgets(buffer, BUFFER_SIZE, os_release) &&
             !(sscanf(buffer, "\nID=\"%s\"", user_info->os_name) ||
               sscanf(buffer, "\nID=%s", user_info->os_name)))
        ;
      // sometimes for some reason sscanf reads the last '\"' too
      int os_name_len = strlen(user_info->os_name);
      if (user_info->os_name[os_name_len - 1] == '\"') {
        user_info->os_name[os_name_len - 1] = '\0';
      }
      // trying to detect amogos because in its os-release file ID value is just "debian", will be removed when amogos will have an os-release file with ID=amogos
      if (strcmp(user_info->os_name, "debian") == 0 ||
          strcmp(user_info->os_name, "raspbian") == 0) {
        DIR* amogos_plymouth = opendir("/usr/share/plymouth/themes/amogos");
        if (amogos_plymouth) {
          closedir(amogos_plymouth);
          sprintf(user_info->os_name, "amogos");
          LOG_V(user_info->os_name);
        }
      }
      LOG_V(user_info->os_name);
    }
  } else { // try for android vars, next for Apple var, or unknown system
           // android
    DIR* system_app      = opendir("/system/app/");
    DIR* system_priv_app = opendir("/system/priv-app/");
    DIR* library         = opendir("/Library/");
    if (system_app && system_priv_app) {
      closedir(system_app);
      closedir(system_priv_app);
      if (flags.os) sprintf(user_info->os_name, "android");
      LOG_V(user_info->os_name);
      if (flags.user) {
        // username
        FILE* whoami = popen("whoami", "r");
        if (fscanf(whoami, "%s", user_info->user) == 3) {
        }
        LOG_V(user_info->user);
        pclose(whoami);
      }
    } else if (library) { // Apple
      closedir(library);
#ifdef __APPLE__
      if (flags.cpu) {
        sysctlbyname("machdep.cpu.brand_string", &cpu_buffer, &cpu_buffer_len, NULL,
                     0); // cpu name
        sprintf(user_info->cpu_model, "%s", cpu_buffer);
      }
      if (flags.os) {
  #ifndef __IPHONE__
        sprintf(user_info->os_name, "macos");
  #else
        sprintf(user_info->os_name, "ios");
  #endif
      }
#endif
    } else // if no option before is working, the system is unknown
      sprintf(user_info->os_name, "unknown");
  }
#ifndef __BSD__
#endif
#ifndef _WIN32
  // getting username and hostname
  if (flags.user) {
    LOG_I("getting username and hostname");
    gethostname(user_info->host, 256);
    LOG_V(user_info->host);
    char* tmp_user = getenv("USER");
    LOG_V(tmp_user);
    if (tmp_user == NULL)
      sprintf(user_info->user, "%s", "");
    else
      sprintf(user_info->user, "%s", tmp_user);
    LOG_V(user_info->user);
    if (os_release) fclose(os_release);
  }
  if (flags.shell) {
    LOG_I("getting shell");
    char* tmp_shell = getenv("SHELL"); // shell name
    LOG_V(tmp_shell);
    if (!tmp_shell)
      sprintf(user_info->shell, "%s", "");
    else
      snprintf(user_info->shell, sizeof user_info->shell, "%s", tmp_shell);
  #ifdef __linux__
    if (strlen(user_info->shell) > 16) // android shell name was too long
      memmove(&user_info->shell, &user_info->shell[27], strlen(user_info->shell));
  #endif
    LOG_V(user_info->shell);
  }
#else  // if _WIN32
  // cpu name
  if (flags.cpu) {
    cpuinfo = popen("wmic cpu get caption", "r");
    while (fgets(buffer, BUFFER_SIZE, cpuinfo)) {
      if (strstr(buffer, "Caption") != 0)
        continue;
      else {
        sprintf(user_info->cpu_model, "%s", buffer);
        user_info->cpu_model[strlen(user_info->cpu_model) - 2] = '\0';
        break;
      }
    }
  }
  // username
  if (flags.user) {
    FILE* user_host_fp = popen("wmic computersystem get username", "r");
    while (fgets(buffer, BUFFER_SIZE, user_host_fp)) {
      if (strstr(buffer, "UserName") != 0)
        continue;
      else {
        sscanf(buffer, "%[^\\]%s", user_info->host, user_info->user);
        memmove(user_info->user, user_info->user + 1, sizeof(user_info->user) - 1);
        break;
      }
    }
  }
  // powershell version
  if (flags.shell) {
    FILE* shell_fp = popen("powershell $PSVersionTable", "r");
    sprintf(user_info->shell, "PowerShell ");
    char tmp_shell[64];
    while (fgets(buffer, BUFFER_SIZE, shell_fp) &&
           sscanf(buffer, "PSVersion                      %s", tmp_shell) == 0)
      ;
    strcat(user_info->shell, tmp_shell);
  }
#endif // _WIN32

#ifdef _WIN32
  if (flags.os) sprintf(user_info->os_name, "windows");
#endif
  get_sys(user_info);
  // are threads overpowered? nah
  void* (*fnptrs[])(void*) = {get_cpu, get_ram, get_gpu, get_res, get_pkg, get_model, get_ker, get_upt};
  struct thread_varg args =
      (struct thread_varg){buffer,
                           user_info,
                           cpuinfo,
                           {flags.cpu, flags.ram, flags.gpu, flags.resolution, flags.pkgs, flags.model, flags.kernel, flags.uptime}};
#define THREAD_COUNT 8
#ifndef _WIN32
  pthread_t tids[THREAD_COUNT] = {0};
#endif
  for (int i = 0; i < THREAD_COUNT; i++) {
    LOG_I("STARTING thread %d", i);
#ifdef _WIN32
    fnptrs[i](&args);
#else
    pthread_create(&tids[i], NULL, fnptrs[i], &args);
#endif
  }
#ifndef _WIN32
  for (int i = 0; i < THREAD_COUNT; i++) {
    if (tids[i] != 0) pthread_join(tids[i], NULL);
    LOG_I("JOINING thread %d", i);
  }
#endif
  fclose(cpuinfo);
}
