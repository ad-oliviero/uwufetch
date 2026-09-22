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

#include "../src/libfetch/fetch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include "tests.h"

#define TEST_USER "uwutester"
#define TEST_HOST "testhost"
#define TEST_SHELL "/bin/fish"
#define MAX_STRING_LEN 1024
#define MAX_GPU_COUNT 64

// every char* getter must return NULL or a non-empty NUL-terminated string
static void check_string(char* s) {
  if (s == NULL) return;
  size_t len = strlen(s);
  CHECK(len > 0);
  CHECK(len < MAX_STRING_LEN);
}

static void run_cycle(void) {
  libfetch_init();

  char* user     = get_user_name();
  char* host     = get_host_name();
  char* shell    = get_shell();
  char* model    = get_model();
  char* kernel   = get_kernel();
  char* os_name  = get_os_name();
  char* cpu      = get_cpu();
  char* packages = get_packages();
  check_string(user);
  check_string(host);
  check_string(shell);
  check_string(model);
  check_string(kernel);
  check_string(os_name);
  check_string(cpu);
  check_string(packages);
#if defined(SYSTEM_BASE_LINUX)
  // the env-honoring paths are getenv-first on Linux (HOST is not: nodename wins)
  CHECK(user != NULL && strcmp(user, TEST_USER) == 0);
  CHECK(shell != NULL && strcmp(shell, TEST_SHELL) == 0);
#endif

  CHECK(get_screen_width() >= 0);
  CHECK(get_screen_height() >= 0);
  unsigned long long memory_total = get_memory_total();
  unsigned long long memory_used  = get_memory_used();
  CHECK(get_uptime() >= 0);
  if (memory_total > 0) CHECK(memory_used <= memory_total);

  // make runs the tests with stdout on a pipe, so the ioctl must fail
  struct winsize terminal_size = get_terminal_size();
  CHECK(terminal_size.ws_col == 0);
  CHECK(terminal_size.ws_row == 0);

  char** gpu_list = get_gpu_list();
  if (gpu_list != NULL) CHECK((size_t)gpu_list[0] < MAX_GPU_COUNT);

  libfetch_cleanup();
}

int main(void) {
  setenv("USER", TEST_USER, 1);
  setenv("HOST", TEST_HOST, 1);
  setenv("SHELL", TEST_SHELL, 1);

  // without init the fb0 getters must degrade to 0, not crash (the fb0 bug class)
  CHECK(get_screen_width() == 0);
  CHECK(get_screen_height() == 0);

  // pointer registry must not exhaust across calls
  run_cycle();
  run_cycle();
  if (failures == 0) printf("test_fetch: all tests passed\n");
  return failures != 0;
}
