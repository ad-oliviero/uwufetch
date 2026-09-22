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
#include <unistd.h>

#include "tests.h"

#define BUF_SIZE 256
#define CANARY "CANARY"
#define CANARY_SIZE 8

// meminfo values in kB
#define MEMINFO_TOTAL 16384000ul
#define MEMINFO_FREE 4200000ul
#define MEMINFO_BUFFERS 640000ul
#define MEMINFO_CACHED 4600000ul
#define MEMINFO_FIXTURE           \
  "MemTotal:       16384000 kB\n" \
  "MemFree:         4200000 kB\n" \
  "MemAvailable:    9800000 kB\n" \
  "Buffers:          640000 kB\n" \
  "Cached:          4600000 kB\n"
#define MEMINFO_MISSING_TOTAL_FIXTURE "MemFree: 4200000 kB\nBuffers: 640000 kB\n"

#define CPU_MODEL "Fixture CPU Model @ 2.9GHz"
#define CPUINFO_FIXTURE       \
  "processor\t: 0\n"          \
  "vendor_id\t: GenuineUwU\n" \
  "model name\t: " CPU_MODEL "\n"
#define CPUINFO_MULTI_FIXTURE \
  "model name\t: Old Model\n" \
  "model name\t: " CPU_MODEL "\n"
#define CPUINFO_NO_MODEL_FIXTURE \
  "processor\t: 0\n"             \
  "vendor_id\t: GenuineUwU\n"

#define OS_ID_UNQUOTED "debian"
#define OS_RELEASE_UNQUOTED_FIXTURE \
  "NAME=\"Debian GNU/Linux\"\n"     \
  "ID=" OS_ID_UNQUOTED "\n"         \
  "HOME_URL=\"https://www.debian.org/\"\n"
// TODO: quoted ids keep a trailing quote and truncate at the first space,
// the fix lands in a later commit
#define OS_ID_QUOTED_NO_SPACE "arch\"" // current: trailing quote kept
#define OS_RELEASE_QUOTED_NO_SPACE_FIXTURE "ID=\"arch\"\n"
#define OS_ID_QUOTED_SPACES "manjaro" // current: truncated at the space
#define OS_RELEASE_QUOTED_SPACES_FIXTURE "ID=\"manjaro linux\"\n"
#define OS_RELEASE_NO_ID_FIXTURE \
  "NAME=\"UwU\"\n"               \
  "ID_LIKE=arch\n"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define SCREEN_FB0_FIXTURE "1920,1080\n"
#define DMESG_WIDTH 1024
#define DMESG_HEIGHT 768
#define DMESG_FIXTURE                \
  "Linux version 6.1.0 (uwu@uwu)\n"  \
  "VT(efifb): resolution 1024x768\n" \
  "random other dmesg line\n"

#define KERNEL_SYSNAME "Linux"
#define KERNEL_RELEASE "6.1.0"
#define KERNEL_MACHINE "x86_64"
#define KERNEL_FULL KERNEL_SYSNAME " " KERNEL_RELEASE " " KERNEL_MACHINE
#define KERNEL_NO_RELEASE KERNEL_SYSNAME " " KERNEL_MACHINE
#define KERNEL_NO_MACHINE KERNEL_SYSNAME " " KERNEL_RELEASE " "

static char tmp_dir[] = "/tmp/uwufetch_test_fetch_XXXXXX";
static char file_path[512];

static void write_file(const char* name, const char* content) {
  snprintf(file_path, sizeof(file_path), "%s/%s", tmp_dir, name);
  FILE* fp = fopen(file_path, "w");
  CHECK(fp != NULL);
  if (fp) {
    CHECK(fputs(content, fp) >= 0);
    fclose(fp);
  }
}

static void check_canary(const char canary[CANARY_SIZE]) {
  CHECK(strcmp(canary, CANARY) == 0);
}

static void test_parse_meminfo(void) {
  unsigned long meminfo[4];
  parse_meminfo(MEMINFO_FIXTURE, meminfo);
  CHECK(meminfo[0] == MEMINFO_TOTAL);
  CHECK(meminfo[1] == MEMINFO_FREE);
  CHECK(meminfo[2] == MEMINFO_BUFFERS);
  CHECK(meminfo[3] == MEMINFO_CACHED);

  parse_meminfo(MEMINFO_MISSING_TOTAL_FIXTURE, meminfo);
  CHECK(meminfo[0] == 0);
  CHECK(meminfo[1] == MEMINFO_FREE);
  CHECK(meminfo[2] == MEMINFO_BUFFERS);

  parse_meminfo("", meminfo);
  CHECK(meminfo[0] == 0 && meminfo[1] == 0 && meminfo[2] == 0 && meminfo[3] == 0);

  parse_meminfo(NULL, meminfo);
  CHECK(meminfo[0] == 0 && meminfo[1] == 0 && meminfo[2] == 0 && meminfo[3] == 0);
}

static void test_parse_cpu_model(void) {
  char out[BUF_SIZE];
  char canary[CANARY_SIZE] = CANARY;

  CHECK(parse_cpu_model(CPUINFO_FIXTURE, out, sizeof(out)) == true);
  CHECK(strcmp(out, CPU_MODEL) == 0);

  // the last "model name" line wins (one line per core, all identical in practice)
  CHECK(parse_cpu_model(CPUINFO_MULTI_FIXTURE, out, sizeof(out)) == true);
  CHECK(strcmp(out, CPU_MODEL) == 0);

  CHECK(parse_cpu_model(CPUINFO_NO_MODEL_FIXTURE, out, sizeof(out)) == false);
  CHECK(out[0] == '\0');

  CHECK(parse_cpu_model(NULL, out, sizeof(out)) == false);
  CHECK(out[0] == '\0');
  check_canary(canary);
}

