#include "../fetch.h"
#define LOGGING_ENABLED
#include "../logging.h"
#include <stdbool.h>
#include <stdio.h>
#include <sys/ioctl.h>

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

TEST_COMPARE_FUNCTION(get_screen_width, == 0)
TEST_COMPARE_FUNCTION(get_screen_height, == 0)
TEST_COMPARE_FUNCTION(get_memory_total, == 0)
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
      LOG_I("uwufetch testing version %s. To enable verbose libfetch logging use -v", UWUFETCH_VERSION);
      return 0;
    }
  }
  libfetch_init();
  LOG_W("Starting tests...");
  int passed_tests = 0;
  int failed_tests = 0;
  for (size_t i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
    if (tests[i].function()) {
      LOG_I("%s <g>PASSED</>", tests[i].name);
      passed_tests++;
    } else {
      LOG_E("%s <r>FAILED</>", tests[i].name);
      failed_tests++;
    }
  }
  LOG_I("\n%d tests <g>passed</>, %d <r>failed</>", passed_tests, failed_tests);
  libfetch_cleanup();
  return 0;
}
