#ifndef __UWUFETCH_H__
#define __UWUFETCH_H__

#ifndef UWUFETCH_VERSION
  #define UWUFETCH_VERSION "unkown" // needs to be changed by the build script
#endif

#include <stdbool.h>
#ifndef _WIN32
  #include <sys/ioctl.h>
#endif // _WIN32

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

#endif
