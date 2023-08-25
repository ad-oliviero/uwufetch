#include "../fetch.h"
#define LOGGING_ENABLED
#include "../logging.h"
#include <stdbool.h>
#include <stdio.h>
#if !defined(SYSTEM_BASE_WINDOWS)
  #include <sys/ioctl.h>
#endif

#include "../actrie.h"

#ifndef UWUFETCH_VERSION
  #define UWUFETCH_VERSION "unkown" // needs to be changed by the build script
#endif

/*
 * the test consists in just comparing the output to NULL.
 * these kind of functions can not be tested with an expected
 * output as it depends on the environment. Instead I chose to
 * not generate segfaults in the program that uses the library.
 */

#define TEST_FUNCTION(f) bool test_##f(void)
#define TEST_COMPARE_FUNCTION(f, c) \
  TEST_FUNCTION(f) { return f() c; }
#define TEST_STRING_FUNCTION(f) TEST_COMPARE_FUNCTION(f, != NULL)

TEST_STRING_FUNCTION(get_user_name)
TEST_STRING_FUNCTION(get_host_name)
TEST_STRING_FUNCTION(get_shell)
TEST_STRING_FUNCTION(get_model)
TEST_STRING_FUNCTION(get_kernel)
TEST_STRING_FUNCTION(get_os_name)
TEST_STRING_FUNCTION(get_cpu_model)
TEST_STRING_FUNCTION(get_gpus)
TEST_STRING_FUNCTION(get_packages)

TEST_COMPARE_FUNCTION(get_screen_width, > 0)
TEST_COMPARE_FUNCTION(get_screen_height, > 0)
TEST_COMPARE_FUNCTION(get_memory_total, > 0)
TEST_COMPARE_FUNCTION(get_memory_used, > 0)
TEST_COMPARE_FUNCTION(get_uptime, > 0)

TEST_FUNCTION(get_terminal_size) {
  struct winsize ws = get_terminal_size();
  return ws.ws_col > 0 && ws.ws_row > 0 && ws.ws_xpixel > 0 && ws.ws_ypixel > 0;
}

