/* NOTE:
 * This mini-program is a utility that generates the ascii_embed.h file.
 * It was previously written in python, but to reduce the dependancy list
 * Of the project, I rewrote it in C.
 */

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
#define LOGGING_ENABLED
#ifndef OUT_FILE_NAME
  #define OUT_FILE_NAME ""
  #error "OUT_FILE_NAME not specified"
#endif
#ifndef RES_ASCII_DIR_NAME
  #define RES_ASCII_DIR_NAME ""
  #error "RES_ASCII_DIR_NAME not specified"
#endif
#define RES_ASCII_PATH_LEN sizeof(RES_ASCII_DIR_NAME)
#include "../actrie.h"
#include "../common.h"
#include "../libfetch/logging.h"
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  char* str;
  size_t len;
} string;

#define LITERAL_STR(s) \
  (string){s, sizeof(s) - 1} // -1 because we don't need '\0'

struct logo_embed {
  uint32_t id;
  size_t line_count, width;
  string content, name;
};

struct file_consts {
  size_t max_name_len,
      file_count,
      max_line_len,
      max_line_count;
};

bool istxt(const char* str, size_t len) {
  if (len < 5) return 0;
  return str[len - 4] == '.' && str[len - 3] == 't' && str[len - 2] == 'x' && str[len - 1] == 't';
}

// the color tags of the ascii files and their ansi escape replacements
static const struct color_tag {
  const char* tag;
  const char* escape;
} color_tags[] = {
    {"{RED}", "\x1b[31m"},
    {"{BOLD}", "\x1b[1m"},
    {"{BLACK}", "\x1b[30m"},
    {"{BLUE}", "\x1b[34m"},
    {"{CYAN}", "\x1b[36m"},
    {"{PINK}", "\x1b[38;5;201m"},
    {"{GREEN}", "\x1b[32m"},
    {"{WHITE}", "\x1b[37m"},
    {"{LPINK}", "\x1b[38;5;213m"},
    {"{BLOCK}", "▇"},
    {"{NORMAL}", "\x1b[0m"},
    {"{YELLOW}", "\x1b[33m"},
    {"{MAGENTA}", "\x1b[0;35m"},
    {"{SPRING_GREEN}", "\x1b[38;5;120m"},
    {"{BLOCK_VERTICAL}", "▇"},
    {"{BACKGROUND_RED}", "\x1b[0;41m"},
    {"{BACKGROUND_GREEN}", "\x1b[0;42m"},
    {"{BACKGROUND_WHITE}", "\x1b[0;47m"},
};

// upper bound of the content length after the color replacement: in the
// worst case the whole file is made of the most expanding tag
static size_t worst_color_len(size_t len) {
  size_t max_escape = 1, min_tag = 1;
  for (size_t i = 0; i < sizeof(color_tags) / sizeof(color_tags[0]); i++) {
    size_t escape_len = strlen(color_tags[i].escape);
    size_t tag_len    = strlen(color_tags[i].tag);
    if (escape_len > max_escape) max_escape = escape_len;
    if (tag_len < min_tag) min_tag = tag_len;
  }
  size_t worst = len * max_escape / min_tag;
  return worst > len ? worst : len;
}

void load_ascii_file(struct logo_embed* logo, char* path) {
  FILE* af = NULL;
  CHECK_FN_NULL_EXIT((af = fopen(path, "rb")));
  fseek(af, 0, SEEK_END);
  logo->content.len = (size_t)ftell(af);
  fseek(af, 0, SEEK_SET);
  CHECK_FN_NULL_EXIT((logo->content.str = malloc(worst_color_len(logo->content.len) + 1)));
  fread(logo->content.str, sizeof(char), logo->content.len, af);
  logo->content.str[logo->content.len] = 0;
  CHECK_FN_NEG_EXIT(fclose(af));
}

void replace_all_colors(string* fcontent, struct actrie_t* replacer) {
  fcontent->len = actrie_t_replace_all_occurances_len(replacer, fcontent->str, fcontent->len);
}

static int txt_filter(const struct dirent* entry) {
  size_t nlen = strlen(entry->d_name);
  return istxt(entry->d_name, nlen);
}

