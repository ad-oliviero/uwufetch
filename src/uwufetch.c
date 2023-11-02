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

#ifdef _WIN32
const char MOVE_CURSOR[] = "\033[21C"; // moves the cursor after printing the image or the ascii logo
#else
const char MOVE_CURSOR[] = "\033[18C";
#endif // _WIN32

// all configuration flags available
struct configuration {
  bool user, shell, model, kernel, get_gpu,
      os, cpu, resolution, ram, pkgs, uptime; // all true by default
  bool image,                                 // false by default
      colors;                                 // true by default
  bool gpu[256];
  bool gpus; // global gpu toggle
};

// user's config stored on the disk
struct user_config {
  char *config_directory, // configuration directory name
      *cache_content;     // cache file content
  int read_enabled, write_enabled;
};

// info that will be printed with the logo
struct info {
  char *user_name,
      *host_name,
      *shell,
      *model,
      *kernel,
      *os_name,
      *cpu_model,
      **gpus,
      *packages,
      *image_name;
  int screen_width,
      screen_height,
      total_pkgs;
  unsigned long ram_total, ram_used;
  long uptime;

  struct winsize term_size;
};

// reads the config file
struct configuration parse_config(struct info* user_info, struct user_config* user_config_file) {
  LOG_I("parsing config");
  char buffer[256]; // buffer for the current line
  // enabling all flags by default
  struct configuration config_flags;
  memset(&config_flags, true, sizeof(config_flags));

  config_flags.image = false;

  FILE* config = NULL; // config file pointer

  if (user_config_file->config_directory == NULL) { // if config directory is not set, try to open the default
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
        } else
          config = fopen("/etc/uwufetch/config", "r");
      }
    }
  } else
    config = fopen(user_config_file->config_directory, "r");
  if (config == NULL) return config_flags; // if config file does not exist, return the defaults

  int gpu_cfg_count = 0;

  // reading the config file
  while (fgets(buffer, sizeof(buffer), config)) {
    sscanf(buffer, "distro=%s", user_info->os_name);
    if (sscanf(buffer, "image=\"%[^\"]\"", user_info->image_name)) {
      if (user_info->image_name[0] == '~') {                                                          // replacing the ~ character with the home directory
        memmove(&user_info->image_name[0], &user_info->image_name[1], strlen(user_info->image_name)); // remove the first char
        char temp[128] = "/home/";
        strcat(temp, user_info->user_name);
        strcat(temp, user_info->image_name);
        sprintf(user_info->image_name, "%s", temp);
      }
      config_flags.image = 1; // enable the image flag
    }

    // reading other values
    if (sscanf(buffer, "user=%[truefalse]", buffer)) {
      config_flags.user = !strcmp(buffer, "true");
      LOG_V(config_flags.user);
    }
    if (sscanf(buffer, "os=%[truefalse]", buffer)) {
      config_flags.os = strcmp(buffer, "false");
      LOG_V(config_flags.os);
    }
    if (sscanf(buffer, "host=%[truefalse]", buffer)) {
      config_flags.model = strcmp(buffer, "false");
      LOG_V(config_flags.model);
    }
    if (sscanf(buffer, "kernel=%[truefalse]", buffer)) {
      config_flags.kernel = strcmp(buffer, "false");
      LOG_V(config_flags.kernel);
    }
    if (sscanf(buffer, "cpu=%[truefalse]", buffer)) {
      config_flags.cpu = strcmp(buffer, "false");
      LOG_V(config_flags.cpu);
    }
    if (sscanf(buffer, "gpu=%d", &gpu_cfg_count)) {
      if (gpu_cfg_count > 255) {
        LOG_E("gpu config index is too high, setting it to 255");
        gpu_cfg_count = 255;
      } else if (gpu_cfg_count < 0) {
        LOG_E("gpu config index is too low, setting it to 0");
        gpu_cfg_count = 0;
      }
      config_flags.gpu[gpu_cfg_count] = false;
      LOG_V(config_flags.gpu[gpu_cfg_count]);
    }
    if (sscanf(buffer, "gpus=%[truefalse]", buffer)) { // global gpu toggle
      if (strcmp(buffer, "false") == 0) {
        config_flags.gpus    = false;
        config_flags.get_gpu = false; // enable getting gpu info
      } else {
        config_flags.gpus    = true;
        config_flags.get_gpu = true;
      }
      LOG_V(config_flags.gpus);
      LOG_V(config_flags.gpu);
    }
    if (sscanf(buffer, "ram=%[truefalse]", buffer)) {
      config_flags.ram = strcmp(buffer, "false");
      LOG_V(config_flags.ram);
    }
    if (sscanf(buffer, "resolution=%[truefalse]", buffer)) {
      config_flags.resolution = strcmp(buffer, "false");
      LOG_V(config_flags.resolution);
    }
    if (sscanf(buffer, "shell=%[truefalse]", buffer)) {
      config_flags.shell = strcmp(buffer, "false");
      LOG_V(config_flags.shell);
    }
    if (sscanf(buffer, "pkgs=%[truefalse]", buffer)) {
      config_flags.pkgs = strcmp(buffer, "false");
      LOG_V(config_flags.pkgs);
    }
    if (sscanf(buffer, "uptime=%[truefalse]", buffer)) {
      config_flags.uptime = strcmp(buffer, "false");
      LOG_V(config_flags.uptime);
    }
    if (sscanf(buffer, "colors=%[truefalse]", buffer)) {
      config_flags.colors = strcmp(buffer, "false");
      LOG_V(config_flags.colors);
    }
  }
  LOG_V(user_info->os_name);
  LOG_V(user_info->image_name);
  fclose(config);
  return config_flags;
}