#define STRUCT_TEST(f) \
  { test_##f, #f }
struct test {
  bool (*function)(void);
  const char* name;
};

struct test tests[] = {
    STRUCT_TEST(get_user_name),
    STRUCT_TEST(get_host_name),
    STRUCT_TEST(get_shell),
    STRUCT_TEST(get_model),
    STRUCT_TEST(get_kernel),
    STRUCT_TEST(get_os_name),
    STRUCT_TEST(get_cpu_model),
    STRUCT_TEST(get_gpus),
    STRUCT_TEST(get_packages),
    STRUCT_TEST(get_screen_width),
    STRUCT_TEST(get_screen_height),
    STRUCT_TEST(get_memory_total),
    STRUCT_TEST(get_memory_used),
    STRUCT_TEST(get_uptime),
    STRUCT_TEST(get_terminal_size),
};

struct test_context_t {
  struct vector_string_t found_occurances;
  struct vector_uint32_t start_indexes_in_original_text;
};

void test_run_text_callback(void* context, const char* found_word, size_t word_length, size_t start_index_in_original_text) {
  struct test_context_t* ctx = (struct test_context_t*)context;
  vector_uint32_t_emplace_back1(&(ctx->start_indexes_in_original_text), (uint32_t)start_index_in_original_text);
  vector_string_t_emplace_back2(&(ctx->start_indexes_in_original_text), found_word, word_length);
}

static bool run_test(
    const char* (*patterns_with_replacements)[2],
    size_t patterns_size,
    const char read_text[],
    const char* expected_occurances[],
    const uint32_t expected_occurances_pos[],
    size_t expected_occurances_size,
    char replacing_text[],
    size_t replacing_text_size,
    const char expected_replacing_text[],
    bool replace_all_occurances) {
  struct actrie_t t;
  actrie_t_ctor(&t);

  struct test_context_t context;
  vector_string_t_ctor(&context.found_occurances);
  vector_uint32_t_ctor(&context.start_indexes_in_original_text);

  actrie_t_reserve_patterns(&t, patterns_size);
  for (size_t i = 0; i < patterns_size; i++) {
    actrie_t_add_pattern(&t, patterns_with_replacements[i][0], patterns_with_replacements[i][1]);
  }

  bool result = false;
  for (size_t i = 0; i < patterns_size; i++) {
    if (!actrie_t_contains_pattern(&t, patterns_with_replacements[i][0])) {
      goto cleanup;
    }
  }

  if (actrie_t_patterns_size(&t) != patterns_size) {
    goto cleanup;
  }

  actrie_t_compute_links(&t);
  if (!actrie_t_is_ready(&t)) {
    goto cleanup;
  }

  vector_string_t_reserve(&context.found_occurances, expected_occurances_size);
  vector_uint32_t_reserve(&context.start_indexes_in_original_text, expected_occurances_size);
  actrie_t_run_text(&t, read_text, (void*)&context, &test_run_text_callback);

  if (context.found_occurances.size != context.start_indexes_in_original_text.size || context.found_occurances.size != expected_occurances_size) {
    goto cleanup;
  }

  for (size_t i = 0; i < expected_occurances_size; i++) {
    if (strcmp(expected_occurances[i], context.found_occurances.data[i].c_str) != 0 || expected_occurances_pos[i] != context.start_indexes_in_original_text.data[i]) {
      goto cleanup;
    }
  }

  replacing_text[replacing_text_size] = '\0';
  if (replace_all_occurances) {
    actrie_t_replace_all_occurances(&t, replacing_text);
  } else {
    actrie_t_replace_first_occurance(&t, replacing_text);
  }

  result = (strcmp(replacing_text, expected_replacing_text) == 0);

cleanup:
  vector_uint32_t_dtor(&context.start_indexes_in_original_text);
  vector_string_t_dtor(&context.found_occurances);
  actrie_t_dtor(&t);

  return result;
}

bool test0_short() {
  const char* patterns_with_replacements[][2] = {
      {"ab", "cd"},
      {"ba", "dc"},
      {"aa", "cc"},
      {"bb", "dd"},
      {"fasb", "xfasbx"},
  };
  const size_t patterns_size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

  const char read_text[]            = "abbabbababbbabfabfbfafafbsfabsfasbabbbababbabbaa";
  const char* expected_occurances[] = {
      "ab", "bb", "ba", "ab", "bb",
      "ba", "ab", "ba", "ab", "bb",
      "bb", "ba", "ab", "ab", "ab",
      "fasb", "ba", "ab", "bb", "bb",
      "ba", "ab", "ba", "ab", "bb",
      "ba", "ab", "bb", "ba", "aa"};

  const uint32_t expected_occurances_pos[] = {
      0, 1, 2, 3, 4, 5,
      6, 7, 8, 9, 10, 11,
      12, 15, 27, 30, 33, 34,
      35, 36, 37, 38, 39, 40,
      41, 42, 43, 44, 45, 46};

  const size_t expected_occurances_size = sizeof(expected_occurances) / sizeof(expected_occurances[0]);
  static_assert(expected_occurances_size == sizeof(expected_occurances_pos) / sizeof(expected_occurances_pos[0]));

  char replacing_text[]                = "ababcdacafaasbfasbabcc  ";
  const char expected_replacing_text[] = "cdcdcdacafccsbxfasbxcdcc";
  const size_t replacing_text_size     = 22; // after 'c'

  return run_test(patterns_with_replacements,
                  patterns_size,
                  read_text,
                  expected_occurances,
                  expected_occurances_pos,
                  expected_occurances_size,
                  replacing_text,
                  replacing_text_size,
                  expected_replacing_text,
                  true);
}

bool test1_short() {
  const char* patterns_with_replacements[][2] = {
      {"ab", "cd"},
      {"ba", "dc"},
      {"aa", "cc"},
      {"bb", "dd"},
      {"xfasbx", "fasb"}};
  const size_t patterns_size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

  const char read_text[]            = "abbabbababbbabxfasbxfbfafafbsfabsxfasbxabbbababbabbaa";
  const char* expected_occurances[] = {
      "ab", "bb", "ba", "ab", "bb",
      "ba", "ab", "ba", "ab", "bb",
      "bb", "ba", "ab", "xfasbx", "ab",
      "xfasbx", "ab", "bb", "bb", "ba",
      "ab", "ba", "ab", "bb", "ba",
      "ab", "bb", "ba", "aa"};

  const uint32_t expected_occurances_pos[] = {
      0, 1, 2, 3, 4,
      5, 6, 7, 8, 9,
      10, 11, 12, 14, 30,
      33, 39, 40, 41, 42,
      43, 44, 45, 46, 47,
      48, 49, 50, 51};

  const size_t expected_occurances_size = sizeof(expected_occurances) / sizeof(expected_occurances[0]);
  static_assert(expected_occurances_size == sizeof(expected_occurances_pos) / sizeof(expected_occurances_pos[0]));

  char replacing_text[]                = "ababcdacafaasbxfasbxabcc";
  const char expected_replacing_text[] = "cdcdcdacafccsbfasbcdcc";
  const size_t replacing_text_size     = 24; // 'c'

  return run_test(patterns_with_replacements,
                  patterns_size,
                  read_text,
                  expected_occurances,
                  expected_occurances_pos,
                  expected_occurances_size,
                  replacing_text,
                  replacing_text_size,
                  expected_replacing_text,
                  true);
}

bool test2_short() {
  const char* patterns_with_replacements[][2] = {
      {"LM", "0000"},
      {"GHI", "111111"},
      {"BCD", "2222222"},
      {"nop", "3333"},
      {"jk", "44444"}};
  const size_t patterns_size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

  const char read_text[]                   = "acbcdbgfgacdbsgdegfdjkjdaskjdegcbegfasdcdg";
  const char* expected_occurances[]        = {"bcd", "jk"};
  const uint32_t expected_occurances_pos[] = {2, 20};

  const size_t expected_occurances_size = sizeof(expected_occurances) / sizeof(expected_occurances[0]);
  static_assert(expected_occurances_size == sizeof(expected_occurances_pos) / sizeof(expected_occurances_pos[0]));

  char replacing_text[]                = "ABCDEFGHIJKLMNOP             ";
  const char expected_replacing_text[] = "A2222222EF1111114444400003333";
  const size_t replacing_text_size     = 16; // after 'P'

  return run_test(patterns_with_replacements,
                  patterns_size,
                  read_text,
                  expected_occurances,
                  expected_occurances_pos,
                  expected_occurances_size,
                  replacing_text,
                  replacing_text_size,
                  expected_replacing_text,
                  true);
}

bool test3_short() {
  const char* patterns_with_replacements[][2] = {
      {"AB", "111111111111111111111111"},
      {"CD", "cd"},
      {"EF", "ef"},
      {"JK", "jk"},
      {"NO", "no"}};
  const size_t patterns_size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

  const char read_text[]            = "bacbcbdbabcbdbbfggasdasdncnononocabcbdbcbdcdbacbcbdbcbfgbcdabbcdgdcnpnpnnonoojkcajcjdk";
  const char* expected_occurances[] = {
      "ab", "no", "no", "no", "ab", "cd",
      "cd", "ab", "cd", "no", "no", "jk"};

  const uint32_t expected_occurances_pos[] = {
      8, 26, 28, 30, 33, 42,
      57, 59, 62, 72, 74, 77};

  const size_t expected_occurances_size = sizeof(expected_occurances) / sizeof(expected_occurances[0]);
  static_assert(expected_occurances_size == sizeof(expected_occurances_pos) / sizeof(expected_occurances_pos[0]));

  char replacing_text[]                = "ABCDEFGHIJKLMNOP                      ";
  const char expected_replacing_text[] = "111111111111111111111111cdefGHIjkLMnoP";
  const size_t replacing_text_size     = 16; // after 'P'
  return run_test(patterns_with_replacements,
                  patterns_size,
                  read_text,
                  expected_occurances,
                  expected_occurances_pos,
                  expected_occurances_size,
                  replacing_text,
                  replacing_text_size,
                  expected_replacing_text,
                  true);
}

bool test4_short() {
  const char* patterns_with_replacements[][2] = {
      {"AB", "ab"},
      {"CD", "cd"},
      {"EF", "ef"},
      {"JK", "jk"},
      {"NO", "111111111111111111111111"}};
  const size_t patterns_size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

  const char read_text[]            = "bacbcbdbabcbdbbfggasdasdncnononocabcbdbcbdcdbacbcbdbcbfgbcdabbcdgdcnpnpnnonoojkcajcjdjk";
  const char* expected_occurances[] = {
      "ab", "no", "no", "no", "ab",
      "cd", "cd", "ab", "cd", "no",
      "no", "jk", "jk"};

  const uint32_t expected_occurances_pos[] = {
      8, 26, 28, 30, 33,
      42, 57, 59, 62, 72,
      74, 77, 85};

  const size_t expected_occurances_size = sizeof(expected_occurances) / sizeof(expected_occurances[0]);
  static_assert(expected_occurances_size == sizeof(expected_occurances_pos) / sizeof(expected_occurances_pos[0]));

  char replacing_text[]                = "ABCDEFGHIJKLMNOP                      ";
  const char expected_replacing_text[] = "abcdefGHIjkLM111111111111111111111111P";
  const size_t replacing_text_size     = 16; // after 'P'
  return run_test(patterns_with_replacements,
                  patterns_size,
                  read_text,
                  expected_occurances,
                  expected_occurances_pos,
                  expected_occurances_size,
                  replacing_text,
                  replacing_text_size,
                  expected_replacing_text,
                  true);
}

bool test5_short() {
  const char* patterns_with_replacements[][2] = {
      {"AB", "ab"},
      {"CD", "cd"},
      {"EF", "111111111111111111111111"},
      {"JK", "jk"},
      {"NO", "no"}};
  const size_t patterns_size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

  const char read_text[]            = "bacbcbdbABcbdbbfggasdasdncnononocabcbdbcbdcdbacbcbdbcbfgbcdabbcdgdcnpnpnnonoojkcajcjdJK";
  const char* expected_occurances[] = {
      "AB", "no", "no", "no", "ab",
      "cd", "cd", "ab", "cd", "no",
      "no", "jk", "JK"};

  const uint32_t expected_occurances_pos[] = {
      8, 26, 28, 30, 33,
      42, 57, 59, 62, 72,
      74, 77, 85};

  const size_t expected_occurances_size = sizeof(expected_occurances) / sizeof(expected_occurances[0]);
  static_assert(expected_occurances_size == sizeof(expected_occurances_pos) / sizeof(expected_occurances_pos[0]));

  char replacing_text[]                = "ABCDEFGHIJKLMNOP                      ";
  const char expected_replacing_text[] = "abcd111111111111111111111111GHIjkLMnoP";
  const size_t replacing_text_size     = 16; // after 'P'
  return run_test(patterns_with_replacements,
                  patterns_size,
                  read_text,
                  expected_occurances,
                  expected_occurances_pos,
                  expected_occurances_size,
                  replacing_text,
                  replacing_text_size,
                  expected_replacing_text,
                  true);
}

bool test6_short() {
  const char* patterns_with_replacements[][2] = {
      {"kernel", "Kewnel"},
      {"linux", "Linuwu"},
      {"debian", "Debinyan"},
      {"ubuntu", "Uwuntu"},
      {"windows", "WinyandOwOws"}};
  const size_t patterns_size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

  const char read_text[]                   = "linux kernel; debian os; ubuntu os; windows os";
  const char* expected_occurances[]        = {"linux", "kernel", "debian", "ubuntu", "windows"};
  const uint32_t expected_occurances_pos[] = {0, 6, 14, 25, 36};

  const size_t expected_occurances_size = sizeof(expected_occurances) / sizeof(expected_occurances[0]);
  static_assert(expected_occurances_size == sizeof(expected_occurances_pos) / sizeof(expected_occurances_pos[0]));

  char replacing_text[]                = "linux kernel; debian os; ubuntu os; windows os        ";
  const char expected_replacing_text[] = "Linuwu Kewnel; Debinyan os; Uwuntu os; WinyandOwOws os";
  const size_t replacing_text_size     = 46; // after 's'
  return run_test(patterns_with_replacements,
                  patterns_size,
                  read_text,
                  expected_occurances,
                  expected_occurances_pos,
                  expected_occurances_size,
                  replacing_text,
                  replacing_text_size,
                  expected_replacing_text,
                  true);
}

bool test7_long() {
  const char* patterns_with_replacements[][2] = {
      {"brew-cask", "bwew-cawsk"},
      {"brew-cellar", "bwew-cewwaw"},
      {"emerge", "emewge"},
      {"flatpak", "fwatpakkies"},
      {"pacman", "pacnyan"},
      {"port", "powt"},
      {"rpm", "rawrpm"},
      {"snap", "snyap"},
      {"zypper", "zyppew"},

      {"lenovo", "LenOwO"},
      {"cpu", "CPUwU"},
      {"core", "Cowe"},
      {"gpu", "GPUwU"},
      {"graphics", "Gwaphics"},
      {"corporation", "COwOpowation"},
      {"nvidia", "NyaVIDIA"},
      {"mobile", "Mwobile"},
      {"intel", "Inteww"},
      {"radeon", "Radenyan"},
      {"geforce", "GeFOwOce"},
      {"raspberry", "Nyasberry"},
      {"broadcom", "Bwoadcom"},
      {"motorola", "MotOwOwa"},
      {"proliant", "ProLinyant"},
      {"poweredge", "POwOwEdge"},
      {"apple", "Nyapple"},
      {"electronic", "ElectrOwOnic"},
      {"processor", "Pwocessow"},
      {"microsoft", "MicOwOsoft"},
      {"ryzen", "Wyzen"},
      {"advanced", "Adwanced"},
      {"micro", "Micwo"},
      {"devices", "Dewices"},
      {"inc.", "Nyanc."},
      {"lucienne", "Lucienyan"},
      {"tuxedo", "TUWUXEDO"},
      {"aura", "Uwura"},

      {"linux", "linuwu"},
      {"alpine", "Nyalpine"},
      {"amogos", "AmogOwOS"},
      {"android", "Nyandroid"},
      {"arch", "Nyarch Linuwu"},

      {"arcolinux", "ArcOwO Linuwu"},

      {"artix", "Nyartix Linuwu"},
      {"debian", "Debinyan"},

      {"devuan", "Devunyan"},

      {"deepin", "Dewepyn"},
      {"endeavouros", "endeavOwO"},
      {"fedora", "Fedowa"},
      {"femboyos", "FemboyOWOS"},
      {"gentoo", "GentOwO"},
      {"gnu", "gnUwU"},
      {"guix", "gnUwU gUwUix"},
      {"linuxmint", "LinUWU Miwint"},
      {"manjaro", "Myanjawo"},
      {"manjaro-arm", "Myanjawo AWM"},
      {"neon", "KDE NeOwOn"},
      {"nixos", "nixOwOs"},
      {"opensuse-leap", "OwOpenSUSE Leap"},
      {"opensuse-tumbleweed", "OwOpenSUSE Tumbleweed"},
      {"pop", "PopOwOS"},
      {"raspbian", "RaspNyan"},
      {"rocky", "Wocky Linuwu"},
      {"slackware", "Swackwawe"},
      {"solus", "sOwOlus"},
      {"ubuntu", "Uwuntu"},
      {"void", "OwOid"},
      {"xerolinux", "xuwulinux"},

      // BSD
      {"freebsd", "FweeBSD"},
      {"openbsd", "OwOpenBSD"},

      // Apple family
      {"macos", "macOwOS"},
      {"ios", "iOwOS"},

      // Windows
      {"windows", "WinyandOwOws"},
  };
  const size_t patterns_size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

  const char read_text[]            = "windows freebsd rocky; neon linux; fedora; pop os; solus; amogos; void; ryzen and intel processor";
  const char* expected_occurances[] = {
      "windows", "freebsd", "rocky", "neon", "linux", "fedora", "pop",
      "solus", "amogos", "void", "ryzen", "intel", "processor"};

  const uint32_t expected_occurances_pos[] = {
      0,
      8,
      16,
      23,
      28,
      35,
      43,
      51,
      58,
      66,
      72,
      82,
      88,
  };

  const size_t expected_occurances_size = sizeof(expected_occurances) / sizeof(expected_occurances[0]);
  static_assert(expected_occurances_size == sizeof(expected_occurances_pos) / sizeof(expected_occurances_pos[0]));

  char replacing_text[]                = "windows freebsd rocky; neon linux; fedora; pop os; solus; amogos; void; ryzen and intel processor                             ";
  const char expected_replacing_text[] = "WinyandOwOws FweeBSD Wocky Linuwu; KDE NeOwOn linuwu; Fedowa; PopOwOS os; sOwOlus; AmogOwOS; OwOid; Wyzen and Inteww Pwocessow";
  const size_t replacing_text_size     = 97; // after 'r'

  return run_test(patterns_with_replacements,
                  patterns_size,
                  read_text,
                  expected_occurances,
                  expected_occurances_pos,
                  expected_occurances_size,
                  replacing_text,
                  replacing_text_size,
                  expected_replacing_text,
                  true);
}

bool test8_long() {
  const char* patterns_with_replacements[][2] = {
      {"brew-cask", "bwew-cawsk"},
      {"brew-cellar", "bwew-cewwaw"},
      {"emerge", "emewge"},
      {"flatpak", "fwatpakkies"},
      {"pacman", "pacnyan"},
      {"port", "powt"},
      {"rpm", "rawrpm"},
      {"snap", "snyap"},
      {"zypper", "zyppew"},

      {"lenovo", "LenOwO"},
      {"cpu", "CPUwU"},
      {"core", "Cowe"},
      {"gpu", "GPUwU"},
      {"graphics", "Gwaphics"},
      {"corporation", "COwOpowation"},
      {"nvidia", "NyaVIDIA"},
      {"mobile", "Mwobile"},
      {"intel", "Inteww"},
      {"radeon", "Radenyan"},
      {"geforce", "GeFOwOce"},
      {"raspberry", "Nyasberry"},
      {"broadcom", "Bwoadcom"},
      {"motorola", "MotOwOwa"},
      {"proliant", "ProLinyant"},
      {"poweredge", "POwOwEdge"},
      {"apple", "Nyapple"},
      {"electronic", "ElectrOwOnic"},
      {"processor", "Pwocessow"},
      {"microsoft", "MicOwOsoft"},
      {"ryzen", "Wyzen"},
      {"advanced", "Adwanced"},
      {"micro", "Micwo"},
      {"devices", "Dewices"},
      {"inc.", "Nyanc."},
      {"lucienne", "Lucienyan"},
      {"tuxedo", "TUWUXEDO"},
      {"aura", "Uwura"},

      {"linux", "linuwu"},
      {"alpine", "Nyalpine"},
      {"amogos", "AmogOwOS"},
      {"android", "Nyandroid"},
      {"arch", "Nyarch Linuwu"},

      {"arcolinux", "ArcOwO Linuwu"},

      {"artix", "Nyartix Linuwu"},
      {"debian", "Debinyan"},

      {"devuan", "Devunyan"},

      {"deepin", "Dewepyn"},
      {"endeavouros", "endeavOwO"},
      {"fedora", "Fedowa"},
      {"femboyos", "FemboyOWOS"},
      {"gentoo", "GentOwO"},
      {"gnu", "gnUwU"},
      {"guix", "gnUwU gUwUix"},
      {"linuxmint", "LinUWU Miwint"},
      {"manjaro", "Myanjawo"},
      {"manjaro-arm", "Myanjawo AWM"},
      {"neon", "KDE NeOwOn"},
      {"nixos", "nixOwOs"},
      {"opensuse-leap", "OwOpenSUSE Leap"},
      {"opensuse-tumbleweed", "OwOpenSUSE Tumbleweed"},
      {"pop", "PopOwOS"},
      {"raspbian", "RaspNyan"},
      {"rocky", "Wocky Linuwu"},
      {"slackware", "Swackwawe"},
      {"solus", "sOwOlus"},
      {"ubuntu", "Uwuntu"},
      {"void", "OwOid"},
      {"xerolinux", "xuwulinux"},

      // BSD
      {"freebsd", "FweeBSD"},
      {"openbsd", "OwOpenBSD"},

      // Apple family
      {"macos", "macOwOS"},
      {"ios", "iOwOS"},

      // Windows
      {"windows", "WinyandOwOws"},
  };
  const size_t patterns_size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

  char replacing_text[]                = "windows FREEBSD ROCKy; NEON linux; FEDORA; pop os; solus; amogos; Void; ryzEN and Intel PROCessor                             ";
  const char expected_replacing_text[] = "WinyandOwOws FweeBSD Wocky Linuwu; KDE NeOwOn linuwu; Fedowa; PopOwOS os; sOwOlus; AmogOwOS; OwOid; Wyzen and Inteww Pwocessow";
  const size_t replacing_text_size     = 97; // after 'r'

  const char read_text[]            = "windows FREEBSD ROCKy; NEON linux; FEDORA; pop os; solus; amogos; Void; ryzEN and Intel PROCessor";
  const char* expected_occurances[] = {
      "windows", "FREEBSD", "ROCKy", "NEON", "linux", "FEDORA", "pop",
      "solus", "amogos", "Void", "ryzEN", "Intel", "PROCessor"};

  const uint32_t expected_occurances_pos[] = {
      0,
      8,
      16,
      23,
      28,
      35,
      43,
      51,
      58,
      66,
      72,
      82,
      88,
  };

  const size_t expected_occurances_size = sizeof(expected_occurances) / sizeof(expected_occurances[0]);
  static_assert(expected_occurances_size == sizeof(expected_occurances_pos) / sizeof(expected_occurances_pos[0]));

  return run_test(patterns_with_replacements,
                  patterns_size,
                  read_text,
                  expected_occurances,
                  expected_occurances_pos,
                  expected_occurances_size,
                  replacing_text,
                  replacing_text_size,
                  expected_replacing_text,
                  true);
}

bool test9_short() {
  const char* patterns_with_replacements[][2] = {
      {"abc", "def"},
      {"ghi", "jkz"},
  };
  const size_t patterns_size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

  const char read_text[]                   = "cabcbacbbcabacbacbbacbachggiuichaihacihaichacbacbbcachachgighighighbcbaa";
  const char* expected_occurances[]        = {"abc", "ghi", "ghi"};
  const uint32_t expected_occurances_pos[] = {1, 59, 62};

  const size_t expected_occurances_size = sizeof(expected_occurances) / sizeof(expected_occurances[0]);
  static_assert(expected_occurances_size == sizeof(expected_occurances_pos) / sizeof(expected_occurances_pos[0]));

  char replacing_text[]                = "Abghciashjdhwdjahwdjhabdabanabwc";
  const char expected_replacing_text[] = "Abghciashjdhwdjahwdjhabdabanabwc";
  const size_t replacing_text_size     = 32; // after 'c'

  return run_test(patterns_with_replacements,
                  patterns_size,
                  read_text,
                  expected_occurances,
                  expected_occurances_pos,
                  expected_occurances_size,
                  replacing_text,
                  replacing_text_size,
                  expected_replacing_text,
                  true);
}

bool test10_short() {
  const char* patterns_with_replacements[][2] = {
      {"abc", "def"},
      {"ghi", "jkz"},
  };
  const size_t patterns_size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

  const char read_text[]                   = "cAbcbacbbcabacbacbbacbachggiuichaihacihaichacbacbbcachachgighiGhighbcbaa";
  const char* expected_occurances[]        = {"Abc", "ghi", "Ghi"};
  const uint32_t expected_occurances_pos[] = {1, 59, 62};

  const size_t expected_occurances_size = sizeof(expected_occurances) / sizeof(expected_occurances[0]);
  static_assert(expected_occurances_size == sizeof(expected_occurances_pos) / sizeof(expected_occurances_pos[0]));

  char replacing_text[]                = "Qghiabcabcghiabc";
  const char expected_replacing_text[] = "Qjkzabcabcghiabc";
  const size_t replacing_text_size     = 16; // after 'c'

  return run_test(patterns_with_replacements,
                  patterns_size,
                  read_text,
                  expected_occurances,
                  expected_occurances_pos,
                  expected_occurances_size,
                  replacing_text,
                  replacing_text_size,
                  expected_replacing_text,
                  false);
}

static void test_actrie(int* passed_tests, int* failed_tests) {
  bool (*test_functions[])() = {
      &test0_short,
      &test1_short,
      &test2_short,
      &test3_short,
      &test4_short,
      &test5_short,
      &test6_short,
      &test7_long,
      &test8_long,
      &test9_short,
      &test10_short,
  };

  for (size_t i = 0; i < sizeof(test_functions) / sizeof(test_functions[0]); i++) {
    if (test_functions[i]()) {
      LOG_I("AC trie test %zu <g>PASSED</>", (i + 1));
      (*passed_tests)++;
    } else {
      LOG_E("AC trie test %zu <r>FAILED</>", (i + 1));
      (*failed_tests)++;
    }
  }
}

int main(int argc, char** argv) {
  SET_LOG_LEVEL(LEVEL_MAX, "TESTS");
  if (argc > 1) {
    if (strcmp(argv[1], "-v") == 0) {
      SET_LIBFETCH_LOG_LEVEL(LEVEL_MAX);
    } else if (strcmp(argv[1], "-h") == 0) {
      LOG_I("uwufetch testing version %s. To enable verbose libfetch logging use -v", UWUFETCH_VERSION);
      return 0;
    }
  }
  libfetch_init();
  LOG_W("Starting tests...");
  int passed_tests = 0;
  int failed_tests = 0;
  for (size_t i = 0; i < sizeof(tests) / sizeof(struct test); i++) {
    if (tests[i].function()) {
      LOG_I("%s <g>PASSED</>", tests[i].name);
      passed_tests++;
    } else {
      LOG_E("%s <r>FAILED</>", tests[i].name);
      failed_tests++;
    }
  }
  test_actrie(&passed_tests, &failed_tests);
  LOG_I("\n%d tests <g>passed</>, %d <r>failed</>", passed_tests, failed_tests);
  libfetch_cleanup();
  return 0;
}
