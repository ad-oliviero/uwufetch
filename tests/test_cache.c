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

#include "../src/cache.h"
#include "../src/uwufetch.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "tests.h"

#define USER_NAME "uwu"
#define HOST_NAME "uwuhost"
#define OS_NAME "arch"
#define MODEL "B450M"
#define KERNEL "LinUwU 6.0"
#define CPU "CPUwU uwu"
#define SHELL "/bin/sh"
#define PACKAGES "42 (pacman)"
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define LOGO_ID 0x12345678u
#define GPU_NAME_LEN 32
#define GPU_NAME "gpu %zu"

static char home_dir[] = "/tmp/uwufetch_test_cache_XXXXXX";
static char cache_dir[512];

static void setup_home(void) {
  CHECK(mkdtemp(home_dir) != NULL);
  snprintf(cache_dir, sizeof(cache_dir), "%s/.cache", home_dir);
  CHECK(mkdir(cache_dir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0);
  setenv("HOME", home_dir, 1);
}

static void cleanup_home(void) {
  char path[512];
  snprintf(path, sizeof(path), "%s/uwufetch.cache", cache_dir);
  unlink(path);
  rmdir(cache_dir);
  rmdir(home_dir);
}

// builds an info struct with a known gpu list ("gpu 1", "gpu 2", ...)
static struct info make_info(size_t gpu_count) {
  struct info user_info   = {0};
  user_info.user_name     = USER_NAME;
  user_info.host_name     = HOST_NAME;
  user_info.os_name       = OS_NAME;
  user_info.model         = MODEL;
  user_info.kernel        = KERNEL;
  user_info.cpu           = CPU;
  user_info.shell         = SHELL;
  user_info.packages      = PACKAGES;
  user_info.screen_width  = SCREEN_WIDTH;
  user_info.screen_height = SCREEN_HEIGHT;
  user_info.logo_id       = LOGO_ID;
  user_info.gpu_list      = malloc(sizeof(char*) * (gpu_count + 1));
  user_info.gpu_list[0]   = (char*)gpu_count; // the [0] element is the "gpu count"
  for (size_t i = 1; i <= gpu_count; i++) {
    user_info.gpu_list[i] = malloc(GPU_NAME_LEN);
    snprintf(user_info.gpu_list[i], GPU_NAME_LEN, GPU_NAME, i);
  }
  return user_info;
}

static void free_info(struct info* user_info) {
  for (size_t i = 1; i <= (size_t)user_info->gpu_list[0]; i++) free(user_info->gpu_list[i]);
  free(user_info->gpu_list);
}

static char* cache_path(void) {
  static char path[512];
  snprintf(path, sizeof(path), "%s/uwufetch.cache", cache_dir);
  return path;
}

static void free_read_back(struct info* read_back, char* content) {
  if (content != NULL) {
    free(read_back->gpu_list);
    free(content);
  }
}

// write -> read round trip (the header size must match the file size)
static void test_round_trip(void) {
  struct info written = make_info(2);
  write_cache(&written);

  FILE* fp = fopen(cache_path(), "rb");
  CHECK(fp != NULL);
  uint32_t header_size = 0;
  CHECK(fread(&header_size, sizeof(header_size), 1, fp) == 1);
  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  fclose(fp);
  CHECK(header_size == (uint32_t)file_size); // the writer used to overcount

  struct info read_back = {0};
  char* content         = read_cache(&read_back);
  CHECK(content != NULL);
  if (content != NULL) {
    CHECK(strcmp(read_back.user_name, USER_NAME) == 0);
    CHECK(strcmp(read_back.host_name, HOST_NAME) == 0);
    CHECK(strcmp(read_back.os_name, OS_NAME) == 0);
    CHECK(strcmp(read_back.model, MODEL) == 0);
    CHECK(strcmp(read_back.kernel, KERNEL) == 0);
    CHECK(strcmp(read_back.cpu, CPU) == 0);
    CHECK(strcmp(read_back.shell, SHELL) == 0);
    CHECK(strcmp(read_back.packages, PACKAGES) == 0);
    CHECK(read_back.screen_width == SCREEN_WIDTH);
    CHECK(read_back.screen_height == SCREEN_HEIGHT);
    CHECK(read_back.logo_id == LOGO_ID);
    CHECK((size_t)read_back.gpu_list[0] == 2);
    char gpu_name[GPU_NAME_LEN];
    snprintf(gpu_name, sizeof(gpu_name), GPU_NAME, 1);
    CHECK(strcmp(read_back.gpu_list[1], gpu_name) == 0);
    snprintf(gpu_name, sizeof(gpu_name), GPU_NAME, 2);
    CHECK(strcmp(read_back.gpu_list[2], gpu_name) == 0);
  }
  free_read_back(&read_back, content);
  free_info(&written);
}

// truncating the cache file must make read_cache fail cleanly
static void test_truncation(void) {
  const size_t cuts[]   = {1, 10, 100};
  struct info user_info = make_info(2);
  write_cache(&user_info);

  for (size_t i = 0; i < sizeof(cuts) / sizeof(cuts[0]); i++) {
    struct stat st;
    CHECK(stat(cache_path(), &st) == 0);
    off_t new_size = (off_t)st.st_size > (off_t)cuts[i] ? (off_t)st.st_size - (off_t)cuts[i] : 0;
    CHECK(truncate(cache_path(), new_size) == 0);

    struct info read_back = {0};
    errno                 = 0;
    char* content         = read_cache(&read_back);
    CHECK(content == NULL);
    CHECK(errno == EIO);
    free_read_back(&read_back, content);
  }
  free_info(&user_info);
}

// multi-gpu: the gpu list allocation used to overflow for any count > 0
static void test_gpu_counts(void) {
  const size_t counts[] = {0, 1, 2, 6, 10};
  for (size_t c = 0; c < sizeof(counts) / sizeof(counts[0]); c++) {
    size_t gpu_count      = counts[c];
    struct info user_info = make_info(gpu_count);
    write_cache(&user_info);

    struct info read_back = {0};
    char* content         = read_cache(&read_back);
    CHECK(content != NULL);
    if (content != NULL) {
      CHECK((size_t)read_back.gpu_list[0] == gpu_count);
      for (size_t i = 1; i <= gpu_count; i++) {
        char expected[GPU_NAME_LEN];
        snprintf(expected, sizeof(expected), GPU_NAME, i);
        CHECK(strcmp(read_back.gpu_list[i], expected) == 0);
      }
    }
    free_read_back(&read_back, content);
    free_info(&user_info);
  }
}

int main(void) {
  setup_home();
  test_round_trip();
  test_truncation();
  test_gpu_counts();
  cleanup_home();
  if (failures == 0) fprintf(stderr, "test_cache: all tests passed\n");
  return failures != 0;
}