static int dirent_name_cmp(const void* a, const void* b) {
  return strcmp((*(const struct dirent* const*)a)->d_name, (*(const struct dirent* const*)b)->d_name);
}

struct logo_embed* load_files(struct file_consts* consts, struct actrie_t* replacer) {
  struct logo_embed* files = NULL;
  string full_path         = {NULL, 0};
  string path_end          = {NULL, 0};
  struct dirent** txtfs    = NULL;

  int file_count = scandir(RES_ASCII_DIR_NAME, &txtfs, txt_filter, NULL);
  if (file_count < 0) {
    LOG_E("Failed to read " RES_ASCII_DIR_NAME);
    exit(1);
  }
  // the scandir order is not deterministic, the file names are sorted to make
  // the generated header reproducible
  qsort(txtfs, (size_t)file_count, sizeof(struct dirent*), dirent_name_cmp);
  consts->file_count = (size_t)file_count;
  CHECK_FN_NULL_EXIT((files = malloc(consts->file_count * sizeof(struct logo_embed))));

  full_path.len = RES_ASCII_PATH_LEN + consts->max_name_len;
  CHECK_FN_NULL_EXIT((full_path.str = malloc(full_path.len)));

  // the initial part of the path will never change
  memcpy(full_path.str, RES_ASCII_DIR_NAME, RES_ASCII_PATH_LEN);
  path_end.str = full_path.str + RES_ASCII_PATH_LEN - 1;
  path_end.len = full_path.len - RES_ASCII_PATH_LEN;

  for (size_t i = 0; i < consts->file_count; i++) {
    const char* name = txtfs[i]->d_name;
    size_t nlen      = strlen(name);
    if (nlen > path_end.len) {
      consts->max_name_len = nlen;
      full_path.str        = realloc(full_path.str, full_path.len + (nlen - path_end.len));
      path_end.str         = full_path.str + RES_ASCII_PATH_LEN - 1;
      path_end.len         = full_path.len - RES_ASCII_PATH_LEN;
    }

    CHECK_ERRNO_EXIT(memcpy(path_end.str, name, nlen));
    full_path.str[RES_ASCII_PATH_LEN + nlen - 1] = 0;

    files[i].name.len = nlen;
    CHECK_FN_NULL_EXIT((files[i].name.str = malloc((files[i].name.len + 1) * sizeof(char))));
    memcpy(files[i].name.str, name, files[i].name.len);
    files[i].name.str[files[i].name.len] = 0;
    load_ascii_file(&files[i], full_path.str);
    replace_all_colors(&files[i].content, replacer);

    string line       = files[i].content;
    char* prevline    = line.str;
    size_t line_count = 0;
    while (*line.str++) {
      if (*line.str == '\n') {
        *line.str            = 0;
        size_t newlen        = (size_t)(line.str - prevline);
        consts->max_line_len = newlen > consts->max_line_len ? newlen : consts->max_line_len;
        line_count++;
        prevline = line.str++;
      }
    }
    consts->max_line_count = line_count > consts->max_line_count ? line_count : consts->max_line_count;
  }

  for (size_t i = 0; i < consts->file_count; i++)
    free(txtfs[i]);
  free(txtfs);
  free(full_path.str);
  return files;
}

void debug_logo(struct actrie_t* replacer, char* logo_name) {
  struct logo_embed logo = {.name = (string){logo_name, strlen(logo_name)}};
  logo.id                = str2id(logo.name.str, (int)logo.name.len);
  char* path             = malloc(RES_ASCII_PATH_LEN + logo.name.len + 6); // +2 for \0 and +4 for .txt
  memcpy(path, RES_ASCII_DIR_NAME, RES_ASCII_PATH_LEN);
  memcpy(path + RES_ASCII_PATH_LEN - 1, logo.name.str, logo.name.len);
  memcpy(path + RES_ASCII_PATH_LEN + logo.name.len - 1, ".txt", sizeof(".txt"));
  path[RES_ASCII_PATH_LEN + logo.name.len + 5] = 0;
  load_ascii_file(&logo, path);
  LOG_V(logo.id);
  LOG_V(logo.name.str);
  LOG_V(path);

  replace_all_colors(&logo.content, replacer);
  for (size_t i = 0; i < logo.content.len; i++)
    putc(logo.content.str[i], stdout);

  puts("\x1b[0m");

  free(logo.content.str);
  free(path);
}

