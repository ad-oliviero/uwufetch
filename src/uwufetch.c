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

#ifndef UWUFETCH_VERSION
  #define UWUFETCH_VERSION "unkown" // needs to be changed by the build script
#endif

#define _GNU_SOURCE // for strcasestr

#include "actrie.h"
#include "libfetch/fetch.h"
#include "libfetch/logging.h"
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
#endif // _WIN32

#include <dirent.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// COLORS
#define NORMAL "\x1b[0m"
#define BOLD "\x1b[1m"
#define BLACK "\x1b[30m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define SPRING_GREEN "\x1b[38;5;120m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[0;35m"
#define CYAN "\x1b[36m"
#define WHITE "\x1b[37m"
#define PINK "\x1b[38;5;201m"
#define LPINK "\x1b[38;5;213m"

#define BLOCK_CHAR "\u2587"

// moves the cursor after printing the image or the ascii logo
#ifdef _WIN32
const char MOVE_CURSOR[] = "\033[21C";
#else
const char MOVE_CURSOR[] = "\033[18C";
#endif // _WIN32

// all configuration flags available
struct configuration {
  bool user_name, shell, model, kernel, os_name, cpu,
      gpu_list, screen, memory, packages, uptime, colors; // all true by default
  char* logo_path;
};

// info that will be printed with the logo
struct info {
  char *user_name,
      *host_name,
      *shell,
      *model,
      *kernel,
      *os_name,
      *cpu,
      **gpu_list,
      *packages,
      *logo;
  int screen_width,
      screen_height,
      total_pkgs;
  unsigned long memory_total, memory_used;
  long uptime;

  struct winsize terminal_size;
};

// reads the config file
struct configuration parse_config(char* config_path) {
  char buffer[256]; // buffer for the current line
  FILE* config = NULL;
  // enabling all flags by default
  struct configuration configuration;
  memset(&configuration, true, sizeof(configuration));
  configuration.logo_path = malloc(sizeof(char) * 256);

  LOG_I("parsing config from");
#if defined(__DEBUG__)
  if (config_path == NULL)
    config_path = "./default.config";
#endif
  if (config_path == NULL) { // if config directory is not set, try to open the default
    if (getenv("HOME") != NULL) {
      char homedir[512];
      sprintf(homedir, "%s/.config/uwufetch/config", getenv("HOME"));
      LOG_V(homedir);
      config = fopen(homedir, "r");
      if (!config) {
        if (getenv("PREFIX") != NULL) {
          char prefixed_etc[512];
          sprintf(prefixed_etc, "%s/etc/uwufetch/config", getenv("PREFIX"));
          LOG_V(prefixed_etc);
          config = fopen(prefixed_etc, "r");
        } else {
          config = fopen("/etc/uwufetch/config", "r");
          LOG_V("/etc/uwufetch/config");
        }
      }
    }
  } else {
    config = fopen(config_path, "r");
    LOG_V(config_path);
  }
  if (config == NULL) return configuration; // if config file does not exist, return the defaults

  // reading the config file
  while (fgets(buffer, sizeof(buffer), config)) {
    if (sscanf(buffer, "logo=%s", configuration.logo_path) > 0) LOG_V(configuration.logo_path);
#define FIND_CFG_VAR(name)                        \
  if (sscanf(buffer, #name "="                    \
                           "%[truefalse]",        \
             buffer)) {                           \
    configuration.name = strcmp(buffer, "false"); \
    LOG_V(configuration.name);                    \
  }
    // reading other values
    FIND_CFG_VAR(user_name);
    FIND_CFG_VAR(os_name);
    FIND_CFG_VAR(model);
    FIND_CFG_VAR(kernel);
    FIND_CFG_VAR(cpu);
    FIND_CFG_VAR(gpu_list);
    FIND_CFG_VAR(memory);
    FIND_CFG_VAR(screen);
    FIND_CFG_VAR(shell);
    FIND_CFG_VAR(packages);
    FIND_CFG_VAR(uptime);
    FIND_CFG_VAR(colors);
#undef FIND_CFG_VAR
  }
  fclose(config);
  if (strlen(configuration.logo_path) == 0) {
    free(configuration.logo_path);
    configuration.logo_path = NULL;
  }
  return configuration;
}

// prints logo (as an image) of the given system.
int print_image(__attribute__((unused)) struct info* user_info) {
  /*
  LOG_I("printing image");
#ifndef __IPHONE__
  char command[256];
  if (strlen(user_info->image_name) < 1) {
    char* repl_str = strcmp(user_info->os_name, "android") == 0 ? "/data/data/com.termux/files/usr/lib/uwufetch/%s.png"
                     : strcmp(user_info->os_name, "macos") == 0 ? "/usr/local/lib/uwufetch/%s.png"
                                                                : "/usr/lib/uwufetch/%s.png";
    sprintf(user_info->image_name, repl_str, user_info->os_name); // image command for android
    LOG_V(user_info->image_name);
  }
  sprintf(command, "viu -t -w 18 -h 9 %s 2> /dev/null", user_info->image_name); // creating the command to show the image
  LOG_V(command);
  if (system(command) != 0) // if viu is not installed or the image is missing
    printf("\033[0E\033[3C%s\n"
           "   There was an\n"
           "    error: viu\n"
           "  is not installed\n"
           " or the image file\n"
           "   was not found\n"
           "   see IMAGES.md\n"
           "   for more info.\n\n",
           RED);
#else
  // unfortunately, the iOS stdlib does not have system(); because it reports that it is not available under iOS during compilation
  printf("\033[0E\033[3C%s\n"
         "   There was an\n"
         "   error: images\n"
         "   are currently\n"
         "  disabled on iOS.\n\n",
         RED);
#endif
  */
  LOG_E("image currently disabled");
  return 9;
}

