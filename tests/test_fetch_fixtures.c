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

#include <stdio.h>
#include <string.h>

#include "tests.h"

// fetch.c is included with the paths overridden to read tests/fixtures/proc
#define PROC_CPUINFO_PATH "fixtures/proc/cpuinfo"
#define FB0_VIRTUAL_SIZE_PATH "fixtures/proc/fb0"
#define OS_RELEASE_PATH "fixtures/proc/os_release"
#define HOSTNAME_PATH "fixtures/proc/hostname"
#if defined(FETCH_FIXTURE_EMPTY)
  #define PROC_MEMINFO_PATH "fixtures/proc/meminfo_empty"
#elif defined(FETCH_FIXTURE_NO_TOTAL)
  #define PROC_MEMINFO_PATH "fixtures/proc/meminfo_no_total"
#else
  #define PROC_MEMINFO_PATH "fixtures/proc/meminfo"
#endif
#include "../src/libfetch/fetch.c"

// expected get_memory_used() for fixtures/proc/meminfo:
// (16384000 - (4200000 + 640000 + 4600000)) / 1024
#define EXPECTED_MEMORY_USED 6781ull
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define CPU_MODEL "Fixture CPU Model @ 2.9GHz"
#define OS_ID "debian"

#if defined(SYSTEM_BASE_LINUX)

static void run_cycle(void) {
  libfetch_init();

  #ifdef FETCH_FIXTURE_EMPTY
  CHECK(get_memory_used() == 0);
  #elif defined(FETCH_FIXTURE_NO_TOTAL)
  CHECK(get_memory_used() == 0);
  #else
  CHECK(get_memory_used() == EXPECTED_MEMORY_USED);
  #endif

  CHECK(get_memory_total() > 0);

  char* cpu = get_cpu();
  CHECK(cpu != NULL && strcmp(cpu, CPU_MODEL) == 0);

  CHECK(get_screen_width() == SCREEN_WIDTH);
  CHECK(get_screen_height() == SCREEN_HEIGHT);

  char* os_name = get_os_name();
  CHECK(os_name != NULL && strcmp(os_name, OS_ID) == 0);

  libfetch_cleanup();
}

int main(void) {
  run_cycle();
  if (failures == 0) printf("test_fetch_fixtures: all tests passed\n");
  return failures != 0;
}

#else // the macOS init reads no files, the fixture paths are unreachable

int main(void) {
  printf("test_fetch_fixtures: all tests passed\n");
  return 0;
}

#endif
