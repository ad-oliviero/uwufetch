#!/bin/bash

set -e

out_file=$SRC_DIR/ascii_embed.h

printf "#ifndef _ASCII_EMBED_H_\n#define _ASCII_EMBED_H_\n\n#include <stdint.h>\n\nstruct logo_embed {unsigned char* content; unsigned int length; uint64_t id;};\n\nstatic struct logo_embed logos[] = {\n" >> $out_file

txts=$(find res/ascii -name "*.txt" -print)
for f in $txts; do
  # replace the color placeholder {COLOR} with the ansi escape code, then convert it to a C variable with xxd
  content=$(cat $f | sed 's/{NORMAL}/\x1b[0m/g;s/{BOLD}/\x1b[1m/g;s/{BLACK}/\x1b[30m/g;s/{RED}/\x1b[31m/g;s/{GREEN}/\x1b[32m/g;s/{SPRING_GREEN}/\x1b[38;5;120m/g;s/{YELLOW}/\x1b[33m/g;s/{BLUE}/\x1b[34m/g;s/{MAGENTA}/\x1b[0;35m/g;s/{CYAN}/\x1b[36m/g;s/{WHITE}/\x1b[37m/g;s/{PINK}/\x1b[38;5;201m/g;s/{LPINK}/\x1b[38;5;213m/g;s/{\(BLOCK_VERTICAL\|BLOCK\)}/▇/g;s/{BACKGROUND_GREEN}/\e[0;42m/g;s/{BACKGROUND_RED}/\e[0;41m/g;s/{BACKGROUND_WHITE}/\e[0;47m/g' | xxd -n content -i)

  printf "$content" | awk '
  BEGIN {print "  (struct logo_embed){"}
  {print "    " $0}
END {printf "    .id = "}
  ' | sed 's/unsigned int content_len/.length/g;s/unsigned char /./g;s/\[\] = /= \(unsigned char\[\]\)/g;s/;/,/g' >> $out_file

  bn=$(basename $f | sed "s/.txt//g")
  python -c '
import sys
if len(sys.argv) < 2:
  exit(1)
h = 0
for c in sys.argv[1]:
  h += ord(c)
  h += h << 10
  h ^= h >> 6
h += h << 3
h ^= h >> 11
h += h << 15
print(hex(h & 0xffffffffffffffff), end="")
' "$bn" >> $out_file
printf " // file name: $bn\n  }," >> $out_file
done

printf "};\nstatic unsigned int logos_count = $(printf "$txts\n" | wc -l);\n\n#endif\n" >> $out_file