// Replaces all terms in a string with another term.
void replace(char* original, const char* search, const char* replacer) {
  char* ch;
  char buffer[1024];
  ssize_t offset       = 0;
  ssize_t search_len   = (ssize_t)strlen(search);
  ssize_t replacer_len = (ssize_t)strlen(replacer);
  while ((ch = strstr(original + offset, search))) {
    strncpy(buffer, original, (size_t)(ch - original));
    buffer[ch - original] = 0;
    sprintf(buffer + (ch - original), "%s%s", replacer, ch + search_len);
    original[0] = 0;
    strcpy(original, buffer);
    offset = ch - original + replacer_len;
  }
}

// Replaces all terms in a string with another term, case insensitive
void replace_ignorecase(char* original, const char* search, const char* replacer) {
  char* ch;
  char buffer[1024];
  ssize_t offset = 0;
#ifdef _WIN32
  #define strcasestr(o, s) strstr(o, s)
#endif
  ssize_t search_len   = (ssize_t)strlen(search);
  ssize_t replacer_len = (ssize_t)strlen(replacer);
  while ((ch = strcasestr(original + offset, search))) {
    strncpy(buffer, original, (size_t)(ch - original));
    buffer[ch - original] = 0;
    sprintf(buffer + (ch - original), "%s%s", replacer, ch + search_len);
    original[0] = 0;
    strcpy(original, buffer);
    offset = ch - original + replacer_len;
  }
}

#ifdef _WIN32
// windows sucks and hasn't a strstep, so I copied one from https://stackoverflow.com/questions/8512958/is-there-a-windows-variant-of-strsep-function
char* strsep(char** stringp, const char* delim) {
  char* start = *stringp;
  char* p;
  p = (start != NULL) ? strpbrk(start, delim) : NULL;
  if (p == NULL)
    *stringp = NULL;
  else {
    *p       = '\0';
    *stringp = p + 1;
  }
  return start;
}
#endif

// uwufies distro name
void uwu_name(const struct actrie_t* replacer, struct info* user_info) {
  char* os_name = user_info->os_name;
  if (os_name == NULL) {
    return;
  }

  if (*os_name == '\0') {
    sprintf(os_name, "%s", "unknown");
    return;
  }

  actrie_t_replace_all_occurances(replacer, os_name);
}

// uwufies kernel name
void uwu_kernel(const struct actrie_t* replacer, char* kernel) {
  LOG_I("uwufing kernel");
  actrie_t_replace_all_occurances(replacer, kernel);
  LOG_V(kernel);
}

// uwufies hardware names
void uwu_hw(const struct actrie_t* replacer, char* hwname) {
  LOG_I("uwufing hardware")
  actrie_t_replace_all_occurances(replacer, hwname);
}

// uwufies package manager names
void uwu_pkgman(const struct actrie_t* replacer, char* pkgman_name) {
  LOG_I("uwufing package managers");
  actrie_t_replace_all_occurances(replacer, pkgman_name);
}

