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
  (string) { s, sizeof(s) - 1 } // -1 because we don't need '\0'

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

/* https://github.com/lattera/freebsd/blob/master/lib/libc/string/strnstr.c */
char* strnstr(const char* s, const char* find, size_t slen) {
  char c, sc;
  size_t len;

  if ((c = *find++) != '\0') {
    len = strlen(find);
    do {
      do {
        if (slen-- < 1 || (sc = *s++) == '\0')
          return (NULL);
      } while (sc != c);
      if (len > slen)
        return (NULL);
    } while (strncmp(s, find, len) != 0);
    s--;
  }
  return ((char*)s);
}

// TODO: use actrie.c instead of this function
void replace(string* src, string* find, string* replace) {
  long int diff = (long int)find->len - (long int)replace->len;
  string ptr    = {src->str, src->len};
  while ((ptr.str = strnstr(ptr.str, find->str, ptr.len))) {
    ptr.len = src->len - (ptr.str - src->str);
    if (diff) memmove(ptr.str + replace->len, ptr.str + find->len, ptr.len); // making space for the replaced string
    memcpy(ptr.str, replace->str, replace->len);
    ptr.str += replace->len;
    src->len += diff;
  }
}

struct logo_embed* load_files(DIR* d, struct file_consts* consts /*, struct actrie_t* replacer*/) {
  struct dirent* txtfs     = NULL;
  struct logo_embed* files = NULL;
  string full_path         = {NULL};
  string path_end          = {NULL};
  full_path.len            = RES_ASCII_PATH_LEN + consts->max_name_len;
  CHECK_FN_NULL_EXIT((full_path.str = malloc(full_path.len)));

  // we need to count the files before doing anything else
  while ((txtfs = readdir(d)) != NULL) {
    size_t nlen = strlen(txtfs->d_name);
    if (istxt(txtfs->d_name, nlen))
      consts->file_count++;
  }
  CHECK_FN_NULL_EXIT((files = malloc(consts->file_count * sizeof(struct logo_embed))));
  rewinddir(d);
  consts->file_count = 0;

  // the initial part of the path will never change
  memcpy(full_path.str, RES_ASCII_DIR_NAME, RES_ASCII_PATH_LEN);
  path_end.str = full_path.str + RES_ASCII_PATH_LEN - 1;
  path_end.len = full_path.len - RES_ASCII_PATH_LEN;

