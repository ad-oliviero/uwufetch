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

#include "uwufetch.h"
#include "actrie.h"
#include "cache.h"
#include "libfetch/fetch.h"
#include "libfetch/logging.h"
#include "translation.h"
#ifdef __APPLE__
  #include <TargetConditionals.h> // for checking iOS
#endif
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
  #include <sys/utsname.h>
#endif // _WIN32

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// reads the config file
struct configuration parse_config(char* config_path) {
  char buffer[256]; // buffer for the current line
  FILE* config = NULL;
  // enabling all flags by default
  struct configuration configuration;
  memset(&configuration, true, sizeof(configuration));

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
    // if (sscanf(buffer, "logo=%s", configuration.logo_path) > 0) LOG_V(configuration.logo_path);
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
  return configuration;
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
  user_info.terminal_size = get_terminal_size();
#undef IF_ENABLED_GET

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
  LOG_I("Execution completed successfully with %d errors", logging_error_count);
  return 0;
}
