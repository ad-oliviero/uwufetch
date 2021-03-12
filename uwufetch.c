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

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

// COLORS
#define NORMAL "\x1b[0m"
#define BOLD "\x1b[1m"
#define BLACK "\x1b[30m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[0;35m"
#define CYAN "\x1b[36m"
#define WHITE "\x1b[37m"
#define PINK "\x1b[38;5;201m"
#define LPINK "\x1b[38;5;213m"

struct utsname sys_var;
struct sysinfo sys;
int ram_max = 0, ram_free = 0, pkgs, a_i_flag = 0;
char user[32], host[256], shell[64], version_name[64], cpu_model[256], pkgman_name[64], image_name[32];
int pkgman();
void get_info();
void list();
void print_ascii();
void print_info();
void print_image();
void usage(char*);
void uwu_name();

int main(int argc, char *argv[]) {
	int opt = 0;
	get_info();
	while((opt = getopt(argc, argv, "ad:hilc:")) != -1) {
		switch(opt) {
			case 'a':
				a_i_flag = 0;
				break;
			case 'c':
				a_i_flag = 1;
				sprintf(image_name, "%s", optarg);
				break;
			case 'd':
				if (optarg) sprintf(version_name, "%s", optarg);
				break;
			case 'h':
				usage(argv[0]);
				return 0;
			case 'i':
				a_i_flag = 1;
				break;
			case 'l':
				list(argv[0]);
				return 0;
			default:
				break;
		}
	}
	if (argc == 1 || a_i_flag == 0) print_ascii();
	else if (a_i_flag) print_image();
	uwu_name();
	print_info();
}

int pkgman() { // this is just a function that returns the total of installed packages
	int apt, dnf, emerge, flatpak, nix, pacman, rpm, xbps, total = 0;

	FILE *file[8];
	file[0] = popen("dpkg-query -f '${binary:Package}\n' -W 2> /dev/null | wc -l", "r");
	file[1] = popen("dnf list installed 2> /dev/null | wc -l", "r");
	file[2] = popen("qlist -I 2> /dev/null | wc -l", "r");
	file[3] = popen("flatpak list 2> /dev/null | wc -l", "r");
	file[4] = popen("nix-store -q --requisites /run/current-sys_vartem/sw 2> /dev/null | wc -l", "r");
	file[5] = popen("pacman -Qq 2> /dev/null | wc -l", "r");
	file[6] = popen("rpm -qa --last 2> /dev/null | wc -l", "r");
	file[7] = popen("xbps-query -l 2> /dev/null | wc -l", "r");

	fscanf(file[0], "%d", &apt);
	fscanf(file[1], "%d", &dnf);
	fscanf(file[2], "%d", &emerge);
	fscanf(file[3], "%d", &flatpak);
	fscanf(file[4], "%d", &nix);
	fscanf(file[5], "%d", &pacman);
	fscanf(file[6], "%d", &rpm);
	fscanf(file[7], "%d", &xbps);
	for (int i = 0; i < 8; i++) fclose(file[i]);

	#define ADD_PACKAGES(package_count, pkgman_to_add) if (package_count > 0) { total += package_count; strcat(pkgman_name, pkgman_to_add); }
		ADD_PACKAGES(apt,    "(apt)")
		ADD_PACKAGES(dnf,    "(dnf)")
		ADD_PACKAGES(emerge, "(emerge)")
		ADD_PACKAGES(flatpak,"(flatpak)")
		ADD_PACKAGES(nix,    "(nix)")
		ADD_PACKAGES(pacman, "(pacman)")
		ADD_PACKAGES(rpm,    "(rpm)")
		ADD_PACKAGES(xbps,   "(xbps)")
	#undef ADD_PACKAGES

	return total;	
}