// prints logo (as an image) of the given system.
int print_image(struct info* user_info) {
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
  if (user_info->gpus)
    for (int i = 0; i < 256; i++)
      if (user_info->gpus[i])
        uwu_hw(&replacer, user_info->gpus[i]);

  if (user_info->cpu_model)
    uwu_hw(&replacer, user_info->cpu_model);
  LOG_V(user_info->cpu_model);

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
int print_info(struct configuration* config_flags, struct info* user_info) {
  int line_count = 0;
// prints without overflowing the terminal width
#define responsively_printf(buf, format, ...)               \
  {                                                         \
    sprintf(buf, format, __VA_ARGS__);                      \
    printf("%.*s\n", user_info->term_size.ws_col - 1, buf); \
    line_count++;                                           \
  }
  char print_buf[1024]; // for responsively print

  // print collected info - from host to cpu info
  if (config_flags->user)
    responsively_printf(print_buf, "%s%s%s%s@%s", MOVE_CURSOR, NORMAL, BOLD, user_info->user_name, user_info->host_name);

  if (config_flags->os)
    responsively_printf(print_buf, "%s%s%sOWOS     %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->os_name);
  if (config_flags->model)
    responsively_printf(print_buf, "%s%s%sMOWODEL  %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->model);
  if (config_flags->kernel)
    responsively_printf(print_buf, "%s%s%sKEWNEL   %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->kernel);
  if (config_flags->cpu)
    responsively_printf(print_buf, "%s%s%sCPUWU    %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->cpu_model);

  if (user_info->gpus)
    for (int i = 0; i < 256; i++) {
      if (config_flags->gpu[i])
        if (user_info->gpus[i])
          responsively_printf(print_buf, "%s%s%sGPUWU    %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->gpus[i]);
    }

  if (config_flags->ram) // print ram
    responsively_printf(print_buf, "%s%s%sMEMOWY   %s%lu MiB/%lu MiB", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->ram_used, user_info->ram_total);
  if (config_flags->resolution) // print resolution
    if (user_info->screen_width != 0 || user_info->screen_height != 0)
      responsively_printf(print_buf, "%s%s%sSCWEEN%s   %dx%d", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->screen_width, user_info->screen_height);
  if (config_flags->shell) // print shell name
    responsively_printf(print_buf, "%s%s%sSHEWW    %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->shell);
  if (config_flags->pkgs) // print pkgs
    responsively_printf(print_buf, "%s%s%sPKGS     %s%s", MOVE_CURSOR, NORMAL, BOLD, NORMAL, user_info->packages);
  if (config_flags->uptime) {
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
  if (config_flags->colors)
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
  FILE* cache_fp = fopen(cache_file, "w");
  if (cache_fp == NULL) {
    LOG_E("Failed to write to %s!", cache_file);
    return;
  }
  // writing all info to the cache file
  fprintf( // writing most of the values to config file
      cache_fp,
      "user=%s\nhost=%s\nversion_name=%s\nhost_model=%s\nkernel=%s\ncpu=%"
      "s\nscreen_width=%d\nscreen_height=%d\nshell=%s\npkgs=%s\n",
      user_info->user_name, user_info->host_name, user_info->os_name, user_info->model, user_info->kernel,
      user_info->cpu_model, user_info->screen_width, user_info->screen_height, user_info->shell, user_info->packages);

  // TODO: re-enable gpu array caching
  // for (int i = 0; i < 256; i++) // writing gpu names to file
  //   if (user_info->gpus[i]) fprintf(cache_fp, "gpu=%s\n", user_info->gpus[i]);

  fclose(cache_fp);
  return;
}

// reads cache file if it exists
int read_cache(struct info* user_info) {
  LOG_I("reading cache");
  char cache_file[512];
  sprintf(cache_file, "%s/.cache/uwufetch.cache", getenv("HOME"));
  LOG_V(cache_file);
  FILE* cache_fp = fopen(cache_file, "r");
  if (cache_fp == NULL) return 0;
  char buffer[256];
  // int gpuc = 0;

// allocating memory
#define DEFAULT_MAX_STRLEN 1024
  char user_name_format[32] = "";
  char host_name_format[32] = "";
  long max_user_name_len    = sysconf(_SC_LOGIN_NAME_MAX);
  long max_host_name_len    = sysconf(_SC_HOST_NAME_MAX);
  user_info->user_name      = malloc(max_user_name_len > 0 ? (size_t)max_user_name_len : DEFAULT_MAX_STRLEN);
  user_info->host_name      = malloc(max_host_name_len > 0 ? (size_t)max_host_name_len : DEFAULT_MAX_STRLEN);
  snprintf(user_name_format, sizeof(user_name_format), "user=%%%ld[^\n]", max_user_name_len);
  snprintf(host_name_format, sizeof(host_name_format), "host=%%%ld[^\n]", max_host_name_len);
  user_info->os_name   = malloc(DEFAULT_MAX_STRLEN);
  user_info->model     = malloc(DEFAULT_MAX_STRLEN);
  user_info->kernel    = malloc(DEFAULT_MAX_STRLEN);
  user_info->cpu_model = malloc(DEFAULT_MAX_STRLEN);
  // user_info->gpus[0] = malloc(DEFAULT_MAX_STRLEN);
  user_info->packages = malloc(DEFAULT_MAX_STRLEN);

  while (fgets(buffer, sizeof(buffer), cache_fp)) { // reading the file
    sscanf(buffer, user_name_format, user_info->user_name);
    sscanf(buffer, host_name_format, user_info->host_name);
    sscanf(buffer, "version_name=%99[^\n]", user_info->os_name);
    sscanf(buffer, "host_model=%99[^\n]", user_info->model);
    sscanf(buffer, "kernel=%99[^\n]", user_info->kernel);
    sscanf(buffer, "cpu=%99[^\n]", user_info->cpu_model);
    // TODO: re-enable gpu array cache reading
    // if (sscanf(buffer, "gpu=%99[^\n]", user_info->gpus[gpuc]) != 0) gpuc++;
    sscanf(buffer, "screen_width=%i", &user_info->screen_width);
    sscanf(buffer, "screen_height=%i", &user_info->screen_height);
    sscanf(buffer, "shell=%99[^\n]", user_info->shell);
    sscanf(buffer, "pkgs=%99[^\n]", user_info->packages);
  }
#undef DEFAULT_MAX_STRLEN
  LOG_V(user_info->user_name);
  LOG_V(user_info->host_name);
  LOG_V(user_info->os_name);
  LOG_V(user_info->model);
  LOG_V(user_info->kernel);
  LOG_V(user_info->cpu_model);
  // LOG_V(user_info->gpus[gpuc]);
  LOG_V(user_info->screen_width);
  LOG_V(user_info->screen_height);
  LOG_V(user_info->shell);
  LOG_V(user_info->packages);
  fclose(cache_fp);
  return 1;
}

// prints logo (as ascii art) of the given system.
int print_ascii(struct info* user_info) {
  FILE* file            = NULL;
  char ascii_file[1024] = "";
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
  if (user_info->os_name)
    if (strlen(user_info->os_name) == 0)
      sprintf(user_info->os_name, "unknown");
  sprintf(ascii_file, PREFIX "ascii/%s.txt", user_info->os_name);
  LOG_V(ascii_file);

  file = fopen(ascii_file, "r");
  if (!file) {
    LOG_E("ascii file \"%s\" not found, falling back to current directory", ascii_file);
    sprintf(ascii_file, "./res/ascii/%s.txt", user_info->os_name);
    LOG_V(ascii_file);
    file = fopen(ascii_file, "r");
    if (!file) return 0;
  }
  char buffer[256]; // line buffer
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

// the main function is on the bottom of the file to avoid double function declarations
int main(int argc, char* argv[]) {
  struct user_config user_config_file = {0};
  struct info user_info               = {0};
  struct configuration config_flags   = parse_config(&user_info, &user_config_file);
  char* custom_distro_name            = NULL;
  char* custom_image_name             = NULL;

#ifdef _WIN32
  // packages disabled by default because chocolatey is too slow
  config_flags.pkgs = 0;
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
      user_config_file.config_directory = optarg;
      config_flags                      = parse_config(&user_info, &user_config_file);
      break;
    case 'd': // set the distribution name
      custom_distro_name = optarg;
      break;
    case 'h':
      usage(argv[0]);
      return 0;
    case 'i': // set ascii logo as output
      config_flags.image = true;
      if (argv[optind]) custom_image_name = argv[optind];
      break;
    case 'l':
      list(argv[0]);
      return 0;
    case 'r':
      user_config_file.read_enabled  = true;
      user_config_file.write_enabled = false;
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
      user_config_file.write_enabled = true;
      break;
    default:
      return 1;
    }
  }
  libfetch_init();

  if (user_config_file.read_enabled) {
    // if no cache file found write to it
    if (!read_cache(&user_info)) {
      user_config_file.read_enabled  = false;
      user_config_file.write_enabled = true;
    } else {
      LOG_I("getting additional not-cached info");
      if (config_flags.ram) {
        user_info.ram_total = get_memory_total();
        user_info.ram_used  = get_memory_used();
      }
      if (config_flags.uptime) user_info.uptime = get_uptime();
    }
  }
  if (!user_config_file.read_enabled) {
    user_info.user_name = get_user_name();
    user_info.host_name = get_host_name();
    user_info.shell     = get_shell();
#if defined(SYSTEM_BASE_ANDROID)
    if (strlen(user_info.shell) > 27) // android shell name was too long
      user_info.shell += 27;
#endif
    user_info.model         = get_model();
    user_info.kernel        = get_kernel();
    user_info.os_name       = get_os_name();
    user_info.cpu_model     = get_cpu_model();
    user_info.gpus          = get_gpus();
    user_info.packages      = get_packages();
    user_info.term_size     = get_terminal_size();
    user_info.screen_width  = get_screen_width();
    user_info.screen_height = get_screen_height();
    user_info.ram_total     = get_memory_total();
    user_info.ram_used      = get_memory_used();
    user_info.uptime        = get_uptime();
  }

  if (user_config_file.write_enabled) write_cache(&user_info);
  if (custom_distro_name) sprintf(user_info.os_name, "%s", custom_distro_name);
  if (custom_image_name) sprintf(user_info.image_name, "%s", custom_image_name);

  // print ascii or image and align cursor for print_info()
  printf("\033[%dA", config_flags.image ? print_image(&user_info) : print_ascii(&user_info));
  uwufy_all(&user_info);

  // print info and move cursor down if the number of printed lines is smaller that the default image height
  int to_move = print_info(&config_flags, &user_info);
  printf("\033[%d%c", to_move < 0 ? -to_move : to_move, to_move < 0 ? 'A' : 'B');
  libfetch_cleanup();
  if (user_config_file.read_enabled) {
    if (user_info.user_name) free(user_info.user_name);
    if (user_info.host_name) free(user_info.host_name);
    if (user_info.shell) free(user_info.shell);
    if (user_info.model) free(user_info.model);
    if (user_info.kernel) free(user_info.kernel);
    if (user_info.os_name) free(user_info.os_name);
    if (user_info.cpu_model) free(user_info.cpu_model);
    if (user_info.gpus) free(user_info.gpus);
    if (user_info.packages) free(user_info.packages);
    if (user_info.image_name) free(user_info.image_name);
  }
  LOG_I("Execution completed successfully with %d errors", logging_error_count);
  return 0;
}
