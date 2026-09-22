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

/* NOTE: CLI integration tests for uwufetch. The path to the binary (built by
 * make test) must be passed as the first argument, only the structure of the
 * output is asserted.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
  #include <direct.h>
  #define mkdir(path, mode) _mkdir(path)
#else
  #include <sys/wait.h>
#endif

#define OUT_FILE "build/run_out.txt"
#define ERR_FILE "build/run_err.txt"
#define HOME_DIR "build/run_home"
#define OUTPUT_CAP 65536

// values pinned in the environment for every run (also asserted in case 8)
#define PINNED_USER "uwutester"
#define PINNED_HOST "uwuhost"
#define PINNED_SHELL "/bin/sh"

static char output[OUTPUT_CAP];   // stdout of the last run
static char stripped[OUTPUT_CAP]; // output without the ansi escape sequences
static int failures;

static void check(const char* description, bool condition) {
  printf("  %s %s\n", condition ? "OK  " : "FAIL", description);
  if (!condition) failures++;
}

static void set_env(const char* name, const char* value) {
#ifdef _WIN32
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "%s=%s", name, value);
  _putenv(buffer);
#else
  setenv(name, value, 1);
#endif
}

// runs the binary and returns its exit code, the stdout is saved in "output"
static int run(const char* binary, const char* args) {
  char command[512];
  snprintf(command, sizeof(command), "%s %s > " OUT_FILE " 2> " ERR_FILE, binary, args);
  int status = system(command);
#ifdef _WIN32
  return status;
#else
  return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
}

static void strip_ansi(const char* in, char* out) {
  while (*in) {
    if (*in == '\x1b' && in[1] == '[') {
      in += 2;
      while (*in && !(*in >= 'A' && *in <= 'Z') && !(*in >= 'a' && *in <= 'z')) in++;
      if (*in) in++;
    } else
      *out++ = *in++;
  }
  *out = '\0';
}

static void read_output(void) {
  output[0]   = '\0';
  stripped[0] = '\0';
  FILE* fp    = fopen(OUT_FILE, "rb");
  if (fp == NULL) return;
  size_t len  = fread(output, sizeof(char), OUTPUT_CAP - 1, fp);
  output[len] = '\0';
  fclose(fp);
  strip_ansi(output, stripped);
}

// value printed after a label (i.e. "OWOS" -> "Nyarch LinUwU"), returns false
// if there is no line starting with the label
static bool field(const char* text, const char* label, char* value, size_t value_len) {
  size_t label_len = strlen(label);
  const char* line = text;
  while (line != NULL && *line != '\0') {
    const char* line_end = strchr(line, '\n');
    size_t line_len      = line_end != NULL ? (size_t)(line_end - line) : strlen(line);
    if (line_len >= label_len && strncmp(line, label, label_len) == 0) {
      const char* start = line + label_len;
      while (start < line + line_len && *start == ' ') start++;
      snprintf(value, value_len, "%.*s", (int)(line + line_len - start), start);
      return true;
    }
    line = line_end != NULL ? line_end + 1 : NULL;
  }
  return false;
}

// every info line starts with a cursor movement escape ("\x1b[<digits>D")
static int art_lines(void) {
  int count        = 0;
  const char* line = output;
  while (*line) {
    const char* p  = line;
    bool info_line = *p++ == '\x1b' && *p++ == '[';
    if (info_line) {
      while (*p >= '0' && *p <= '9') p++;
      info_line = *p == 'D';
    }
    if (!info_line) count++;
    const char* line_end = strchr(line, '\n');
    if (line_end == NULL) break;
    line = line_end + 1;
  }
  return count;
}

static void check_not_empty(const char* label) {
  char description[64], value[256];
  bool found = field(stripped, label, value, sizeof(value));
  snprintf(description, sizeof(description), "%s is not empty", label);
  check(description, found && value[0] != '\0');
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: run <path to the uwufetch binary>\n");
    return 1;
  }
  const char* binary = argv[1];
  char value[256], first[OUTPUT_CAP];
  int status;

  set_env("HOME", HOME_DIR);
  set_env("USER", PINNED_USER);
  set_env("HOST", PINNED_HOST);
  set_env("SHELL", PINNED_SHELL);
  mkdir(HOME_DIR, 0755);
  mkdir(HOME_DIR "/.cache", 0755);

  printf("case 1: default config\n");
  status = run(binary, "");
  read_output();
  check("exit code is 0", status == 0);
  check("stdout is not empty", output[0] != '\0');
  check("no (null) in the output", strstr(output, "(null)") == NULL);

  printf("case 2: no empty enabled text fields\n");
  // MOWODEL, GPUWU and SCWEEN depend on the system (dmi, pci devices,
  // framebuffer): they are checked only when present
  static const char* const optional[] = {"MOWODEL", "GPUWU", "SCWEEN"};
  for (size_t i = 0; i < sizeof(optional) / sizeof(optional[0]); i++) {
    if (field(stripped, optional[i], value, sizeof(value)))
      check_not_empty(optional[i]);
    else
      printf("  SKIP %s (not available on this system)\n", optional[i]);
  }
  static const char* const required[] = {"OWOS", "KEWNEL", "CPUWU", "MEMOWY", "SHEWW", "PKGS", "UWUPTIME"};
  for (size_t i = 0; i < sizeof(required) / sizeof(required[0]); i++)
    check_not_empty(required[i]);

  printf("case 3: -l debian prints the debian art\n");
  status = run(binary, "-l debian");
  read_output();
  check("exit code is 0", status == 0);
  check("debian art marker", strstr(output, "OωO") != NULL);

  printf("case 4: -l nosuch still prints art (logo fallback)\n");
  status = run(binary, "-l nosuch");
  read_output();
  check("exit code is 0", status == 0);
  check("art is still printed", art_lines() > 0);

  printf("case 5: -c reproduces the cached fields\n");
  run(binary, "");
  read_output();
  memcpy(first, stripped, strlen(stripped) + 1);
  run(binary, "-c");
  read_output();
  static const char* const cached[] = {"OWOS", "CPUWU", "KEWNEL"};
  for (size_t i = 0; i < sizeof(cached) / sizeof(cached[0]); i++) {
    char a[256], b[256], description[64];
    bool found_a = field(first, cached[i], a, sizeof(a));
    bool found_b = field(stripped, cached[i], b, sizeof(b));
    snprintf(description, sizeof(description), "%s matches the cached value", cached[i]);
    check(description, found_a && found_b && a[0] != '\0' && strcmp(a, b) == 0);
  }

  printf("case 6: -v prints the version\n");
  status = run(binary, "-v");
  read_output();
  check("exit code is 0", status == 0);
  check("version line", strstr(output, "UwUfetch version") != NULL);

  printf("case 7: config with everything disabled\n");
  status = run(binary, "-C tests/fixtures/off.conf");
  read_output();
  check("exit code is 0", status == 0);
  bool no_labels = true;
  for (size_t i = 0; i < sizeof(required) / sizeof(required[0]); i++)
    if (field(stripped, required[i], value, sizeof(value))) no_labels = false;
  check("no info labels are printed", no_labels);

  printf("case 8: environment-pinned field values\n");
  status = run(binary, "");
  read_output();
  check("exit code is 0", status == 0);
  // the user line is "user@host" and the host comes from uname's nodename
  // (not from $HOST), so only the user is matched exactly
  char host[256];
  bool user_found = field(stripped, PINNED_USER "@", host, sizeof(host));
  check("user field is the pinned user", user_found);
  check("host field is not empty", user_found && host[0] != '\0');
  bool shell_found = field(stripped, "SHEWW", value, sizeof(value));
  check("shell field is the pinned shell", shell_found && strcmp(value, PINNED_SHELL) == 0);
  unsigned long mem_used = 0, mem_total = 0;
  bool mem_found  = field(stripped, "MEMOWY", value, sizeof(value));
  bool mem_parsed = mem_found && sscanf(value, "%lu MiB/%lu MiB", &mem_used, &mem_total) == 2;
  check("memory used is at most total", mem_parsed && mem_used <= mem_total);
  unsigned long pkg_count = 0;
  bool pkgs_found         = field(stripped, "PKGS", value, sizeof(value));
  check("packages field starts with \"<digits>: \"", pkgs_found && sscanf(value, "%lu: ", &pkg_count) == 1);

  if (failures == 0) {
    printf("run: all tests passed\n");
    return 0;
  }
  printf("run: %d checks failed\n", failures);
  return 1;
}