// uwufies everything
void uwufy_all(struct info* user_info) {
  struct actrie_t replacer;
  actrie_t_ctor(&replacer);

  const size_t PATTERNS_COUNT = 73;
  // Not necessary but recommended
  actrie_t_reserve_patterns(&replacer, PATTERNS_COUNT);

  // these package managers do not have edits yet:
  // apk, apt, guix, nix, pkg, xbps
  actrie_t_add_pattern(&replacer, "brew-cask", "bwew-cawsk");
  actrie_t_add_pattern(&replacer, "brew-cellar", "bwew-cewwaw");
  actrie_t_add_pattern(&replacer, "emerge", "emewge");
  actrie_t_add_pattern(&replacer, "flatpak", "fwatpakkies");
  actrie_t_add_pattern(&replacer, "pacman", "pacnyan");
  actrie_t_add_pattern(&replacer, "port", "powt");
  actrie_t_add_pattern(&replacer, "rpm", "rawrpm");
  actrie_t_add_pattern(&replacer, "snap", "snyap");
  actrie_t_add_pattern(&replacer, "zypper", "zyppew");

  actrie_t_add_pattern(&replacer, "lenovo", "LenOwO");
  actrie_t_add_pattern(&replacer, "cpu", "CPUwU");
  actrie_t_add_pattern(&replacer, "core", "Cowe");
  actrie_t_add_pattern(&replacer, "gpu", "GPUwU");
  actrie_t_add_pattern(&replacer, "graphics", "Gwaphics");
  actrie_t_add_pattern(&replacer, "corporation", "COwOpowation");
  actrie_t_add_pattern(&replacer, "nvidia", "NyaVIDIA");
  actrie_t_add_pattern(&replacer, "mobile", "Mwobile");
  actrie_t_add_pattern(&replacer, "intel", "Inteww");
  actrie_t_add_pattern(&replacer, "radeon", "Radenyan");
  actrie_t_add_pattern(&replacer, "geforce", "GeFOwOce");
  actrie_t_add_pattern(&replacer, "raspberry", "Nyasberry");
  actrie_t_add_pattern(&replacer, "broadcom", "Bwoadcom");
  actrie_t_add_pattern(&replacer, "motorola", "MotOwOwa");
  actrie_t_add_pattern(&replacer, "proliant", "ProLinyant");
  actrie_t_add_pattern(&replacer, "poweredge", "POwOwEdge");
  actrie_t_add_pattern(&replacer, "apple", "Nyapple");
  actrie_t_add_pattern(&replacer, "electronic", "ElectrOwOnic");
  actrie_t_add_pattern(&replacer, "processor", "Pwocessow");
  actrie_t_add_pattern(&replacer, "microsoft", "MicOwOsoft");
  actrie_t_add_pattern(&replacer, "ryzen", "Wyzen");
  actrie_t_add_pattern(&replacer, "advanced", "Adwanced");
  actrie_t_add_pattern(&replacer, "micro", "Micwo");
  actrie_t_add_pattern(&replacer, "devices", "Dewices");
  actrie_t_add_pattern(&replacer, "inc.", "Nyanc.");
  actrie_t_add_pattern(&replacer, "lucienne", "Lucienyan");
  actrie_t_add_pattern(&replacer, "tuxedo", "TUWUXEDO");
  actrie_t_add_pattern(&replacer, "aura", "Uwura");

  actrie_t_add_pattern(&replacer, "linux", "Linuwu");
  actrie_t_add_pattern(&replacer, "alpine", "Nyalpine");
  actrie_t_add_pattern(&replacer, "amogos", "AmogOwOS");
  actrie_t_add_pattern(&replacer, "android", "Nyandroid");
  actrie_t_add_pattern(&replacer, "arch", "Nyarch Linuwu");

  actrie_t_add_pattern(&replacer, "arcolinux", "ArcOwO Linuwu");

  actrie_t_add_pattern(&replacer, "artix", "Nyartix Linuwu");
  actrie_t_add_pattern(&replacer, "debian", "Debinyan");

  actrie_t_add_pattern(&replacer, "devuan", "Devunyan");

  actrie_t_add_pattern(&replacer, "deepin", "Dewepyn");
  actrie_t_add_pattern(&replacer, "endeavouros", "endeavOwO");
  actrie_t_add_pattern(&replacer, "fedora", "Fedowa");
  actrie_t_add_pattern(&replacer, "femboyos", "FemboyOWOS");
  actrie_t_add_pattern(&replacer, "gentoo", "GentOwO");
  actrie_t_add_pattern(&replacer, "gnu", "gnUwU");
  actrie_t_add_pattern(&replacer, "guix", "gnUwU gUwUix");
  actrie_t_add_pattern(&replacer, "linuxmint", "LinUWU Miwint");
  actrie_t_add_pattern(&replacer, "manjaro", "Myanjawo");
  actrie_t_add_pattern(&replacer, "manjaro-arm", "Myanjawo AWM");
  actrie_t_add_pattern(&replacer, "neon", "KDE NeOwOn");
  actrie_t_add_pattern(&replacer, "nixos", "nixOwOs");
  actrie_t_add_pattern(&replacer, "opensuse-leap", "OwOpenSUSE Leap");
  actrie_t_add_pattern(&replacer, "opensuse-tumbleweed", "OwOpenSUSE Tumbleweed");
  actrie_t_add_pattern(&replacer, "pop", "PopOwOS");
  actrie_t_add_pattern(&replacer, "raspbian", "RaspNyan");
  actrie_t_add_pattern(&replacer, "rocky", "Wocky Linuwu");
  actrie_t_add_pattern(&replacer, "slackware", "Swackwawe");
  actrie_t_add_pattern(&replacer, "solus", "sOwOlus");
  actrie_t_add_pattern(&replacer, "ubuntu", "Uwuntu");
  actrie_t_add_pattern(&replacer, "void", "OwOid");
  actrie_t_add_pattern(&replacer, "xerolinux", "xuwulinux");

  // BSD
  actrie_t_add_pattern(&replacer, "freebsd", "FweeBSD");
  actrie_t_add_pattern(&replacer, "openbsd", "OwOpenBSD");

  // Apple family
  actrie_t_add_pattern(&replacer, "macos", "macOwOS");
  actrie_t_add_pattern(&replacer, "ios", "iOwOS");

  // Windows
  actrie_t_add_pattern(&replacer, "windows", "WinyandOwOws");

  actrie_t_compute_links(&replacer);

  LOG_I("uwufing everything");
  if (user_info->kernel) uwu_kernel(&replacer, user_info->kernel);
  if (user_info->gpu_list)
    for (size_t i = 1; i <= (size_t)user_info->gpu_list[0]; i++)
      if (user_info->gpu_list[i])
        uwu_hw(&replacer, user_info->gpu_list[i]);

  if (user_info->cpu)
    uwu_hw(&replacer, user_info->cpu);
  LOG_V(user_info->cpu);

  if (user_info->model)
    uwu_hw(&replacer, user_info->model);
  LOG_V(user_info->model);

  if (user_info->packages)
    uwu_pkgman(&replacer, user_info->packages);
  LOG_V(user_info->packages);

  uwu_name(&replacer, user_info);

  actrie_t_dtor(&replacer);
}

