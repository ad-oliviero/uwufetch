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

/* NOTE: unit tests for the config module */

#include "../src/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures;
#define CHECK(cond)                                                   \
  do {                                                                \
    if (!(cond)) {                                                    \
      fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
      failures++;                                                     \
    }                                                                 \
  } while (0)

static char tmp_dir[] = "/tmp/uwufetch_test_config_XXXXXX";

static void write_config(const char* content) {
  char path[512];
  snprintf(path, sizeof(path), "%s/config", tmp_dir);
  FILE* fp = fopen(path, "w");
  CHECK(fp != NULL);
  if (fp != NULL) {
    fputs(content, fp);
    fclose(fp);
  }
}

static struct configuration parse(const char* content) {
  struct configuration configuration = {0};
  write_config(content);
  char path[512];
  snprintf(path, sizeof(path), "%s/config", tmp_dir);
  parse_config(&configuration, path);
  return configuration;
}

// the logo name must be NUL terminated with and without a trailing newline
static void test_logo(void) {
  struct configuration c = parse("logo=arch\n");
  CHECK(c.logo_name != NULL && strcmp(c.logo_name, "arch") == 0);
  free(c.logo_name);

  c = parse("logo=arch"); // no trailing newline: the last char used to be cut off
  CHECK(c.logo_name != NULL && strcmp(c.logo_name, "arch") == 0);
  free(c.logo_name);

  c = parse("logo=debian\r\n");
  CHECK(c.logo_name != NULL && strcmp(c.logo_name, "debian") == 0);
  free(c.logo_name);

  c = parse("logo=\n"); // empty value
  CHECK(c.logo_name != NULL && strcmp(c.logo_name, "") == 0);
  free(c.logo_name);

  c = parse("user_name=true\n"); // a "logo" substring is not a logo line
  CHECK(c.logo_name == NULL);
}

// quoted and unquoted boolean values
static void test_booleans(void) {
  struct configuration c = parse("user_name=true\nos_name=false\n");
  CHECK(c.user_name == true);
  CHECK(c.os_name == false);

  // quoted values used to be silently ignored
  c = parse("user_name=\"true\"\nos_name=\"false\"\nmodel=\"false\"\nkernel=\"true\"\n");
  CHECK(c.user_name == true);
  CHECK(c.os_name == false);
  CHECK(c.model == false);
  CHECK(c.kernel == true);

  c = parse("cpu=false\nmemory=true\nscreen=false\nshell=true\npackages=false\nuptime=true\ncolors=false\ngpu_list=false\n");
  CHECK(c.cpu == false);
  CHECK(c.memory == true);
  CHECK(c.screen == false);
  CHECK(c.shell == true);
  CHECK(c.packages == false);
  CHECK(c.uptime == true);
  CHECK(c.colors == false);
  CHECK(c.gpu_list == false);
}

// malformed and oversized lines must not crash
static void test_malformed(void) {
  char oversized[512];
  memset(oversized, 'x', sizeof(oversized) - 2);
  oversized[sizeof(oversized) - 2] = '\n';
  oversized[sizeof(oversized) - 1] = '\0';

  char content[2048];
  snprintf(content, sizeof(content), "\n\n   \n%s\n=garbage=\nlogo=ubuntu\n", oversized);
  struct configuration c = parse(content);
  CHECK(c.logo_name != NULL && strcmp(c.logo_name, "ubuntu") == 0);
  CHECK(c.user_name == false); // untouched: the defaults are preserved
  free(c.logo_name);

  c = parse("this is not a config line at all\n\xff\xfe binary garbage\n");
  CHECK(c.logo_name == NULL);
  CHECK(c.user_name == false);
}

// a NULL config path with HOME unset must not crash (in debug builds it
// falls back to ./default.config)
static void test_no_home(void) {
  unsetenv("HOME");
  struct configuration c = {0};
  parse_config(&c, NULL);
  free(c.logo_name); // may have been set by ./default.config
}

int main(void) {
  CHECK(mkdtemp(tmp_dir) != NULL);
  test_logo();
  test_booleans();
  test_malformed();
  test_no_home();
  rmdir(tmp_dir);
  if (failures == 0) fprintf(stderr, "test_config: all tests passed\n");
  return failures != 0;
}
