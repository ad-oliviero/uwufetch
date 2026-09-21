#!/bin/sh
#
#  UwUfetch is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.

# NOTE: CLI integration tests for uwufetch. The binary must be built first
# (make / make test): the environment is pinned with env -i, so with stdout
# not being a tty the terminal size is unknown, show_info defaults to 70
# columns and the output becomes reproducible. Machine dependent values (cpu,
# gpu, model, memory, uptime, packages) are never asserted, only the
# structure of the output.

set -u
cd "$(dirname "$0")/.."

binary=./build/uwufetch
if [ ! -x "$binary" ]; then
  echo "$binary not found, run make first"
  exit 1
fi

esc=$(printf '\033')
tmp=$(mktemp -d)
mkdir -p "$tmp/.cache" # the cache is written to $HOME/.cache
trap 'rm -rf "$tmp"' EXIT

# pinned environment (USER, HOST and SHELL are read by libfetch)
run() { env -i HOME="$tmp" USER=uwutester HOST=uwuhost SHELL=/bin/sh "$binary" "$@"; }

# removes the ansi escape sequences from a file
strip() { sed "s/${esc}[[0-9;]*[A-Za-z]//g" "$1"; }

# prints the value printed after a label (i.e. "OWOS" -> "Nyarch LinUwU")
field() { strip "$1" | grep "^$2" | head -1 | sed "s/^$2 *//"; }

# counts the art lines: every info line starts with a cursor movement escape
art_lines() { grep -c -v "^${esc}\\[[0-9]*D" "$1"; }

failed=0
check() { # check <description> <condition (evaluated in this shell)>
  if eval "$2"; then
    echo "  OK   $1"
  else
    echo "  FAIL $1"
    failed=$((failed + 1))
  fi
}

echo "case 1: default config"
run > "$tmp/out" 2> "$tmp/err"
status=$?
check "exit code is 0" "test $status -eq 0"
check "stdout is not empty" "test -s '$tmp/out'"
check "no (null) in the output" "! grep -q '(null)' '$tmp/out'"

echo "case 2: no empty enabled text fields"
# MOWODEL, GPUWU and SCWEEN depend on the system (dmi, pci devices,
# framebuffer): they are checked only when present
for label in MOWODEL GPUWU SCWEEN; do
  if strip "$tmp/out" | grep -q "^$label"; then
    check "$label is not empty" "test -n \"\$(field '$tmp/out' $label)\""
  else
    echo "  SKIP $label (not available on this system)"
  fi
done
for label in OWOS KEWNEL CPUWU MEMOWY SHEWW PKGS UWUPTIME; do
  check "$label is not empty" "test -n \"\$(field '$tmp/out' $label)\""
done

echo "case 3: -l debian prints the debian art"
run -l debian > "$tmp/out" 2> "$tmp/err"
status=$?
check "exit code is 0" "test $status -eq 0"
check "debian art marker" "grep -q 'OωO' '$tmp/out'"

echo "case 4: -l nosuch still prints art (logo fallback)"
run -l nosuch > "$tmp/out" 2> "$tmp/err"
status=$?
check "exit code is 0" "test $status -eq 0"
check "art is still printed" "test \"\$(art_lines '$tmp/out')\" -gt 0"

echo "case 5: -c reproduces the cached fields"
run > "$tmp/first" 2> "$tmp/err"
run -c > "$tmp/second" 2> "$tmp/err"
for label in OWOS CPUWU KEWNEL; do
  check "$label matches the cached value" "test \"\$(field '$tmp/first' $label)\" = \"\$(field '$tmp/second' $label)\" -a -n \"\$(field '$tmp/first' $label)\""
done

echo "case 6: -v prints the version"
run -v > "$tmp/out" 2> "$tmp/err"
status=$?
check "exit code is 0" "test $status -eq 0"
check "version line" "grep -q 'UwUfetch version' '$tmp/out'"

echo "case 7: config with everything disabled"
run -C tests/fixtures/off.conf > "$tmp/out" 2> "$tmp/err"
status=$?
check "exit code is 0" "test $status -eq 0"
check "no info labels are printed" "! grep -qE '^(OWOS|MOWODEL|KEWNEL|CPUWU|GPUWU|MEMOWY|SCWEEN|SHEWW|PKGS|UWUPTIME)' '$tmp/out'"

if [ "$failed" -eq 0 ]; then
  echo "run.sh: all tests passed"
else
  echo "run.sh: $failed checks failed"
fi
exit "$failed"