static void test_parse_os_id(void) {
  char out[BUF_SIZE];

  CHECK(parse_os_id(OS_RELEASE_UNQUOTED_FIXTURE, out, sizeof(out)) == true);
  CHECK(strcmp(out, OS_ID_UNQUOTED) == 0);

  CHECK(parse_os_id(OS_RELEASE_QUOTED_NO_SPACE_FIXTURE, out, sizeof(out)) == true);
  CHECK(strcmp(out, OS_ID_QUOTED_NO_SPACE) == 0);

  CHECK(parse_os_id(OS_RELEASE_QUOTED_SPACES_FIXTURE, out, sizeof(out)) == true);
  CHECK(strcmp(out, OS_ID_QUOTED_SPACES) == 0);

  CHECK(parse_os_id(OS_RELEASE_NO_ID_FIXTURE, out, sizeof(out)) == false);

  CHECK(parse_os_id(NULL, out, sizeof(out)) == false);
  CHECK(out[0] == '\0');
}

static void test_parse_screen_size(void) {
  int width = 0, height = 0;

  CHECK(parse_screen_size(SCREEN_FB0_FIXTURE, &width, &height) == true);
  CHECK(width == SCREEN_WIDTH);
  CHECK(height == SCREEN_HEIGHT);

  CHECK(parse_screen_size(DMESG_FIXTURE, &width, &height) == true);
  CHECK(width == DMESG_WIDTH);
  CHECK(height == DMESG_HEIGHT);

  // without the dmesg marker the "WxH" format is not recognized
  CHECK(parse_screen_size("1024x768", &width, &height) == false);

  CHECK(parse_screen_size("", &width, &height) == false);
  CHECK(parse_screen_size("not a resolution", &width, &height) == false);
  CHECK(parse_screen_size(NULL, &width, &height) == false);
  CHECK(width == 0 && height == 0);
}

static void test_format_kernel(void) {
  char out[BUF_SIZE];
  char canary[CANARY_SIZE] = CANARY;

  // the getter's alloc() zeroes the buffer; format_kernel only appends
  out[0] = '\0';
  format_kernel(KERNEL_SYSNAME, KERNEL_RELEASE, KERNEL_MACHINE, out, sizeof(out));
  CHECK(strcmp(out, KERNEL_FULL) == 0);

  out[0] = '\0';
  format_kernel(KERNEL_SYSNAME, "", KERNEL_MACHINE, out, sizeof(out));
  CHECK(strcmp(out, KERNEL_NO_RELEASE) == 0);

  out[0] = '\0';
  format_kernel(KERNEL_SYSNAME, KERNEL_RELEASE, "", out, sizeof(out));
  CHECK(strcmp(out, KERNEL_NO_MACHINE) == 0);

  out[0] = '\0';
  format_kernel("", "", "", out, sizeof(out));
  CHECK(out[0] == '\0');

  check_canary(canary);
}

static void test_read_file_head(void) {
  char buf[64];
  char canary[CANARY_SIZE] = CANARY;

  CHECK(mkdtemp(tmp_dir) != NULL);

  write_file("normal.txt", "the content\n");
  snprintf(file_path, sizeof(file_path), "%s/normal.txt", tmp_dir);
  CHECK(read_file_head(file_path, buf, sizeof(buf)) == true);
  CHECK(strcmp(buf, "the content") == 0); // one trailing newline is stripped

  write_file("nonl.txt", "no newline");
  snprintf(file_path, sizeof(file_path), "%s/nonl.txt", tmp_dir);
  CHECK(read_file_head(file_path, buf, sizeof(buf)) == true);
  CHECK(strcmp(buf, "no newline") == 0);

  write_file("empty.txt", "");
  snprintf(file_path, sizeof(file_path), "%s/empty.txt", tmp_dir);
  CHECK(read_file_head(file_path, buf, sizeof(buf)) == true);
  CHECK(buf[0] == '\0');

  write_file("oversize.txt",
             "0123456789012345678901234567890123456789012345678901234567890123456789"
             "0123456789012345678901234567890123456789012345678901234567890123456789"
             "0123456789012345678901234567890123456789012345678901234567890123456789"
             "0123456789012345678901234567890123456789012345678901234567890123456789");
  snprintf(file_path, sizeof(file_path), "%s/oversize.txt", tmp_dir);
  CHECK(read_file_head(file_path, buf, sizeof(buf)) == true);
  CHECK(strlen(buf) == sizeof(buf) - 1);
  CHECK(buf[sizeof(buf) - 1] == '\0');
  check_canary(canary);

  snprintf(file_path, sizeof(file_path), "%s/does_not_exist.txt", tmp_dir);
  CHECK(read_file_head(file_path, buf, sizeof(buf)) == false);

  snprintf(file_path, sizeof(file_path), "%s/normal.txt", tmp_dir);
  unlink(file_path);
  snprintf(file_path, sizeof(file_path), "%s/empty.txt", tmp_dir);
  unlink(file_path);
  snprintf(file_path, sizeof(file_path), "%s/oversize.txt", tmp_dir);
  unlink(file_path);
  snprintf(file_path, sizeof(file_path), "%s/nonl.txt", tmp_dir);
  unlink(file_path);
  rmdir(tmp_dir);
}

int main(void) {
  test_parse_meminfo();
  test_parse_cpu_model();
  test_parse_os_id();
  test_parse_screen_size();
  test_format_kernel();
  test_read_file_head();
  if (failures == 0) printf("test_fetch_parse: all tests passed\n");
  return failures != 0;
}
