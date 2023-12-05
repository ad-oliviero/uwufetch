#!/bin/bash

set -e

out_file=$SRC_DIR/ascii_embed.h
txts=$(find res/ascii -name "*.txt" -print)
txtsc=$(printf "$txts\n" | wc -l)

printf "#ifndef _ASCII_EMBED_H_\n#define _ASCII_EMBED_H_\n\n#include <stdint.h>\n\nstruct logo_embed {uint64_t id; unsigned int length; unsigned char content[$(expr $(du -b $txts | cut -f1 | sort -rn | head -n1) + 1)];};\n\nstatic struct logo_embed logos[$txtsc] __attribute__((unused)) = {\n" >> $out_file

for f in $txts; do
  bn=$(basename $f | sed "s/.txt//g")
  printf "  { .id = " >> $out_file
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
  printf ", // file name: $bn\n" >> $out_file

  # replace the color placeholder {COLOR} with the ansi escape code, then convert it to a C variable with xxd
  content=$(cat $f | sed 's/{NORMAL}/\x1b[0m/g;s/{BOLD}/\x1b[1m/g;s/{BLACK}/\x1b[30m/g;s/{RED}/\x1b[31m/g;s/{GREEN}/\x1b[32m/g;s/{SPRING_GREEN}/\x1b[38;5;120m/g;s/{YELLOW}/\x1b[33m/g;s/{BLUE}/\x1b[34m/g;s/{MAGENTA}/\x1b[0;35m/g;s/{CYAN}/\x1b[36m/g;s/{WHITE}/\x1b[37m/g;s/{PINK}/\x1b[38;5;201m/g;s/{LPINK}/\x1b[38;5;213m/g;s/{\(BLOCK_VERTICAL\|BLOCK\)}/▇/g;s/{BACKGROUND_GREEN}/\e[0;42m/g;s/{BACKGROUND_RED}/\e[0;41m/g;s/{BACKGROUND_WHITE}/\e[0;47m/g' | xxd -n content -i)
  # separate the content "string" and its length
  clen=$(printf "$content" | grep "content_len" | awk '{print $5}' | tr -d ';')
  content=$(printf "$content" | grep -v "content_len")
  printf "    .length = %s,\n" $clen >> $out_file
  printf "    $content},\n" | sed "s/unsigned char /./g;s/\[\]//g" | tr -d ';' >> $out_file
done

printf "};\nstatic unsigned int logos_count __attribute__((unused)) = $txtsc;\n\n#endif\n" >> $out_file

