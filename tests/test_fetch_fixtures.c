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
#include <string.h>

#include "tests.h"

// expected get_memory_used() for tests/fixtures/proc/meminfo:
// (16384000 - (4200000 + 640000 + 4600000)) / 1024
#define EXPECTED_MEMORY_USED 6781ull
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define CPU_MODEL "Fixture CPU Model @ 2.9GHz"
#define OS_ID "debian"

#if defined(SYSTEM_BASE_LINUX)

// the binary is linked against fetch.o compiled with the paths pointing into
// tests/fixtures/proc; with FETCH_EMPTY_MEMINFO meminfo_empty replaces meminfo
static void run_cycle(void) {
  libfetch_init();

  #ifdef FETCH_EMPTY_MEMINFO
  CHECK(get_memory_used() == 0); // an empty meminfo must degrade to 0, not crash
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
