#include "libfetch/logging.h"
#include "uwufetch.h"
#include <stdlib.h>
#include <stdint.h>

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