void print_info() {	// print collected info
	printf(	"\033[9A\033[18C%s%s%s@%s\n"
			"\033[18C%s%sOWOS     %s%s\n"
			"\033[18C%s%sKERNEL   %s%s %s\n"
			"\033[18C%s%sCPUWU    %s%s\n"
			"\033[18C%s%sWAM      %s%i MB/%i MB\n"
			"\033[18C%s%sSHELL    %s%s\n"
			"\033[18C%s%sPKGS     %s%s%d %s\n"
			"\033[18C%s%sUWUPTIME %s"/*"%lid, "*/"%lih, %lim\n"
			"\033[18C%s%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\n",
			NORMAL, BOLD, user, host,
			NORMAL, BOLD, NORMAL, version_name,
			NORMAL, BOLD, NORMAL, sys_var.release, sys_var.machine,
			NORMAL, BOLD, NORMAL, cpu_model,
			NORMAL, BOLD, NORMAL, (ram_max - ram_free), ram_max,
			NORMAL, BOLD, NORMAL, shell,
			NORMAL, BOLD, NORMAL, NORMAL, pkgs, pkgman_name,
			NORMAL, BOLD, NORMAL, /*sys.uptime/60/60/24,*/ sys.uptime/60/60, sys.uptime/60%60,
			BOLD, BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN,  WHITE, NORMAL);
}

void get_info() {	// get all necessary info
	char line[256];	// var to scan file lines
	// os version
	FILE *os_release = fopen("/etc/os-release", "r");
	FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
	if (os_release) {	// get normal vars
		while (fgets(line, sizeof(line), os_release)) {
			sscanf(line, "\nID=%s", version_name);
			if (strlen(version_name)) break;
		}
		while (fgets(line, sizeof(line), cpuinfo)) if (fscanf(cpuinfo, "model name      : %[^with\n]", cpu_model)) break;
		sprintf(user, "%s", getenv("USER"));
		fclose(os_release);
	} else {	// try for android vars, or unknown system
		DIR *system_app = opendir("/system/app/");
		DIR *system_priv_app = opendir("/system/priv-app/");
		if (system_app && system_priv_app) {	// android
			closedir(system_app);
			closedir(system_priv_app);
			sprintf(version_name, "android");
			// android vars
			FILE *whoami = popen("whoami", "r");
			fscanf(whoami, "%s", user);
			fclose(whoami);
			while (fgets(line, sizeof(line), cpuinfo)) if (fscanf(cpuinfo, "Hardware        : %[^\n]", cpu_model)) break;
		} else sprintf(version_name, "unknown");
	}
	fclose(cpuinfo);
	gethostname(host, 256);
	sscanf(getenv("SHELL"), "%*[bin/]%s", shell);

	// system resources
	uname(&sys_var);
	sysinfo(&sys);
	
	// ram
	FILE *meminfo = fopen("/proc/meminfo", "r");
	while (fgets(line, sizeof(line), meminfo)) {
		sscanf(line, "MemFree: %d kB", &ram_free);
		sscanf(line, "MemTotal: %d kB", &ram_max);
		if (ram_max > 0 && ram_free > 0) {
			ram_max *= 0.001024;
			ram_free *= 0.001024;
			fclose(meminfo);
			break;
		}
	}
	pkgs = pkgman();
}

void list(char* arg) {	// prints distribution list
	/*	distributions are listed by distribution branch
		to make the output easier to understand by the user.*/
	printf(	"%s -d <options>\n"
			"  Available distributions:\n"
			"    %sArch linux %sbased:\n"
			"      %sarch, artix, %smanjaro, \"manjaro-arm\"\n\n"
			"    %sDebian/%sUbuntu %sbased:\n"
			"      %sdebian, %slinuxmint, %spopos\n\n"
			"    %sOther/spare distributions:\n"
			"      %sfedora, %sgentoo, %svoid, android, %sunknown\n\n"
			"    %sBSD:\n"
			"      freebsd, %sopenbsd\n",
			arg, BLUE, NORMAL, BLUE, GREEN,			// Arch based colors
			RED, YELLOW, NORMAL, RED, GREEN, BLUE,	// Debian based colors
			NORMAL, BLUE, PINK, GREEN, WHITE,		// Other/spare distributions colors
			RED, YELLOW);							// BSD colors
}

