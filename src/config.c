#include "config.h"
#include "libfetch/logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// reads the config file
void parse_config(struct configuration* configuration, char* config_path) {
  char buffer[256]; // buffer for the current line
  FILE* config = NULL;
#if defined(SYSTEM_BASE_WINDOWS)
  configuration->packages = false; // chocolatey is too slow
#endif

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
  if (config == NULL) return; // if config file does not exist, return the defaults

  // reading the config file
  while (fgets(buffer, sizeof(buffer), config)) {
    if (strstr(buffer, "logo")) {
      size_t len               = strlen(buffer) - sizeof("logo");
      configuration->logo_name = malloc(len);
      memcpy(configuration->logo_name, buffer + sizeof("logo"), len - 1);
      LOG_V(configuration->logo_name);
    }
#define FIND_CFG_VAR(name)                         \
  if (sscanf(buffer, #name "="                     \
                           "%[truefalse]",         \
             buffer)) {                            \
    configuration->name = strcmp(buffer, "false"); \
    LOG_V(configuration->name);                    \
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
  return;
}
