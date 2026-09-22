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

#include "../src/common.h"
#include <stdio.h>
#include <string.h>

#include "tests.h"

// known Jenkins one-at-a-time hash vectors
static void test_known_vectors(void) {
  CHECK(str2id("", 0) == 0x00000000u);
  CHECK(str2id("a", 1) == 0xCA2E9442u);
  CHECK(str2id("arch", 4) == 0xCD997403u);
  CHECK(str2id("debian", 6) == 0x339322F1u);
  CHECK(str2id("uwufetch", 8) == 0xB3B41C79u);
  CHECK(str2id("linuxmint", 9) == 0xE1E26A8Au);
}

// the ids must be stable across calls and different for different names
static void test_stability(void) {
  uint32_t first = str2id("arch", 4);
  for (int i = 0; i < 100; i++) CHECK(str2id("arch", 4) == first);
  CHECK(str2id("arch", 4) != str2id("arch2", 5));
  CHECK(str2id("manjaro", 7) != str2id("manjaro-arm", 11));
}

int main(void) {
  test_known_vectors();
  test_stability();
  if (failures == 0) fprintf(stderr, "test_common: all tests passed\n");
  return failures != 0;
}