void print_ascii() {	// prints logo (as ascii art) of the given system. distributions listed alphabetically.
	
	// linux
	if (strcmp(version_name, "arch") == 0) {
		printf(	"\033[1E\033[8C%s/\\\n"
				"       /  \\\n"
				"      /\\   \\\n"
				"     / > w <\\\n"
				"    /   __   \\\n"
				"   / __|  |__-\\\n"
				"  /_-''    ''-_\\\n\n", BLUE);
	} else if (strcmp(version_name, "artix") == 0) {
		printf(	"\033[1E\033[8C%s/\\\n"
				"       /  \\\n"
				"      /`'.,\\\n"
				"     /\u2022 w \u2022 \\\n"
				"    /      ,`\\\n"
				"   /   ,.'`.  \\\n"
				"  /.,'`     `'.\\\n\n", BLUE);
	} else if (strcmp(version_name, "debian") == 0) {
		printf(	"\033[1E\033[6C%s______\n"  
				"     /  ___ \\\n"
				"    |  / OwO |\n"
				"    |  \\____-\n"
				"    -_\n"
				"      --_\n\n\n", RED);
	} else if (strcmp(version_name, "fedora") == 0) {
		printf(	"\033[1E\033[8C%s_____\n"
				"       /   __)%s\\\n"
				"     %s> %s|  / %s<%s\\ \\\n"
				"    __%s_| %sw%s|_%s_/ /\n"
				"   / %s(_    _)%s_/\n"
				"  / /  %s|  |\n"
				"  %s\\ \\%s__/  |\n"
				"   %s\\%s(_____/\n", BLUE, CYAN, WHITE, BLUE, WHITE, CYAN, BLUE, CYAN, BLUE, CYAN, BLUE, CYAN, BLUE, CYAN, BLUE, CYAN, BLUE);
	} else if (strcmp(version_name, "gentoo") == 0) {
		printf(	"\033[1E\033[3C%s_-----_\n"
				"  (       \\\n"
				"  \\   OwO   \\\n"
				"%s   \\         )\n"
				"   /       _/\n"
				"  (      _-\n"
				"  \\____-\n\n", MAGENTA, WHITE);
	} else if (strcmp(version_name, "manjaro") == 0) {
		printf(	"\033[0E\033[1C\u25b3       \u25b3   \u25e0\u25e0\u25e0\u25e0\n"
				" \e[0;42m          \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m \e[0m\e[0;42m\e[1;30m > w < \e[0m\e[0;42m  \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m        \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n");
	} else if (strcmp(version_name, "\"manjaro-arm\"") == 0) {
		printf(	"\033[0E\033[1C\u25b3       \u25b3   \u25e0\u25e0\u25e0\u25e0\n"
				" \e[0;42m          \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m \e[0m\e[0;42m\e[1;30m > w < \e[0m\e[0;42m  \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m        \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n");
	} else if (strcasecmp(version_name, "linuxmint") == 0) {
		printf( "\033[2E\033[4C%s__/\\____/\\.\n"
  				"   |%s.--.      %s|\n"
 				"  %s, %s¯| %s| UwU| %s|\n"
 				" %s||  %s| %s|    | %s|\n"
 				" %s |  %s|  %s----  %s|\n"
 				" %s  --%s'--------'\n\n",GREEN, WHITE, GREEN, WHITE, GREEN, WHITE, GREEN, WHITE, GREEN, WHITE, GREEN, WHITE, GREEN, WHITE, GREEN, WHITE, GREEN);
	} else if (strcasecmp(version_name, "popos") == 0) {
		printf("\033[2E\033[6C%s|\\.-----./|\n"
     			"      |/       \\|\n"
     			"      |  >   <  |\n"
     			"      | %s~  %sP! %s~ %s|\n"
				"_   ---\\   w   /\n"
				" \\_/    '-----'\n\n", BLUE, LPINK, WHITE, LPINK, BLUE);  
	} else if (strcmp(version_name, "ubuntu") == 0) {
		printf(	"\033[1E\033[9C%s_\n"
				"     %s\u25E3%s__(_)%s\u25E2%s\n"
				"   _/  ---  \\\n"
				"  (_) |>w<| |\n"
				"    \\  --- _/\n"
				"  %sC__/%s---(_)\n\n\n", LPINK, PINK, LPINK, PINK, LPINK, PINK, LPINK);
	} else if (strcmp(version_name, "void") == 0){
		printf("\033[2E\033[2C%s |\\_____/|\n"
			"  _\\____   |\n" 
			" | \\    \\  |\n"
			" | | %s\u00d2w\u00d3 %s| |     ,\n"   
			" | \\_____\\_|-,  |\n"
 			" -_______\\    \\_/\n\n", GREEN, WHITE, GREEN);
	} else if (strcmp(version_name, "android") == 0) {	// android at the end because it could be not considered as an actual distribution of gnu/linux
		printf(	"\033[2E\033[3C%s\\ _------_ /\n"
			"   /          \\\n"
			"  | %s~ %s> w < %s~  %s|\n"
			"   ------------\n\n\n\n", GREEN, RED, GREEN, RED, GREEN);

	}

	// BSD
	else if (strcmp(version_name, "freebsd") == 0) {
		printf(	"\032[1E\033[3C%s\n"
				" /\\,-'''''-,/\\\n"
				" \\_)       (_/\n"
				" |   \\   /   |\n"
				" |   O w O   |\n"
				"  ;         ;\n"
				"   '-_____-'\n", RED);

	} else if (strcmp(version_name, "openbsd") == 0) {
		printf(	"\033[1E\033[3C%s  ______  \n"
				"   \\-      -/  %s\u2665  \n"
				"%s\\_/          \\  \n"
				"|        %s>  < %s|   \n"
				"|_  <  %s//  %sW %s//   \n"
				"%s/ \\          /   \n"
				"  /-________-\\   \n\n", YELLOW, RED, YELLOW, WHITE, YELLOW, LPINK, WHITE, LPINK, YELLOW);

 	}
	else printf(	"\033[0E\033[4C%s.--.\n"
				"   |o_o |\n"
				"   |:_/ |\n"
				"  //    \\ \\\n"
				" (|      | )\n"
				" %s/'\\_   _/`\\\n"
				" \\___)=(___/\n\n", WHITE, YELLOW);
}

