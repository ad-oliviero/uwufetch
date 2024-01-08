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
        h &= 0xFFFFFFFF
        h += h << 10
        h &= 0xFFFFFFFF
        h ^= h >> 6
        h &= 0xFFFFFFFF
    h += h << 3
    h &= 0xFFFFFFFF
    h ^= h >> 11
    h &= 0xFFFFFFFF
    h += h << 15
    return h & 0xFFFFFFFF


# PlaceHolder to ansi escape Color
def ph2c(src: str) -> str:
    for p, r in COLOR_PHS.items():
        src = re.sub(p, r, src)
    return src


# MaKe Line Indipendent
# separates colors (placeholders) for each line
def mkli(src: str, color: list[tuple]) -> tuple[str, int, int]:
    in_line_colors = [(s.group(), s.start()) for s in re.finditer(ALL_COLORS_REGEX, l)]
    cc = len(in_line_colors)
    ncst = src
    for s in [k for k in COLOR_PHS]:
        ncst = ncst.replace(s, "")
    width = len(ncst)
    if src != "":
        if in_line_colors != []:
            if in_line_colors[0][1] != 0 and color[0] != None:
                src = color[0][0] + src
            if in_line_colors[-1][0] != "{NORMAL}":
                color[0] = in_line_colors[len(in_line_colors) - 1]
                src += "{NORMAL}"
                cc += 1
        else:
            if color[0] != None:
                src = color[0][0] + src
            src += "{NORMAL}"
            cc += 1
    return (src, cc, width)


if __name__ == "__main__":
    newContent = ""
    txts = [e for e in os.listdir(ASCII_DIR) if e.endswith(".txt")]
    newContent += f"static const struct logo_embed logos[{len(txts)}] = {{\n"
    maxLineLength = 0
    maxLineCount = 0
    for fn in txts:
        bnfn = fn.replace(".txt", "")
        newContent += f"{{.id={hex(jenkins_hash(bnfn))}, // file name: {bnfn}\n"
        localMaxLineLength = 0
        with open(f"{ASCII_DIR}/{fn}", "r") as af:
            content = af.read()
            lines = []
            current_color = [("", 0)]
            width = 0
            # split every line,
            # make it "indipendent" and
            # convert every color_placeholder
            for l in content.split("\n"):
                if l != "":
                    l, cc, width = mkli(l, current_color)
                    lines.append(ph2c(l))
            lc = len(lines)
            if lc > maxLineCount:
                maxLineCount = lc
            newContent += f".line_count={len(lines)},.max_length={{MAX_LENGTH_PLACEHOLDER_{fn}}},.width={{WIDTH_PLACEHOLDER_{fn}}},.lines={{"
            # convert every line in its hex representation
            for l in lines:
                visualLength = 0
                # increment visual length if some chars are not ascii
                for c in l:
                    isUnicode = ord(c) > 255
                    visualLength += isUnicode
                    width += isUnicode
                lineAsciiCodes = [c for c in l.encode("utf-8")]
                llen = len(lineAsciiCodes)
                if llen > maxLineLength:
                    maxLineLength = llen
                if llen > localMaxLineLength:
                    localMaxLineLength = llen
                newContent += (
                    f"{{.length = {llen+1},.visual_length={visualLength},.content={{"
                    + ",".join(map(hex, lineAsciiCodes))
                    + "}},"
                )
            newContent = newContent.replace(
                f"{{MAX_LENGTH_PLACEHOLDER_{fn}}}", str(localMaxLineLength)
            ).replace(f"{{WIDTH_PLACEHOLDER_{fn}}}", str(width))

        newContent += "},},\n"

    newContent = (
        f"""
#ifndef _ASCII_EMBED_H_
#define _ASCII_EMBED_H_

#include <stdint.h>
#include <unistd.h>

struct logo_line {{
  const size_t length, visual_length;
  const unsigned char content[{maxLineLength}];
}};

struct logo_embed {{
  const uint32_t id;
  const size_t line_count, max_length, width;
  const struct logo_line lines[{maxLineCount}];
}};

"""
        + newContent
        + f"""}};
static const unsigned int logos_count __attribute__((unused)) = {len(txts)};
#endif // _ASCII_EMBED_H_
"""
    )
    with open(f"{os.environ.get('SRC_DIR')}/ascii_embed.h", "w") as of:
        of.write(newContent)
