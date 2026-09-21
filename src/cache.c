#include "libfetch/logging.h"
#include "uwufetch.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
  // writing most of the values to config file (they may be NULL when the
  // corresponding option is disabled in the config)
  uint32_t cache_size = (uint32_t)fprintf(
      cache_fp,
      "0000"                      // placeholder for the cache size
      "%s;%s;%s;%s;%s;%s;%s;%s;", // no need to be human readable
      user_info->user_name ? user_info->user_name : "",
      user_info->host_name ? user_info->host_name : "",
      user_info->os_name ? user_info->os_name : "",
      user_info->model ? user_info->model : "",
      user_info->kernel ? user_info->kernel : "",
      user_info->cpu ? user_info->cpu : "",
      user_info->shell ? user_info->shell : "",
      user_info->packages ? user_info->packages : "");

  // writing numbers before gpus (because gpus are a variable amount)
  cache_size += (uint32_t)fwrite(&user_info->screen_width, sizeof(char), sizeof(user_info->screen_width), cache_fp);
  cache_size += (uint32_t)fwrite(&user_info->screen_height, sizeof(char), sizeof(user_info->screen_height), cache_fp);
  cache_size += (uint32_t)fwrite(&user_info->logo_id, sizeof(char), sizeof(user_info->logo_id), cache_fp);

  // the first element of gpu_list is the number of gpus (written as the 8
  // byte value it is stored as; gpu_list may be NULL when the option is
  // disabled in the config)
  char* gpu_count = user_info->gpu_list ? user_info->gpu_list[0] : NULL;
  cache_size += (uint32_t)fwrite(&gpu_count, sizeof(char), sizeof(gpu_count), cache_fp);

  for (size_t i = 1; i <= (size_t)gpu_count; i++) // writing gpu names to file
    cache_size += (uint32_t)fprintf(cache_fp, ";%s", user_info->gpu_list[i]);
  cache_size += (uint32_t)fprintf(cache_fp, ";"); // the last gpu name must be terminated

  // writing cache size at the beginning of the file
  fseek(cache_fp, 0, SEEK_SET);
  fwrite(&cache_size, sizeof(char), sizeof(cache_size), cache_fp);

  fclose(cache_fp);
  return;
}

// splits the next ';' separated field, sets errno and returns false if the
// cache is malformed
static bool next_field(char** field, char* buffer_end) {
  char* separator = memchr(*field, ';', (size_t)(buffer_end - *field));
  if (separator == NULL) {
    errno = EINVAL;
    return false;
  }
  *separator = '\0';
  *field     = separator + 1;
  return true;
}

// parses the cache content into user_info, sets errno and returns false if it
// is malformed
static bool parse_cache(struct info* user_info, char* buffer, char* buffer_end) {
  user_info->gpu_list = NULL;

  // reading strings
  user_info->user_name = buffer;
  if (!next_field(&buffer, buffer_end)) return false;
  user_info->host_name = buffer;
  if (!next_field(&buffer, buffer_end)) return false;
  user_info->os_name = buffer;
  if (!next_field(&buffer, buffer_end)) return false;
  user_info->model = buffer;
  if (!next_field(&buffer, buffer_end)) return false;
  user_info->kernel = buffer;
  if (!next_field(&buffer, buffer_end)) return false;
  user_info->cpu = buffer;
  if (!next_field(&buffer, buffer_end)) return false;
  user_info->shell = buffer;
  if (!next_field(&buffer, buffer_end)) return false;
  user_info->packages = buffer;
  if (!next_field(&buffer, buffer_end)) return false;

  // reading numbers (at fixed offsets: binary data may contain ';' bytes)
  if (buffer_end - buffer <
      (ptrdiff_t)(sizeof(user_info->screen_width) + sizeof(user_info->screen_height) +
                  sizeof(user_info->logo_id) + sizeof(char*))) {
    errno = EINVAL;
    return false;
  }
  memcpy(&user_info->screen_width, buffer, sizeof(user_info->screen_width));
  buffer += sizeof(user_info->screen_width);
  memcpy(&user_info->screen_height, buffer, sizeof(user_info->screen_height));
  buffer += sizeof(user_info->screen_height);
  memcpy(&user_info->logo_id, buffer, sizeof(user_info->logo_id));
  buffer += sizeof(user_info->logo_id);

  // reading gpus: the first element of gpu_list is the number of gpus
  char* gpu_count = NULL;
  memcpy(&gpu_count, buffer, sizeof(gpu_count));
  buffer += sizeof(gpu_count);
  if (!next_field(&buffer, buffer_end)) return false;      // the ';' before the first gpu name
  if ((size_t)gpu_count > (size_t)(buffer_end - buffer)) { // each name needs at least one byte
    errno = EINVAL;
    return false;
  }
  user_info->gpu_list = malloc(sizeof(char*) * ((size_t)gpu_count + 1));
  if (user_info->gpu_list == NULL) {
    errno = ENOMEM;
    return false;
  }
  user_info->gpu_list[0] = gpu_count;
  memset(user_info->gpu_list + 1, 0, (size_t)gpu_count * sizeof(char*));
  for (size_t i = 1; i <= (size_t)gpu_count; i++) {
    user_info->gpu_list[i] = buffer;
    if (!next_field(&buffer, buffer_end)) return false;
  }
  return true;
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
    return NULL; // errno is already set by fopen
  }
  uint32_t cache_size = 0;
  if (fread(&cache_size, sizeof(cache_size), 1, cache_fp) != 1) {
    LOG_E("Failed to read the cache size from %s!", cache_fn);
    fclose(cache_fp);
    errno = EIO;
    return NULL;
  }
  if (cache_size < sizeof(cache_size)) { // the size includes the header itself
    fclose(cache_fp);
    errno = EINVAL;
    return NULL;
  }
  uint32_t body_size = cache_size - (uint32_t)sizeof(cache_size);
  char* start        = malloc(body_size > 0 ? body_size : 1);
  if (start == NULL) {
    fclose(cache_fp);
    errno = ENOMEM;
    return NULL;
  }
  if (fread(start, sizeof(char), body_size, cache_fp) != body_size) {
    LOG_E("Cache file %s is truncated!", cache_fn);
    free(start);
    fclose(cache_fp);
    errno = EIO;
    return NULL;
  }
  fclose(cache_fp);

  if (!parse_cache(user_info, start, start + body_size)) {
    LOG_E("Failed to parse %s!", cache_fn);
    free(start);
    free(user_info->gpu_list);
    return NULL;
  }

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
  LOG_V(user_info->logo_id);
#ifdef LOGGING_ENABLED
  for (size_t i = 1; i <= (size_t)user_info->gpu_list[0]; i++)
    LOG_V(user_info->gpu_list[i]);
#endif
  return start;
}
