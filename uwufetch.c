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
#include <sys/resource.h>
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

struct rusage r_usage;
struct utsname sys_var;
struct sysinfo sys;
int ram_max, pkgs, a_i_flag = 0;
char user[32], host[253], shell[64], version_name[64], cpu_model[256], pkgman_name[64], image_name[32];
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
	
	if (apt > 0) { total += apt; strcat(pkgman_name, "(apt)"); }
	if (dnf > 0) { total += dnf; strcat(pkgman_name, "(dnf)"); }
	if (emerge > 0) { total += emerge; strcat(pkgman_name, "(emerge)"); }
	if (flatpak > 0) { total += flatpak; strcat(pkgman_name, "(flatpak)"); }
	if (nix > 0) { total += nix; strcat(pkgman_name, "(nix)"); }
	if (pacman > 0) { total += pacman; strcat(pkgman_name, "(pacman)"); }
	if (rpm > 0) { total += rpm; strcat(pkgman_name, "(rpm)"); }
	if (xbps > 0) { total += xbps; strcat(pkgman_name, "(xbps)"); }

	return total;	
}

void print_info() {	// print collected info
	printf(	"\033[9A\033[18C%s%s%s@%s\n"
			"\033[17C %s%sOWOS     %s%s\n"
			"\033[17C %s%sKERNEL   %s%s %s\n"
			"\033[17C %s%sCPUWU    %s%s\n"
			"\033[17C %s%sWAM      %s%ldM/%iM\n"
			"\033[17C %s%sSHELL    %s%s\n"
			"\033[17C %s%sPKGS     %s%s%d %s\n"
			"\033[17C %s%sUWUPTIME %s%lid, %lih, %lim\n"
			"\033[17C %s%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\n",
			NORMAL, BOLD, user, host,
			NORMAL, BOLD, NORMAL, version_name,
			NORMAL, BOLD, NORMAL, sys_var.release, sys_var.machine,
			NORMAL, BOLD, NORMAL, cpu_model,
			NORMAL, BOLD, NORMAL, r_usage.ru_maxrss, ram_max,
			NORMAL, BOLD, NORMAL, shell,
			NORMAL, BOLD, NORMAL, NORMAL, pkgs, pkgman_name,
			NORMAL, BOLD, NORMAL, sys.uptime/60/60/24, sys.uptime/60/60%24, sys.uptime/60%60,
			BOLD, BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN,  WHITE, NORMAL);
}

void get_info() {	// get all necessary info
	// os version
	FILE *fos_rel = popen("cat /etc/os-release 2> /dev/null | awk '/^ID=/' | awk -F  '=' '{print $2}'", "r");
	fscanf(fos_rel,"%[^\n]", version_name);
	fclose(fos_rel);

	if (strlen(version_name) < 1) {	// handling unknown distribution
		DIR *system_app = opendir("/system/app/");
		DIR *system_priv_app = opendir("/system/priv-app/");
		if (system_app && system_priv_app) {	// android
			closedir(system_app);
			closedir(system_priv_app);
			sprintf(version_name, "android");
		} else sprintf(version_name, "unknown");
	}
	// user name, host name and shell
	if (strcmp(version_name, "android") != 0) {
		snprintf(user, 32, "%s", getenv("USER"));
		// cpu (this is here and not near the ram for efficiency)
		FILE *fcpu = popen("lscpu | grep 'Model name:' | cut -d ':' -f2 | sed 's/  //g' 2> /dev/null", "r");
		fscanf(fcpu, "%[^\n]", cpu_model);
		fclose(fcpu);
	}
	else if (strcmp(version_name, "android") == 0) {	// android vars
		FILE *whoami = popen("whoami", "r");
		fscanf(whoami, "%s", user);
		fclose(whoami);
		FILE *fcpu = popen("cat /proc/cpuinfo | grep 'Hardware' | cut -d ':' -f2 | sed 's/  //g' 2> /dev/null", "r");
		fscanf(fcpu, "%[^\n]", cpu_model);
		fclose(fcpu);
	}
	gethostname(host, 253);
	snprintf(shell, 16, "%s", getenv("SHELL"));
	memmove(&shell[0], &shell[5], 16);

	// system info
	uname(&sys_var);
	sysinfo(&sys);
	
	// ram
	ram_max = sys.totalram * sys.mem_unit / 1048576;
	getrusage(RUSAGE_SELF, &r_usage);
	pkgs = pkgman();
}

void list(char* arg) {	// prints distribution list
	/*	distributions are listed by distribution branch
		to make the output easier to understand by the user,
		also i didn't like the previous listing.*/
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
			NORMAL, BLUE, PINK, GREEN, WHITE,				// Other/spare distributions colors
			RED, YELLOW);								// BSD colors
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
}

void print_image() {	// prints logo (as an image) of the given system. distributions listed alphabetically.
	char command[256];
	if (strlen(image_name) > 1) sprintf(command, "viu -t -w 18 -h 8 %s 2> /dev/null", image_name);
	else sprintf(command, "viu -t -w 18 -h 8 /usr/lib/uwufetch/%s.png 2> /dev/null", version_name);
	if (system(command) != 0) {	// if viu is not installed
		printf(	"\033[1E\033[3C%s\n"
				"   There was an\n"
				" error, maybe viu\n"
				" is not installed.\n"
				" Read IMAGES.md\n"
				"  for more info.\n\n", RED);
	}
	printf("\033[1E\033[0C\b");
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

	// linux
	if (strcmp(version_name, "arch") == 0) sprintf(version_name, "%s", "Nyarch Linuwu");
	else if (strcmp(version_name, "artix") == 0) sprintf(version_name, "%s", "Nyartix Linuwu");
	else if (strcmp(version_name, "debian") == 0) sprintf(version_name, "%s", "Debinyan");
	else if (strcmp(version_name, "fedora") == 0) sprintf(version_name, "%s", "Fedowa");
	else if (strcmp(version_name, "gentoo") == 0) sprintf(version_name, "%s", "GentOwO");
	else if (strcmp(version_name, "linuxmint") == 0) sprintf(version_name, "%s", "LinUWU Miwint");
	else if (strcmp(version_name, "manjaro") == 0) sprintf(version_name, "%s", "Myanjawo");
	else if (strcmp(version_name, "\"manjaro-arm\"") == 0) sprintf(version_name, "%s", "Myanjawo AWM");
	else if (strcmp(version_name, "popos") == 0) sprintf(version_name, "%s", "PopOwOS");
	else if (strcmp(version_name, "ubuntu") == 0) sprintf(version_name, "%s", "Uwuntu");
	else if (strcmp(version_name, "void") == 0) sprintf(version_name, "%s", "OwOid");
	else if (strcmp(version_name, "android") == 0) sprintf(version_name, "%s", "Nyandroid");	// android at the end because it could be not considered as an actual distribution of gnu/linux
	
	// BSD
	else if (strcmp(version_name, "freebsd") == 0) sprintf(version_name, "%s", "FweeBSD");
	else if (strcmp(version_name, "openbsd") == 0) sprintf(version_name, "%s", "OwOpenBSD");
}