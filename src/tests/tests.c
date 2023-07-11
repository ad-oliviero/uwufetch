#include "../fetch.h"
#define LOGGING_ENABLED
#include "../logging.h"
#include <stdbool.h>
#include <stdio.h>
#include <sys/ioctl.h>

/*
 * the test consists in just comparing the output to NULL.
 * these kind of functions can not be tested with an expected
 * output as it depends on the environment. Instead I chose to
 * not generate segfaults in the program that uses the library.
 */
bool test_get_user_name() {
  return get_user_name() != NULL;
}

bool test_get_host_name() {
  return get_user_name() != NULL;
}

bool test_get_shell() {
  return get_shell() != NULL;
}

bool test_get_model() {
  return get_model() != NULL;
}

bool test_get_kernel() {
  return get_kernel() != NULL;
}

bool test_get_os_name() {
  return get_os_name() != NULL;
}

bool test_get_cpu_model() {
  return get_cpu_model() != NULL;
}

bool test_get_packages() {
  return get_packages() != NULL;
}

bool test_get_screen_width() {
  return get_screen_width() != 0;
}

bool test_get_screen_height() {
  return get_screen_height() != 0;
}

bool test_get_memory_total() {
  return get_memory_total() != 0;
}

bool test_get_memory_used() {
  return get_memory_used() > 0;
}

bool test_get_uptime() {
  return get_uptime() > 0;
}

bool test_get_terminal_size() {
  struct winsize ws = get_terminal_size();
  return ws.ws_col > 0 && ws.ws_row > 0 && ws.ws_xpixel > 0 && ws.ws_ypixel > 0;
}

struct test {
  bool (*function)();
  const char* name;
};

struct test tests[] = {
    {test_get_user_name, "get_user_name"},
    {test_get_host_name, "get_host_name"},
    {test_get_shell, "get_shell"},
    {test_get_model, "get_model"},
    {test_get_kernel, "get_kernel"},
    {test_get_os_name, "get_os_name"},
    {test_get_cpu_model, "get_cpu_model"},
    {test_get_packages, "get_packages"},
    {test_get_screen_width, "get_screen_width"},
    {test_get_screen_height, "get_screen_height"},
    {test_get_memory_total, "get_memory_total"},
    {test_get_memory_used, "get_memory_used"},
    {test_get_uptime, "get_uptime"},
    {test_get_terminal_size, "get_terminal_size"},
};

int main(void) {
  libfetch_init();
  SET_LOG_LEVEL(LEVEL_MAX, "TESTS");
  SET_LIBFETCH_LOG_LEVEL(LEVEL_MAX);
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
  LOG_I("%d tests <g>passed</>, %d <r>failed</>", passed_tests, failed_tests);
  libfetch_cleanup();
  return 0;
}
