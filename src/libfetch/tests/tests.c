/* NOTE:
 * This test "suite" is made only for the backend of uwufetch (libfetch).
 * The frontend (everything outside src/libfetch) will not be tested!
 * Maybe future versions of this project will have tests for frontend functions,
 * but the new tests will be in a different file.
 */

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

#ifndef __DEBUG__
  #error "The test suite must be compiled in __DEBUG__ mode!" // we need the logging system
#endif
#include "../fetch.h"
#define LOGGING_ENABLED
#include "../logging.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#if !defined(SYSTEM_BASE_WINDOWS)
  #include <sys/ioctl.h>
#endif

#ifndef UWUFETCH_VERSION
  #define UWUFETCH_VERSION "unkown" // needs to be changed by the build script
#endif

/*
 * the test consists in just comparing the output to NULL.
 * these kind of functions can not be tested with an expected
 * output as it depends on the environment. Instead I chose to
 * not generate segfaults in the program that uses the library.
 */

#define TEST_FUNCTION(f) bool test_##f(void)
#define TEST_COMPARE_FUNCTION(f, c) \
  TEST_FUNCTION(f) { return f() c; }
#define TEST_STRING_FUNCTION(f) TEST_COMPARE_FUNCTION(f, != NULL)

TEST_STRING_FUNCTION(get_user_name)
TEST_STRING_FUNCTION(get_host_name)
TEST_STRING_FUNCTION(get_shell)
TEST_STRING_FUNCTION(get_model)
TEST_STRING_FUNCTION(get_kernel)
TEST_STRING_FUNCTION(get_os_name)
TEST_STRING_FUNCTION(get_cpu_model)
TEST_STRING_FUNCTION(get_gpus)
TEST_STRING_FUNCTION(get_packages)

TEST_COMPARE_FUNCTION(get_screen_width, > 0)
TEST_COMPARE_FUNCTION(get_screen_height, > 0)
TEST_COMPARE_FUNCTION(get_memory_total, > 0)
TEST_COMPARE_FUNCTION(get_memory_used, > 0)
TEST_COMPARE_FUNCTION(get_uptime, > 0)

TEST_FUNCTION(get_terminal_size) {
  struct winsize ws = get_terminal_size();
  return ws.ws_col > 0 && ws.ws_row > 0 && ws.ws_xpixel > 0 && ws.ws_ypixel > 0;
}

#define STRUCT_TEST(f) \
  { test_##f, #f }
struct test {
  bool (*function)(void);
  const char* name;
};

struct test tests[] = {
    STRUCT_TEST(get_user_name),
    STRUCT_TEST(get_host_name),
    STRUCT_TEST(get_shell),
    STRUCT_TEST(get_model),
    STRUCT_TEST(get_kernel),
    STRUCT_TEST(get_os_name),
    STRUCT_TEST(get_cpu_model),
    STRUCT_TEST(get_gpus),
    STRUCT_TEST(get_packages),
    STRUCT_TEST(get_screen_width),
    STRUCT_TEST(get_screen_height),
    STRUCT_TEST(get_memory_total),
    STRUCT_TEST(get_memory_used),
    STRUCT_TEST(get_uptime),
    STRUCT_TEST(get_terminal_size),
};

int main(int argc, char** argv) {
  SET_LOG_LEVEL(LEVEL_MAX, "TESTS");
  if (argc > 1) {
    if (strcmp(argv[1], "-v") == 0) {
      SET_LIBFETCH_LOG_LEVEL(LEVEL_MAX);
    } else if (strcmp(argv[1], "-h") == 0) {
      LOG_I("libfetch testing version %s. To enable verbose libfetch logging use -v", UWUFETCH_VERSION);
      return 0;
    }
  }
  libfetch_init();
  LOG_W("Starting tests...");
  int passed_tests = 0;
  int failed_tests = 0;
  // tests will be ran in a random order to make sure they do not depend on each other
#define TEST_COUNT (sizeof(tests) / sizeof(struct test))
  size_t test_order[TEST_COUNT] = {0};
  srand((unsigned int)time(NULL));
  for (size_t i = 0; i < TEST_COUNT;) {
    size_t new_id = (size_t)(rand() % (int)TEST_COUNT);
    bool exists   = false;
    for (size_t j = 0; j < i; j++) {
      if (test_order[j] == new_id) {
        exists = true;
        break;
      }
    }
    if (!exists) test_order[i++] = new_id;
  }
#undef TEST_COUNT
  for (size_t i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
    size_t id = test_order[i];
    if (tests[id].function()) {
      LOG_I("%s <g>PASSED</>", tests[id].name);
      passed_tests++;
    } else {
      LOG_E("%s <r>FAILED</>", tests[id].name);
      failed_tests++;
    }
  }
  LOG_I("\n%d tests <g>passed</>, %d <r>failed</>", passed_tests, failed_tests);
  libfetch_cleanup();
  return 0;
}