int main(int argc, char** argv) {
  logging_level = 4;

  // setting up colors replacement
  struct actrie_t replacer;
  actrie_t_ctor(&replacer);
  actrie_t_reserve_patterns(&replacer, sizeof(color_tags) / sizeof(color_tags[0]));
  for (size_t i = 0; i < sizeof(color_tags) / sizeof(color_tags[0]); i++)
    actrie_t_add_pattern(&replacer, color_tags[i].tag, color_tags[i].escape);
  actrie_t_compute_links(&replacer);

  if (argc > 1) {
    debug_logo(&replacer, argv[1]);
    exit(0);
  }
  /*
   * ---- variable definitions ----
   */
  FILE* outf               = NULL;
  struct logo_embed* files = NULL;

  /* These values are hard coded, but could change, so to *not*
   * recompile every time this program, I programmed them so
   * even if they are hard coded, they can change if needed */
  struct file_consts consts = {0};

  /*
   * ---- getting resources ----
   */

  CHECK_FN_NULL_EXIT((outf = fopen(OUT_FILE_NAME, "w")));

  /*
   * ---- preparing the output file ----
   */

  files = load_files(&consts, &replacer);
  CHECK_FN_NEG_EXIT(fprintf(outf,
                            "#ifndef ASCII_EMBED_H\n"
                            "#define ASCII_EMBED_H\n\n"
                            "#include <assert.h>\n"
                            "#include <stdint.h>\n"
                            "#include <unistd.h>\n\n"
                            "struct logo_line {\n"
                            "  const size_t length;\n"
                            "  const unsigned char content[%lu];\n"
                            "};\n"
                            "struct logo_embed {\n"
                            "  const struct logo_line lines[%lu];\n"
                            "  const uint32_t id;\n"
                            "  const size_t line_count, width;\n"
                            "};\n"
                            "static const struct logo_embed logos[%lu] = {\n",
                            consts.max_line_len, consts.max_line_count, consts.file_count));

  /*
   * ---- processing ----
   */

  for (size_t i = 0; i < consts.file_count; i++) {
    size_t line_count = 0;
    size_t logo_width = 0;
    string line       = files[i].content;
    CHECK_FN_NEG_EXIT(fprintf(outf, "    {\n        .lines      = {\n"));
    for (size_t j = 0; *line.str && (size_t)(line.str - files[i].content.str) < files[i].content.len;) {
      while (*(line.str + j++));
      CHECK_FN_NEG_EXIT(fprintf(outf, "            {.length = %li, .content = {", j));
      while (*line.str)
        CHECK_ERRNO_EXIT(fprintf(outf, "0x%02x, ", (unsigned char)*line.str++));
      CHECK_ERRNO_EXIT(fprintf(outf, "}},\n"));
      line.str++;
      logo_width = j > logo_width ? j : logo_width;
      j          = 0;
      line_count++;
    }
    CHECK_FN_NEG_EXIT(fprintf(outf, "        },\n"
                                    "        .id         = 0x%x, // file name: %s\n"
                                    "        .line_count = %lu,\n"
                                    "        .width      = %lu,\n"
                                    "    },\n",
                              str2id(files[i].name.str, (int)files[i].name.len - 4), files[i].name.str, line_count, logo_width));
  }

  /*
   * ----completing output file----
   */

  CHECK_ERRNO_EXIT(fprintf(outf,
                           "};\n"
                           "static const unsigned int logos_count __attribute__((unused)) = sizeof(logos) / sizeof(logos[0]);\n"
                           "static_assert(sizeof(logos) / sizeof(logos[0]) > 0, \"empty logo table\");\n"
                           "#endif // ASCII_EMBED_H\n"));

  /*
   * ---- releasing resources ----
   */

  actrie_t_dtor(&replacer);
  CHECK_FN_NEG(fclose(outf));
  for (size_t i = 0; i < consts.file_count; i++) {
    free(files[i].content.str);
    free(files[i].name.str);
  }
  free(files);

  LOG_I("Completed " OUT_FILE_NAME " with %d errors", logging_error_count);
  exit(0);
}
