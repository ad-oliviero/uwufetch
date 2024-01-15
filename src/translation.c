#define _GNU_SOURCE // for strcasestr

#include "actrie.h"
#include "libfetch/logging.h"
#include "uwufetch.h"

// uwufies distro name
void uwu_name(const struct actrie_t* replacer, struct info* user_info) {
  char* os_name = user_info->os_name;
  if (os_name == NULL) {
    return;
  }

  if (*os_name == '\0') {
    sprintf(os_name, "%s", "unknown");
    return;
  }

  actrie_t_replace_all_occurances(replacer, os_name);
}

// uwufies kernel name
void uwu_kernel(const struct actrie_t* replacer, char* kernel) {
  LOG_I("uwufing kernel");
  actrie_t_replace_all_occurances(replacer, kernel);
  LOG_V(kernel);
}

// uwufies hardware names
void uwu_hw(const struct actrie_t* replacer, char* hwname) {
  LOG_I("uwufing hardware")
  actrie_t_replace_all_occurances(replacer, hwname);
}

// uwufies package manager names
void uwu_pkgman(const struct actrie_t* replacer, char* pkgman_name) {
  LOG_I("uwufing package managers");
  actrie_t_replace_all_occurances(replacer, pkgman_name);
}

// uwufies everything
void uwufy_all(struct info* user_info) {
  struct actrie_t replacer;
  actrie_t_ctor(&replacer);

  const size_t PATTERNS_COUNT = 73;
  // Not necessary but recommended
  actrie_t_reserve_patterns(&replacer, PATTERNS_COUNT);

  // these package managers do not have edits yet:
  // apk, apt, guix, nix, pkg, xbps
  actrie_t_add_pattern(&replacer, "brew-cask", "bwew-cawsk");
  actrie_t_add_pattern(&replacer, "brew-cellar", "bwew-cewwaw");
  actrie_t_add_pattern(&replacer, "emerge", "emewge");
  actrie_t_add_pattern(&replacer, "flatpak", "fwatpakkies");
  actrie_t_add_pattern(&replacer, "pacman", "pacnyan");
  actrie_t_add_pattern(&replacer, "port", "powt");
  actrie_t_add_pattern(&replacer, "rpm", "rawrpm");
  actrie_t_add_pattern(&replacer, "snap", "snyap");
  actrie_t_add_pattern(&replacer, "zypper", "zyppew");

  actrie_t_add_pattern(&replacer, "lenovo", "LenOwO");
  actrie_t_add_pattern(&replacer, "cpu", "CPUwU");
  actrie_t_add_pattern(&replacer, "core", "Cowe");
  actrie_t_add_pattern(&replacer, "gpu", "GPUwU");
  actrie_t_add_pattern(&replacer, "graphics", "Gwaphics");
  actrie_t_add_pattern(&replacer, "corporation", "COwOpowation");
  actrie_t_add_pattern(&replacer, "nvidia", "NyaVIDIA");
  actrie_t_add_pattern(&replacer, "mobile", "Mwobile");
  actrie_t_add_pattern(&replacer, "intel", "Inteww");
  actrie_t_add_pattern(&replacer, "radeon", "Radenyan");
  actrie_t_add_pattern(&replacer, "geforce", "GeFOwOce");
  actrie_t_add_pattern(&replacer, "raspberry", "Nyasberry");
  actrie_t_add_pattern(&replacer, "broadcom", "Bwoadcom");
  actrie_t_add_pattern(&replacer, "motorola", "MotOwOwa");
  actrie_t_add_pattern(&replacer, "proliant", "ProLinyant");
  actrie_t_add_pattern(&replacer, "poweredge", "POwOwEdge");
  actrie_t_add_pattern(&replacer, "apple", "Nyapple");
  actrie_t_add_pattern(&replacer, "electronic", "ElectrOwOnic");
  actrie_t_add_pattern(&replacer, "processor", "Pwocessow");
  actrie_t_add_pattern(&replacer, "microsoft", "MicOwOsoft");
  actrie_t_add_pattern(&replacer, "ryzen", "Wyzen");
  actrie_t_add_pattern(&replacer, "advanced", "Adwanced");
  actrie_t_add_pattern(&replacer, "micro", "Micwo");
  actrie_t_add_pattern(&replacer, "devices", "Dewices");
  actrie_t_add_pattern(&replacer, "inc.", "Nyanc.");
  actrie_t_add_pattern(&replacer, "lucienne", "Lucienyan");
  actrie_t_add_pattern(&replacer, "tuxedo", "TUWUXEDO");
  actrie_t_add_pattern(&replacer, "aura", "Uwura");

  actrie_t_add_pattern(&replacer, "linux", "Linuwu");
  actrie_t_add_pattern(&replacer, "alpine", "Nyalpine");
  actrie_t_add_pattern(&replacer, "amogos", "AmogOwOS");
  actrie_t_add_pattern(&replacer, "android", "Nyandroid");
  actrie_t_add_pattern(&replacer, "arch", "Nyarch Linuwu");

  actrie_t_add_pattern(&replacer, "arcolinux", "ArcOwO Linuwu");

  actrie_t_add_pattern(&replacer, "artix", "Nyartix Linuwu");
  actrie_t_add_pattern(&replacer, "debian", "Debinyan");

  actrie_t_add_pattern(&replacer, "devuan", "Devunyan");

  actrie_t_add_pattern(&replacer, "deepin", "Dewepyn");
  actrie_t_add_pattern(&replacer, "endeavouros", "endeavOwO");
  actrie_t_add_pattern(&replacer, "fedora", "Fedowa");
  actrie_t_add_pattern(&replacer, "femboyos", "FemboyOWOS");
  actrie_t_add_pattern(&replacer, "gentoo", "GentOwO");
  actrie_t_add_pattern(&replacer, "gnu", "gnUwU");
  actrie_t_add_pattern(&replacer, "guix", "gnUwU gUwUix");
  actrie_t_add_pattern(&replacer, "linuxmint", "LinUWU Miwint");
  actrie_t_add_pattern(&replacer, "manjaro", "Myanjawo");
  actrie_t_add_pattern(&replacer, "manjaro-arm", "Myanjawo AWM");
  actrie_t_add_pattern(&replacer, "neon", "KDE NeOwOn");
  actrie_t_add_pattern(&replacer, "nixos", "nixOwOs");
  actrie_t_add_pattern(&replacer, "opensuse-leap", "OwOpenSUSE Leap");
  actrie_t_add_pattern(&replacer, "opensuse-tumbleweed", "OwOpenSUSE Tumbleweed");
  actrie_t_add_pattern(&replacer, "pop", "PopOwOS");
  actrie_t_add_pattern(&replacer, "raspbian", "RaspNyan");
  actrie_t_add_pattern(&replacer, "rocky", "Wocky Linuwu");
  actrie_t_add_pattern(&replacer, "slackware", "Swackwawe");
  actrie_t_add_pattern(&replacer, "solus", "sOwOlus");
  actrie_t_add_pattern(&replacer, "ubuntu", "Uwuntu");
  actrie_t_add_pattern(&replacer, "void", "OwOid");
  actrie_t_add_pattern(&replacer, "xerolinux", "xuwulinux");

  // BSD
  actrie_t_add_pattern(&replacer, "freebsd", "FweeBSD");
  actrie_t_add_pattern(&replacer, "openbsd", "OwOpenBSD");

  // Apple family
  actrie_t_add_pattern(&replacer, "macos", "macOwOS");
  actrie_t_add_pattern(&replacer, "ios", "iOwOS");

  // Windows
  actrie_t_add_pattern(&replacer, "windows", "WinyandOwOws");

  actrie_t_compute_links(&replacer);

  LOG_I("uwufing everything");
  if (user_info->kernel) uwu_kernel(&replacer, user_info->kernel);
  if (user_info->gpu_list)
    for (size_t i = 1; i <= (size_t)user_info->gpu_list[0]; i++)
      if (user_info->gpu_list[i])
        uwu_hw(&replacer, user_info->gpu_list[i]);

  if (user_info->cpu)
    uwu_hw(&replacer, user_info->cpu);
  LOG_V(user_info->cpu);

  if (user_info->model)
    uwu_hw(&replacer, user_info->model);
  LOG_V(user_info->model);

  if (user_info->packages)
    uwu_pkgman(&replacer, user_info->packages);
  LOG_V(user_info->packages);

  uwu_name(&replacer, user_info);

  actrie_t_dtor(&replacer);
}
