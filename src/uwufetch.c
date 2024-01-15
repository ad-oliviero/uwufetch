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
#include "ascii_embed.h"
#include "cache.h"
#include "common.h"
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
void show_usage(char* arg) {
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

void show_version(void) {
#ifdef UWUFETCH_VERSION
  LOG(LEVEL_INFO, "UwUfetch version: %s", UWUFETCH_VERSION);
#endif
#ifdef UWUFETCH_GIT_COMMIT
  LOG(LEVEL_INFO, "git commit: %s", UWUFETCH_GIT_COMMIT);
#endif
#ifdef UWUFETCH_GIT_BRANCH
  LOG(LEVEL_INFO, "git branch: %s", UWUFETCH_GIT_BRANCH);
#endif
#ifdef UWUFETCH_COMPILER_VERSION
  LOG(LEVEL_INFO, "compiled with: %s", UWUFETCH_COMPILER_VERSION);
#endif
}

// count the number of not visible characters
int nvccount(const unsigned char* str, const int len) {
  int count = 0;
  for (int i = 0; i < len; i++) count += str[i] > 127;
  return count + 7;
}

size_t show_info(struct info* user_info, struct configuration* configuration) {
  const struct logo_embed* logo = &(logos[user_info->logo_idx]);
  int buf_len                   = user_info->terminal_size.ws_col * 3;
  size_t printed_lines          = 0;
  char* buf                     = malloc((size_t)buf_len);

  memset(buf, 0, (size_t)buf_len);

#define PRINTLN_BUF(format, ...)                                                                                                                                                                            \
  {                                                                                                                                                                                                         \
    snprintf(buf, (size_t)buf_len, format, ##__VA_ARGS__);                                                                                                                                                  \
    printf("\x1b[%luD\x1b[%luC%.*s\n", (size_t)user_info->terminal_size.ws_col, logo->width + 1, (int)(user_info->terminal_size.ws_col - logo->width) + nvccount((const unsigned char*)buf, buf_len), buf); \
    printed_lines++;                                                                                                                                                                                        \
  }

  if (configuration->user_name) PRINTLN_BUF(BOLD "%s@%s" NORMAL, user_info->user_name, user_info->host_name);
  if (configuration->os_name) PRINTLN_BUF(BOLD "OWOS" NORMAL "     %s", user_info->os_name);
  if (configuration->model) PRINTLN_BUF(BOLD "MOWODEL" NORMAL "  %s", user_info->model);
  if (configuration->kernel) PRINTLN_BUF(BOLD "KEWNEL" NORMAL "   %s", user_info->kernel);
  if (configuration->cpu) PRINTLN_BUF(BOLD "CPUWU" NORMAL "    %s", user_info->cpu);
  if (configuration->gpu_list && user_info->gpu_list)
    for (size_t i = 1; i <= (size_t)user_info->gpu_list[0]; i++)
      PRINTLN_BUF(BOLD "GPUWU" NORMAL "    %s", user_info->gpu_list[i]);
  if (configuration->memory) PRINTLN_BUF(BOLD "MEMOWY" NORMAL "   %lu MiB/%lu MiB", user_info->memory_used, user_info->memory_total);
  if (configuration->screen && (user_info->screen_width != 0 || user_info->screen_height != 0))
    PRINTLN_BUF(BOLD "SCWEEN" NORMAL "   %dx%d", user_info->screen_width, user_info->screen_height);
  if (configuration->shell) PRINTLN_BUF(BOLD "SHEWW" NORMAL "    %s", user_info->shell);
  if (configuration->packages) PRINTLN_BUF(BOLD "PKGS" NORMAL "     %s", user_info->packages);
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
    PRINTLN_BUF(BOLD "UWUPTIME" NORMAL " %s%s%s%s", days > 0 ? str_days : "", hours > 0 ? str_hours : "", mins > 0 ? str_mins : "", secs > 0 ? str_secs : "");
  }

  // clang-format off
  if (configuration->colors) {
  #define COLOR_STRING BOLD WHITE BLOCK_CHAR BLOCK_CHAR CYAN BLOCK_CHAR \
                    BLOCK_CHAR MAGENTA BLOCK_CHAR BLOCK_CHAR BLUE \
                    BLOCK_CHAR BLOCK_CHAR YELLOW BLOCK_CHAR BLOCK_CHAR \
                    GREEN BLOCK_CHAR BLOCK_CHAR RED BLOCK_CHAR \
                    BLOCK_CHAR BLACK BLOCK_CHAR BLOCK_CHAR NORMAL
  #define COLOR_STRING_LEN 58 // strlen(COLOR_STRING) - nvccount(COLOR_STRING); it makes no sense to calculate it everytime since it is static
    printf("\x1b[%luD\x1b[%luC%.*s\n", (size_t)user_info->terminal_size.ws_col, logo->width + 1, (int)(user_info->terminal_size.ws_col- logo->width) + COLOR_STRING_LEN, COLOR_STRING);
    printed_lines++;
  }
  // clang-format on
  free(buf);
  return printed_lines;
}
void show_logo(struct info* user_info, struct configuration* configuration, size_t printed_lines) {
  if (configuration->ascii_logo) {
    const struct logo_embed* logo = &(logos[user_info->logo_idx]);
    size_t goback                 = printed_lines > logo->line_count ? printed_lines - ((printed_lines - logo->line_count) / 2) : printed_lines;
    printf("\x1b[%luA\x1b[0G", goback);
    for (size_t i = 0; i < logo->line_count; i++)
      puts((const char*)logo->lines[i].content);
    printf("\x1b[%luB", printed_lines - goback + 1);
  } else {
    LOG_E("TODO: implement image logo");
  }
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
      {"full", no_argument, NULL, 'f'},
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
  #define OPT_STRING "c:d:fhi::lrVv::w"
#else
  #define OPT_STRING "c:d:fhi::lrVw"
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
    case 'f':
      memset(&configuration, 1, sizeof(struct configuration));
      break;
    case 'h':
      show_usage(argv[0]);
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
      show_version();
      return 0;
#if defined(LOGGING_ENABLED)
    case 'v':
      if (argv[optind]) {
        SET_LOG_LEVEL(atoi(argv[optind]), "uwufetch");
        char* sep = strchr(argv[optind], ',');
        if (sep) *(sep++) = '\0';
        SET_LIBFETCH_LOG_LEVEL(atoi(sep ? sep : argv[optind]));
      }
      show_version();
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
    if (configuration.shell)
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

  // before we "uwufy" the os name, we need to calculate the jenkins hash of it
  if (user_info.os_name) {
    user_info.logo_id = str2id(user_info.os_name, (int)strlen(user_info.os_name));
    for (size_t i = 0; i < logos_count; i++) {
      user_info.logo_idx = i;
      if (user_info.logo_id == logos[i].id) {
        user_info.logo_idx = i;
        break;
      }
    }
  }
  if (!cache.read) uwufy_all(&user_info);
  if (cache.write) write_cache(&user_info);

  size_t printed_lines = show_info(&user_info, &configuration);
  show_logo(&user_info, &configuration, printed_lines);

  libfetch_cleanup();
  if (cache.read) {
    if (cache.content) free(cache.content);
    if (user_info.gpu_list) free(user_info.gpu_list);
  }
  LOG_I("Execution completed successfully with %d errors", logging_error_count);
  return 0;
}
