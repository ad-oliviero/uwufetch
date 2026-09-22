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
#include <string.h>

#include "tests.h"

int main(void) {
#if defined(SYSTEM_BASE_MACOS)
  char** gpu_list = get_gpu_list();
  CHECK(gpu_list != NULL);
  if (gpu_list) {
    size_t gpu_count = (size_t)gpu_list[0]; // the [0] element is the "gpu count"
    CHECK(gpu_count >= 1);
    for (size_t i = 1; i <= gpu_count; i++) {
      CHECK(gpu_list[i] != NULL);
      if (gpu_list[i]) CHECK(gpu_list[i][0] != '\0');
    }
  }
  libfetch_cleanup();
#else
  printf("  SKIP gpu detection (macos only)\n");
#endif
  return failures;
}
