import os, regex as re


ASCII_DIR = "res/ascii"
COLOR_PHS = {
    "{NORMAL}": "\x1b[0m",
    "{BOLD}": "\x1b[1m",
    "{BLACK}": "\x1b[30m",
    "{RED}": "\x1b[31m",
    "{GREEN}": "\x1b[32m",
    "{SPRING_GREEN}": "\x1b[38;5;120m",
    "{YELLOW}": "\x1b[33m",
    "{BLUE}": "\x1b[34m",
    "{MAGENTA}": "\x1b[0;35m",
    "{CYAN}": "\x1b[36m",
    "{WHITE}": "\x1b[37m",
    "{PINK}": "\x1b[38;5;201m",
    "{LPINK}": "\x1b[38;5;213m",
    "{BLOCK_VERTICAL}": "▇",
    "{BLOCK}": "▇",
    "{BACKGROUND_GREEN}": "\x1b[0;42m",
    "{BACKGROUND_RED}": "\x1b[0;41m",
    "{BACKGROUND_WHITE}": "\x1b[0;47m",
}
ALL_COLORS_REGEX = re.compile("|".join(COLOR_PHS.keys()))


# calculates the jenkins hash, used to give an id to a logo
def jenkins_hash(key: str) -> int:
    h = 0
    for c in key:
        h += ord(c)
        h += h << 10
        h ^= h >> 6
    h += h << 3
    h ^= h >> 11
    h += h << 15
    return h & 0xFFFFFFFFFFFFFFFF


# PlaceHolder to ansi escape Color
def ph2c(src: str) -> str:
    for p, r in COLOR_PHS.items():
        src = re.sub(p, r, src)
    return src


# MaKe Line Indipendent
# separates colors (placeholders) for each line
def mkli(src: str, color: list[tuple]) -> str:
    if src != "":
        lc = [(s.group(), s.start()) for s in re.finditer(ALL_COLORS_REGEX, src)]
        if lc != []:
            if lc[0][1] != 0 and color[0] != None:
                src = color[0][0] + src
            if lc[-1][0] != "{NORMAL}":
                color[0] = lc[len(lc) - 1]
                src += "{NORMAL}"
        else:
            if color[0] != None:
                src = color[0][0] + src
            src += "{NORMAL}"
    return src


if __name__ == "__main__":
    newContent = ""
    txts = [e for e in os.listdir(ASCII_DIR) if e.endswith(".txt")]
    newContent += f"static struct logo_embed logos[{len(txts)}] = {{\n"
    maxLineLength = 0
    maxLineCount = 0
    for fn in txts:
        bnfn = fn.replace(".txt", "")
        newContent += f"{{.id={hex(jenkins_hash(bnfn))}, // file name: {bnfn}\n"
        with open(f"{ASCII_DIR}/{fn}", "r") as af:
            content = af.read()
            lines = []
            current_color = [("", 0)]
            # split every line,
            # make it "indipendent" and
            # convert every color_placeholder
            for l in content.split("\n"):
                if l != "":
                    l = mkli(l, current_color)
                    lines.append(ph2c(l))
            lc = len(lines)
            if lc > maxLineCount:
                maxLineCount = lc
            newContent += f".line_count={len(lines)},.lines={{"
            # convert every line in its hex representation
            for l in lines:
                lineAsciiCodes = [c for c in l.encode("utf-8")]
                llen = len(lineAsciiCodes)
                if llen > maxLineLength:
                    maxLineLength = llen
                newContent += (
                    f"{{.length = {llen},.content={{"
                    + ",".join(map(hex, lineAsciiCodes))
                    + "}},"
                )

        newContent += "},},\n"

    newContent = (
        f"""
#ifndef _ASCII_EMBED_H_
#define _ASCII_EMBED_H_

#include <stdint.h>
#include <unistd.h>

struct logo_line {{
  size_t length;
  unsigned char content[{maxLineLength}];
}};

struct logo_embed {{
  uint64_t id;
  size_t line_count;
  struct logo_line lines[{maxLineCount}];
}};

"""
        + newContent
        + f"""}};
static unsigned int logos_count __attribute__((unused)) = {len(txts)};
#endif // _ASCII_EMBED_H_
"""
    )
    with open(f"{os.environ.get('SRC_DIR')}/ascii_embed.h", "w") as of:
        of.write(newContent)