// prints all the collected info and returns the number of printed lines
int print_info(struct configuration* configuration, struct info* user_info) {
  int line_count = 0;
// prints without overflowing the terminal width
#define responsively_printf(buf, format, ...)                   \
  {                                                             \
    sprintf(buf, format, __VA_ARGS__);                          \
    printf("%.*s\n", user_info->terminal_size.ws_col - 1, buf); \
    line_count++;                                               \
  }
  char print_buf[1024]; // for responsively print

  // print collected info - from host to cpu info
  if (configuration->user_name)
    responsively_printf(print_buf, "%s%s%s%s@%s", MOVE_CURSOR, NORMAL, BOLD, user_info->user_name, user_info->host_name);

  if (configuration->os_name)
    responsively_printf(print_buf, "%s%s%sOWOS     %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->os_name);
  if (configuration->model)
    responsively_printf(print_buf, "%s%s%sMOWODEL  %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->model);
  if (configuration->kernel)
    responsively_printf(print_buf, "%s%s%sKEWNEL   %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->kernel);
  if (configuration->cpu)
    responsively_printf(print_buf, "%s%s%sCPUWU    %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->cpu);

  if (configuration->gpu_list && user_info->gpu_list) {
    for (size_t i = 1; i <= (size_t)user_info->gpu_list[0]; i++) {
      if (user_info->gpu_list[i])
        responsively_printf(print_buf, "%s%s%sGPUWU    %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->gpu_list[i]);
    }
  }

  if (configuration->memory) // print ram
    responsively_printf(print_buf, "%s%s%sMEMOWY   %s%lu MiB/%lu MiB", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->memory_used, user_info->memory_total);
  if (configuration->screen) // print resolution
    if (user_info->screen_width != 0 || user_info->screen_height != 0)
      responsively_printf(print_buf, "%s%s%sSCWEEN%s   %dx%d", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->screen_width, user_info->screen_height);
  if (configuration->shell) // print shell name
    responsively_printf(print_buf, "%s%s%sSHEWW    %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->shell);
  if (configuration->packages) // print pkgs
    responsively_printf(print_buf, "%s%s%sPKGS     %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->packages);
  if (configuration->uptime) {
    // using chars because all the space provided by long or int types is not needed
    char secs  = (char)(user_info->uptime % 60);
    char mins  = (char)((user_info->uptime / 60) % 60);
    char hours = (char)((user_info->uptime / 3600) % 24);
    long days  = user_info->uptime / 86400;

    char str_secs[6]  = "";
    char str_mins[6]  = "";
    char str_hours[6] = "";
    char str_days[20] = "";

    sprintf(str_secs, "%is ", secs);
    sprintf(str_mins, "%im ", mins);
    sprintf(str_hours, "%ih ", hours);
    sprintf(str_days, "%lid ", days);

    responsively_printf(print_buf, "%s%s%sUWUPTIME %s%s%s%s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, days > 0 ? str_days : "", hours > 0 ? str_hours : "", mins > 0 ? str_mins : "", secs > 0 ? str_secs : "");
  }
  // clang-format off
  if (configuration->colors)
    printf("%s" BOLD BLACK BLOCK_CHAR BLOCK_CHAR RED BLOCK_CHAR
                BLOCK_CHAR GREEN BLOCK_CHAR BLOCK_CHAR YELLOW
                BLOCK_CHAR BLOCK_CHAR BLUE BLOCK_CHAR BLOCK_CHAR
                MAGENTA BLOCK_CHAR BLOCK_CHAR CYAN BLOCK_CHAR
                BLOCK_CHAR WHITE BLOCK_CHAR BLOCK_CHAR NORMAL "\n", MOVE_CURSOR);
  // clang-format on
  return line_count;
}

// writes cache to cache file
void write_cache(struct info* user_info) {
  LOG_I("writing cache");
  char cache_file[512];
  sprintf(cache_file, "%s/.cache/uwufetch.cache", getenv("HOME")); // default cache file location
  LOG_V(cache_file);
  FILE* cache_fp = fopen(cache_file, "wb");
  if (cache_fp == NULL) {
    LOG_E("Failed to write to %s!", cache_file);
    return;
  }
  // writing most of the values to config file
  uint32_t cache_size = (uint32_t)fprintf(
      cache_fp,
      "0000"                         // placeholder for the cache size
      "%s;%s;%s;%s;%s;%s;%s;%s;%s;", // no need to be human readable
      user_info->user_name, user_info->host_name, user_info->os_name, user_info->model,
      user_info->kernel, user_info->cpu, user_info->shell, user_info->packages, user_info->logo);

  // writing numbers before gpus (because gpus are a variable amount)
  cache_size += (uint32_t)(fwrite(&user_info->screen_width, sizeof(user_info->screen_width), 1, cache_fp) * sizeof(user_info->screen_width));
  cache_size += (uint32_t)(fwrite(&user_info->screen_height, sizeof(user_info->screen_height), 1, cache_fp) * sizeof(user_info->screen_height));

  // the first element of gpu_list is the number of gpus
  cache_size += (uint32_t)(fwrite(&user_info->gpu_list[0], sizeof(user_info->gpu_list[0]), 1, cache_fp) * sizeof(user_info->gpu_list[0]));

  for (size_t i = 1; i <= (size_t)user_info->gpu_list[0]; i++) // writing gpu names to file
    cache_size += (uint32_t)fprintf(cache_fp, ";%s", user_info->gpu_list[i]);

  // writing cache size at the beginning of the file
  fseek(cache_fp, 0, SEEK_SET);
  fwrite(&cache_size, sizeof(cache_size), 1, cache_fp);

  fclose(cache_fp);
  return;
}

// reads cache file if it exists
char* read_cache(struct info* user_info) {
  LOG_I("reading cache");
  char cache_fn[512];
  sprintf(cache_fn, "%s/.cache/uwufetch.cache", getenv("HOME"));
  LOG_V(cache_fn);
  FILE* cache_fp = fopen(cache_fn, "rb");
  if (cache_fp == NULL) {
    LOG_E("Failed to read from %s!", cache_fn);
    return NULL;
  }
  uint32_t cache_size = 0;
  fread(&cache_size, sizeof(cache_size), 1, cache_fp);
  char* start = malloc(cache_size);
  fread(start, cache_size, 1, cache_fp);
  fclose(cache_fp);
  char* buffer = start;

  // allocating memory
  user_info->user_name = start;
  user_info->host_name = strchr(user_info->user_name, ';') + 1;
  user_info->os_name   = strchr(user_info->host_name, ';') + 1;
  user_info->model     = strchr(user_info->os_name, ';') + 1;
  user_info->kernel    = strchr(user_info->model, ';') + 1;
  user_info->cpu       = strchr(user_info->kernel, ';') + 1;
  user_info->shell     = strchr(user_info->cpu, ';') + 1;
  user_info->packages  = strchr(user_info->shell, ';') + 1;
  user_info->logo      = strchr(user_info->packages, ';') + 1;
  buffer               = strchr(user_info->logo, ';') + 1;
  memcpy(&user_info->screen_width, buffer, sizeof(user_info->screen_width));
  buffer += sizeof(user_info->screen_width);
  memcpy(&user_info->screen_height, buffer, sizeof(user_info->screen_height));
  buffer += sizeof(user_info->screen_height);
  user_info->gpu_list = malloc(sizeof(char*));
  memcpy(&user_info->gpu_list[0], buffer, sizeof(user_info->gpu_list[0]));
  buffer += sizeof(user_info->gpu_list[0]) + 1;
  user_info->gpu_list = realloc(user_info->gpu_list, (size_t)user_info->gpu_list[0] + 1);
  memset(user_info->gpu_list + 1, 0, (size_t)user_info->gpu_list[0] * sizeof(char*));
  for (size_t i = 1; i <= (size_t)user_info->gpu_list[0]; i++) {
    user_info->gpu_list[i] = buffer;
    buffer                 = strchr(user_info->gpu_list[i], ';') + 1;
  }

  for (size_t i = 0; i < cache_size; i++)
    if (start[i] == ';') start[i] = 0;

  LOG_V(user_info->user_name);
  LOG_V(user_info->host_name);
  LOG_V(user_info->os_name);
  LOG_V(user_info->model);
  LOG_V(user_info->kernel);
  LOG_V(user_info->cpu);
  LOG_V(user_info->screen_width);
  LOG_V(user_info->screen_height);
  LOG_V(user_info->shell);
  LOG_V(user_info->packages);
  LOG_V(user_info->logo);
  LOG_V(user_info->screen_width);
  LOG_V(user_info->screen_height);
  for (size_t i = 1; i <= (size_t)user_info->gpu_list[0]; i++)
    LOG_V(user_info->gpu_list[i]);
  return start;
}

// prints logo (as ascii art) of the given system.
int print_ascii(char* path) {
  FILE* file = NULL;

  file = fopen(path, "r");
  if (!file) {
    LOG_E("Failed to open %s!", path);
    printf("Failed to open\n%s!", path);
    return 2;
  }
  char buffer[256];
  int line_count = 1;
  printf("\n");
  while (fgets(buffer, 256, file)) { // replacing color placecholders
    replace(buffer, "{NORMAL}", NORMAL);
    replace(buffer, "{BOLD}", BOLD);
    replace(buffer, "{BLACK}", BLACK);
    replace(buffer, "{RED}", RED);
    replace(buffer, "{GREEN}", GREEN);
    replace(buffer, "{SPRING_GREEN}", SPRING_GREEN);
    replace(buffer, "{YELLOW}", YELLOW);
    replace(buffer, "{BLUE}", BLUE);
    replace(buffer, "{MAGENTA}", MAGENTA);
    replace(buffer, "{CYAN}", CYAN);
    replace(buffer, "{WHITE}", WHITE);
    replace(buffer, "{PINK}", PINK);
    replace(buffer, "{LPINK}", LPINK);
    replace(buffer, "{BLOCK}", BLOCK_CHAR);
    replace(buffer, "{BLOCK_VERTICAL}", BLOCK_CHAR);
    replace(buffer, "{BACKGROUND_GREEN}", "\033[0;42m");
    replace(buffer, "{BACKGROUND_RED}", "\033[0;41m");
    replace(buffer, "{BACKGROUND_WHITE}", "\033[0;47m");
    printf("%s", buffer); // print the line after setting the color
    line_count++;
  }
  // Always set color to NORMAL, so there's no need to do this in every ascii file.
  printf(NORMAL);
  fclose(file);
  return line_count;
}

/* prints distribution list
   distributions are listed by distribution branch
   to make the output easier to understand by the user.*/
void list(char* arg) {
  LOG_I("printing supported distro list");
  // clang-format off
	printf("%s -d <options>\n"
				 "  Available distributions:\n"
				 "    "BLUE"Arch linux "NORMAL"based:\n"
				 "      "BLUE"arch, arcolinux, "MAGENTA"artix, endeavouros "GREEN"manjaro, manjaro-arm, "BLUE"xerolinux\n\n"
				 "    "RED"Debian/"YELLOW"Ubuntu "NORMAL"based:\n"
				 "      "RED"amogos, debian, deepin, "GREEN"linuxmint, neon, "BLUE"pop, "RED"raspbian "YELLOW"ubuntu\n\n"
				 "    "RED"BSD "NORMAL"based:\n"
				 "      "RED"freebsd, "YELLOW"openbsd, "GREEN"m"YELLOW"a"RED"c"PINK"o"BLUE"s, "WHITE"ios\n\n"
				 "    "RED"RHEL "NORMAL"based:\n"
				 "      "BLUE"fedora, "GREEN"rocky\n\n"
				 "    "NORMAL"Other/spare distributions:\n"
				 "      "BLUE"alpine, "PINK"femboyos, gentoo, "MAGENTA"slackware, "WHITE"solus, "GREEN"void, opensuse-leap, android, "YELLOW"gnu, guix, "BLUE"windows, "WHITE"unknown\n\n",
				 arg); // Other/spare distributions colors
  // clang-format on
}

// prints the usage
void usage(char* arg) {
  LOG_I("printing usage");
  // TODO: add some more info
  printf("Usage: %s <args>\n"
         "    -c  --config        use custom config path\n"
         "    -d, --distro        lets you choose the logo to print\n"
         "    -h, --help          prints this help page\n"
#ifndef __IPHONE__
         "    -i, --image         prints logo as image and use a custom image "
         "if provided\n"
         "                        %sworks in most terminals\n"
#else
         "    -i, --image         prints logo as image and use a custom image "
         "if provided\n"
         "                        %sdisabled under iOS\n"
#endif
         "                        read README.md for more info%s\n"
         "    -l, --list          lists all supported distributions\n"
         "    -V, --version       prints the current uwufetch version\n"
#ifdef __DEBUG__
         "    -v, --verbose       sets logging level\n"
#endif
         "    -w, --write-cache   writes to the cache file (~/.cache/uwufetch.cache)\n"
         "    -r, --read-cache    reads from the cache file (~/.cache/uwufetch.cache)\n",
         arg,
#ifndef __IPHONE__
         BLUE,
#else
         RED,
#endif
         NORMAL);
}

// count the number of ansi escape code-related characters
size_t aeccount(const char* str) {
  size_t count = 0;
  while (*str != 0) {
    if (*str == '\x1b') {
      while (*str != '\0' && *str != 'm') {
        str++;
        count++;
      }
    }
    str++;
  }
  return count;
}

char* render(struct info* user_info, struct configuration* configuration) {
  // yes this is ugly and bad to maintain, I'll find a better solution later
  size_t enabled_flags = (size_t)(configuration->user_name + configuration->shell + configuration->model + configuration->kernel +
                                  configuration->os_name + configuration->cpu + configuration->screen + configuration->memory +
                                  configuration->packages + configuration->uptime + configuration->colors) +
                         ((size_t)configuration->gpu_list * (size_t)user_info->gpu_list[0]);
  const size_t width  = (size_t)(user_info->terminal_size.ws_col);
  const size_t height = (size_t)(enabled_flags < user_info->terminal_size.ws_row ? enabled_flags : user_info->terminal_size.ws_row);
  const size_t buf_sz = ((width * height) + 1) * 2;
  char* buffer        = malloc(buf_sz);
  memset(buffer, ' ', buf_sz);
  size_t current_line = 0;
#define LOGO_OFFSET 20
  size_t cursor = (width * current_line++) + LOGO_OFFSET;
#define PRINTLN_BUF(offset, format, ...)                                                     \
  {                                                                                          \
    snprintf(buffer + cursor, width - offset + aeccount(format) + 3, format, ##__VA_ARGS__); \
    cursor += strlen(buffer + cursor) + (size_t)offset + 1;                                  \
  }

  if (configuration->user_name) PRINTLN_BUF(LOGO_OFFSET, BOLD "%s@%s" NORMAL, user_info->user_name, user_info->host_name);
  if (configuration->os_name) PRINTLN_BUF(LOGO_OFFSET, BOLD "OWOS" NORMAL "     %s", user_info->os_name);
  if (configuration->model) PRINTLN_BUF(LOGO_OFFSET, BOLD "MOWODEL" NORMAL "  %s", user_info->model);
  if (configuration->kernel) PRINTLN_BUF(LOGO_OFFSET, BOLD "KEWNEL" NORMAL "   %s", user_info->kernel);
  if (configuration->cpu) PRINTLN_BUF(LOGO_OFFSET, BOLD "CPUWU" NORMAL "    %s", user_info->cpu);
  if (configuration->gpu_list && user_info->gpu_list)
    for (size_t i = 1; i <= (size_t)user_info->gpu_list[0]; i++)
      PRINTLN_BUF(LOGO_OFFSET, BOLD "GPUWU" NORMAL "    %s", user_info->gpu_list[i]);
  if (configuration->memory) PRINTLN_BUF(LOGO_OFFSET, BOLD "MEMOWY" NORMAL "   %lu MiB/%lu MiB", user_info->memory_used, user_info->memory_total);
  if (configuration->screen && (user_info->screen_width != 0 || user_info->screen_height != 0))
    PRINTLN_BUF(LOGO_OFFSET, BOLD "SCWEEN" NORMAL "   %dx%d", user_info->screen_width, user_info->screen_height);
  if (configuration->shell) PRINTLN_BUF(LOGO_OFFSET, BOLD "SHEWW" NORMAL "    %s", user_info->shell);
  if (configuration->packages) PRINTLN_BUF(LOGO_OFFSET, BOLD "PKGS" NORMAL "     %s", user_info->packages);
  if (configuration->uptime) {
    // split the uptime before printing it
    char secs  = (char)(user_info->uptime % 60);
    char mins  = (char)((user_info->uptime / 60) % 60);
    char hours = (char)((user_info->uptime / 3600) % 24);
    long days  = user_info->uptime / 86400;

    char str_secs[6]  = "";
    char str_mins[6]  = "";
    char str_hours[6] = "";
    char str_days[20] = "";

    sprintf(str_secs, "%is ", secs);
    sprintf(str_mins, "%im ", mins);
    sprintf(str_hours, "%ih ", hours);
    sprintf(str_days, "%lid ", days);
    PRINTLN_BUF(LOGO_OFFSET, BOLD "UWUPTIME" NORMAL " %s%s%s%s", days > 0 ? str_days : "", hours > 0 ? str_hours : "", mins > 0 ? str_mins : "", secs > 0 ? str_secs : "");
  }

  // clang-format off
    if (configuration->colors) {
#define COLOR_STRING BOLD WHITE BLOCK_CHAR BLOCK_CHAR CYAN BLOCK_CHAR \
                       BLOCK_CHAR MAGENTA BLOCK_CHAR BLOCK_CHAR BLUE \
                       BLOCK_CHAR BLOCK_CHAR YELLOW BLOCK_CHAR BLOCK_CHAR \
                       GREEN BLOCK_CHAR BLOCK_CHAR RED BLOCK_CHAR \
                       BLOCK_CHAR BLACK BLOCK_CHAR BLOCK_CHAR NORMAL
#define COLOR_STRING_LEN sizeof(COLOR_STRING)
      snprintf(buffer + cursor, width - COLOR_STRING_LEN, COLOR_STRING);
      cursor += COLOR_STRING_LEN;
    }
  // clang-format on

  // replace all the null terminators added by snprintf
  for (size_t i = 0; i < buf_sz; i++)
    if (buffer[i] == 0) buffer[i] = '\n';

  // null terminate after the last char
  buffer[cursor < buf_sz ? cursor : buf_sz - 1] = 0;
  return buffer;
}

// the main function is on the bottom of the file to avoid double function declarations
int main(int argc, char** argv) {
  struct configuration configuration = parse_config(NULL);
  struct info user_info              = {0};
  struct {
    char* content;
    bool read,
        write;
  } cache = {NULL, false, false};

#ifdef _WIN32
  // packages disabled by default because chocolatey is too slow
  configuration.packages = 0;
#endif

  int opt                      = 0;
  struct option long_options[] = {
      {"config", required_argument, NULL, 'c'},
      {"distro", required_argument, NULL, 'd'},
      {"help", no_argument, NULL, 'h'},
      {"image", optional_argument, NULL, 'i'},
      {"list", no_argument, NULL, 'l'},
      {"read-cache", no_argument, NULL, 'r'},
      {"version", no_argument, NULL, 'V'},
#ifdef LOGGING_ENABLED
      {"verbose", optional_argument, NULL, 'v'},
#endif
      {"write-cache", no_argument, NULL, 'w'},
      {0}};
#ifdef LOGGING_ENABLED
  #define OPT_STRING "c:d:hi::lrVv::w"
#else
  #define OPT_STRING "c:d:hi::lrVw"
#endif

  // reading cmdline options
  while ((opt = getopt_long(argc, argv, OPT_STRING, long_options, NULL)) != -1) {
    switch (opt) {
    case 'c': // set the config directory
      configuration = parse_config(optarg);
      break;
    case 'd': // set the distribution name
      // custom_distro_name = optarg;
      break;
    case 'h':
      usage(argv[0]);
      return 0;
    case 'i': // set ascii logo as output
      // configuration.image = true;
      // if (argv[optind]) custom_image_name = argv[optind];
      break;
    case 'l':
      list(argv[0]);
      return 0;
    case 'r':
      cache.read  = true;
      cache.write = false;
      break;
    case 'V':
      printf("UwUfetch version %s\n", UWUFETCH_VERSION);
      return 0;
#if defined(LOGGING_ENABLED)
    case 'v':
      if (argv[optind]) {
        SET_LOG_LEVEL(atoi(argv[optind]), "uwufetch");
        char* sep = strchr(argv[optind], ',');
        if (sep) *(sep++) = '\0';
        SET_LIBFETCH_LOG_LEVEL(atoi(sep ? sep : argv[optind]));
      }
      LOG_I("version %s", UWUFETCH_VERSION);
      break;
#endif
    case 'w':
      cache.write = true;
      break;
    default:
      return 1;
    }
  }
  libfetch_init();

  if (cache.read) {
    cache.content = read_cache(&user_info);
    // if no cache file found write to it
    if (!cache.content) {
      cache.read  = false;
      cache.write = true;
    }
  }
  if (!cache.read) {
#define IF_ENABLED_GET(name) \
  if (configuration.name) user_info.name = get_##name();
    if (configuration.user_name) {
      user_info.user_name = get_user_name();
      user_info.host_name = get_host_name();
    }
    IF_ENABLED_GET(shell);
    IF_ENABLED_GET(model);
    IF_ENABLED_GET(kernel);
    if (!user_info.os_name) user_info.os_name = get_os_name(); // get os name only if it was not set by either the configuration or the cli args
    IF_ENABLED_GET(cpu);
    IF_ENABLED_GET(gpu_list);
    IF_ENABLED_GET(packages);
    user_info.terminal_size = get_terminal_size();
    if (configuration.screen) {
      user_info.screen_height = get_screen_height();
      user_info.screen_width  = get_screen_width();
    }
#if defined(SYSTEM_BASE_ANDROID)
    if (configuration->shell)
      if (strlen(user_info.shell) > 27) // android shell name was too long
        user_info.shell += 27;
#endif
  }
  if (configuration.memory) {
    user_info.memory_total = get_memory_total();
    user_info.memory_used  = get_memory_used();
  }
  IF_ENABLED_GET(uptime);
#undef IF_ENABLED_GET

  // print ascii or image and align cursor for print_info()
#if defined(__DEBUG__)
  #define PREFIX "./res/"
#else
  #if defined(SYSTEM_BASE_LINUX)
    #define PREFIX "/usr/lib/uwufetch/"
  #elif defined(SYSTEM_BASE_ANDROID)
    #define PREFIX "/data/data/com.termux/files/usr/lib/uwufetch/"
  #elif defined(SYSTEM_BASE_FREEBSD)
    #define PREFIX "/usr/lib/uwufetch/"
  #elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not Implemented");
  #elif defined(SYSTEM_BASE_MACOS)
    #define PREFIX "/usr/local/lib/uwufetch/"
  #elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not Implemented");
  #else
  LOG_E("System not supported or system base not specified");
  #endif
#endif
  // TODO: implement image printing
  if (!configuration.logo_path) {
    if (!user_info.os_name) {
      configuration.logo_path = malloc(sizeof(char) * (strlen(PREFIX) + strlen("ascii/unknown.txt") + 1));
      sprintf(configuration.logo_path, PREFIX "ascii/unknown.txt");
    } else {
      configuration.logo_path = malloc(sizeof(char) * (strlen(PREFIX) + strlen("ascii/.txt") + strlen(user_info.os_name) + 1));
      sprintf(configuration.logo_path, PREFIX "ascii/%s.txt", user_info.os_name);
    }
  }
  LOG_V(configuration.logo_path);
  user_info.logo = configuration.logo_path;
  if (!cache.read) uwufy_all(&user_info);
  if (cache.write) write_cache(&user_info);

  char* render_buf = render(&user_info, &configuration);
  if (render_buf) {
    printf("%s\n", render_buf);
    free(render_buf);
  }

  libfetch_cleanup();
  if (cache.read) {
    if (cache.content) free(cache.content);
    if (user_info.gpu_list) free(user_info.gpu_list);
  }
  if (configuration.logo_path) free(configuration.logo_path);
  LOG_I("Execution completed successfully with %d errors", logging_error_count);
  return 0;
}