void print_image() {	// prints logo (as an image) of the given system. distributions listed alphabetically.
	char command[256];
	if (strlen(image_name) > 1) sprintf(command, "viu -t -w 18 -h 8 %s 2> /dev/null", image_name);
	else {
		if (strcmp(version_name, "android") == 0) sprintf(command, "viu -t -w 18 -h 8 /data/data/com.termux/files/usr/lib/uwufetch/%s.png 2> /dev/null", version_name);
		else sprintf(command, "viu -t -w 18 -h 8 /usr/lib/uwufetch/%s.png 2> /dev/null", version_name);
	}
	if (system(command) != 0) {	// if viu is not installed or the image is missing
		printf(	"\033[0E\033[3C%s\n"
				"   There was an\n"
				"    error: viu\n"
				" is not installed\n"
				"   or the image\n"
				"   is not fount\n"
				"  Read IMAGES.md\n"
				"   for more info.\n\n", RED);
	}
}

void usage(char* arg) {
	printf("Usage: %s <args>\n"
			"    -a, --ascii     prints logo as ascii text (default)\n"
			"    -c, --custom    choose a custom image\n"
			"    -d, --distro    lets you choose the logo to print\n"
			"    -h, --help      prints this help page\n"
			"    -i, --image     prints logo as image\n"
			"                    %sworks in most terminals\n"
			"                    read res/IMAGES.md for more info%s\n",
			arg, BLUE, NORMAL);
}

void uwu_name() {	// changes distro name to uwufied(?) name

	#define STRING_TO_UWU(original, uwufied) if (strcmp(version_name, original) == 0) sprintf(version_name, "%s", uwufied)
		// linux
		STRING_TO_UWU("arch", "Nyarch Linuwu");
		else STRING_TO_UWU("artix", "Nyartix Linuwu");
		else STRING_TO_UWU("debian", "Debinyan");
		else STRING_TO_UWU("fedora", "Fedowa");
		else STRING_TO_UWU("gentoo", "GentOwO");
		else STRING_TO_UWU("linuxmint", "LinUWU Miwint");
		else STRING_TO_UWU("manjaro", "Myanjawo");
		else STRING_TO_UWU("manjaro-arm", "Myanjawo AWM");
		else STRING_TO_UWU("popos", "PopOwOS");
		else STRING_TO_UWU("ubuntu", "Uwuntu");
		else STRING_TO_UWU("void", "OwOid");
		else STRING_TO_UWU("android", "Nyandroid");	// android at the end because it could be not considered as an actual distribution of gnu/linux

		// BSD
		else STRING_TO_UWU("freebsd", "FweeBSD");
		else STRING_TO_UWU("openbsd", "OwOpenBSD");


		else {
			sprintf(version_name, "%s", "unknown");
			if (a_i_flag == 1) {
				print_image();
				printf("\n");
			}
		}
	#undef STRING_TO_UWU
}