  while ((txtfs = readdir(d)) != NULL) {
    size_t nlen = strlen(txtfs->d_name);
    if (istxt(txtfs->d_name, nlen)) {
      if (nlen > path_end.len) {
        consts->max_name_len = nlen;
        full_path.str        = realloc(full_path.str, full_path.len + (nlen - path_end.len));
        path_end.str         = full_path.str + RES_ASCII_PATH_LEN - 1;
        path_end.len         = full_path.len - RES_ASCII_PATH_LEN;
      }

      CHECK_ERRNO_EXIT(memcpy(path_end.str, txtfs->d_name, nlen));
      full_path.str[RES_ASCII_PATH_LEN + nlen - 1] = 0;

      FILE* af = NULL;
      CHECK_FN_NULL_EXIT((af = fopen(full_path.str, "rb")));
      fseek(af, 0, SEEK_END);
      files[consts->file_count].content.len = (size_t)ftell(af);
      fseek(af, 0, SEEK_SET);
      // TODO:
      // CHECK_FN_NULL_EXIT((files[consts->file_count].content.str = malloc((files[consts->file_count].content.len + 1) * sizeof(char))));
      CHECK_FN_NULL_EXIT((files[consts->file_count].content.str = malloc(4096 > files[consts->file_count].content.len + 1 ? 4096 : files[consts->file_count].content.len + 1)));
      fread(files[consts->file_count].content.str, sizeof(char), files[consts->file_count].content.len, af);
      files[consts->file_count].content.str[files[consts->file_count].content.len] = 0;
      CHECK_FN_NEG_EXIT(fclose(af));

      files[consts->file_count].name.len = nlen;
      CHECK_FN_NULL_EXIT((files[consts->file_count].name.str = malloc((files[consts->file_count].name.len + 1) * sizeof(char))));
      memcpy(files[consts->file_count].name.str, txtfs->d_name, files[consts->file_count].name.len);
      files[consts->file_count].name.str[files[consts->file_count].name.len] = 0;

      // files[consts->file_count].content.len = actrie_t_replace_all_occurances(replacer, (char*)files[consts->file_count].content.str);
      replace(&files[consts->file_count].content, &LITERAL_STR("{RED}"), &LITERAL_STR("\x1b[31m"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{BOLD}"), &LITERAL_STR("\x1b[1m"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{BLACK}"), &LITERAL_STR("\x1b[30m"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{BLUE}"), &LITERAL_STR("\x1b[34m"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{CYAN}"), &LITERAL_STR("\x1b[36m"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{PINK}"), &LITERAL_STR("\x1b[38;5;201m"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{GREEN}"), &LITERAL_STR("\x1b[32m"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{WHITE}"), &LITERAL_STR("\x1b[37m"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{LPINK}"), &LITERAL_STR("\x1b[38;5;213m"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{BLOCK}"), &LITERAL_STR("▇"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{NORMAL}"), &LITERAL_STR("\x1b[0m"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{YELLOW}"), &LITERAL_STR("\x1b[33m"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{MAGENTA}"), &LITERAL_STR("\x1b[0;35m"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{SPRING_GREEN}"), &LITERAL_STR("\x1b[38;5;120m"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{BLOCK_VERTICAL}"), &LITERAL_STR("▇"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{BACKGROUND_RED}"), &LITERAL_STR("\x1b[0;41m"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{BACKGROUND_GREEN}"), &LITERAL_STR("\x1b[0;42m"));
      replace(&files[consts->file_count].content, &LITERAL_STR("{BACKGROUND_WHITE}"), &LITERAL_STR("\x1b[0;47m"));

      string line       = files[consts->file_count].content;
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
      consts->file_count++;
    }
  }

  free(full_path.str);
  return files;
}

int main() {
  logging_level = 4;
  /*
   * ---- variable definitions ----
   */
  FILE* outf               = NULL;
  DIR* res_ascii           = NULL;
  struct logo_embed* files = NULL;
  // struct actrie_t replacer;

  /* These values are hard coded, but could change, so to *not*
   * recompile every time this program, I programmed them so
   * even if they are hard coded, they can change if needed */
  struct file_consts consts = {0};

  /*
   * ---- getting resources ----
   */

  CHECK_FN_NULL_EXIT((outf = fopen(OUT_FILE_NAME, "w")));
  CHECK_FN_NULL_EXIT((res_ascii = opendir(RES_ASCII_DIR_NAME)));

  // setting up colors replacement
  /*
  actrie_t_ctor(&replacer);
  actrie_t_reserve_patterns(&replacer, 18);
  actrie_t_add_pattern(&replacer, "{RED}", "\x1b[31m");
  actrie_t_add_pattern(&replacer, "{BOLD}", "\x1b[1m");
  actrie_t_add_pattern(&replacer, "{BLACK}", "\x1b[30m");
  actrie_t_add_pattern(&replacer, "{BLUE}", "\x1b[34m");
  actrie_t_add_pattern(&replacer, "{CYAN}", "\x1b[36m");
  actrie_t_add_pattern(&replacer, "{PINK}", "\x1b[38;5;201m");
  actrie_t_add_pattern(&replacer, "{GREEN}", "\x1b[32m");
  actrie_t_add_pattern(&replacer, "{WHITE}", "\x1b[37m");
  actrie_t_add_pattern(&replacer, "{LPINK}", "\x1b[38;5;213m");
  actrie_t_add_pattern(&replacer, "{BLOCK}", "▇");
  actrie_t_add_pattern(&replacer, "{NORMAL}", "\x1b[0m");
  actrie_t_add_pattern(&replacer, "{YELLOW}", "\x1b[33m");
  actrie_t_add_pattern(&replacer, "{MAGENTA}", "\x1b[0;35m");
  actrie_t_add_pattern(&replacer, "{SPRING_GREEN}", "\x1b[38;5;120m");
  actrie_t_add_pattern(&replacer, "{BLOCK_VERTICAL}", "▇");
  actrie_t_add_pattern(&replacer, "{BACKGROUND_RED}", "\x1b[0;41m");
  actrie_t_add_pattern(&replacer, "{BACKGROUND_GREEN}", "\x1b[0;42m");
  actrie_t_add_pattern(&replacer, "{BACKGROUND_WHITE}", "\x1b[0;47m");
  actrie_t_compute_links(&replacer);
  */

  /*
   * ---- preparing the output file ----
   */

  files = load_files(res_ascii, &consts /*, &replacer*/);
  CHECK_FN_NEG(closedir(res_ascii));
  CHECK_FN_NEG_EXIT(fprintf(outf,
                            "#ifndef _ASCII_EMBED_H_\n"
                            "#define _ASCII_EMBED_H_\n\n"
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
  // for (size_t i = 20; i <= 20; i++) {
    size_t line_count = 0;
    size_t logo_width = 0;
    string line       = files[i].content;
    CHECK_FN_NEG_EXIT(fprintf(outf, "    {\n        .lines      = {\n"));
    for (size_t j = 0; *line.str && (size_t)(line.str - files[i].content.str) < files[i].content.len;) {
      while (*(line.str + j++))
        ;
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

  CHECK_ERRNO_EXIT(fprintf(outf, "};\nstatic const unsigned int logos_count __attribute__((unused)) = 35;\n#endif // _ASCII_EMBED_H_"));

  /*
   * ---- releasing resources ----
   */

  // actrie_t_dtor(&replacer);
  CHECK_FN_NEG(fclose(outf));
  for (size_t i = 0; i < consts.file_count; i++) {
    free(files[i].content.str);
    free(files[i].name.str);
  }
  free(files);

  LOG_I("Completed " OUT_FILE_NAME " with %d errors", logging_error_count);
  exit(0);
}
