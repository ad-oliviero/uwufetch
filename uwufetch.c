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
#define PINK "\x1b[1;35m"

struct rusage r_usage;
struct utsname sys_var;
struct sysinfo sys;
int ram_max, pkgs;
char user[32], host[253], shell[64], version_name[64], cpu_model[256], pkgman_name[64];
int pkgman();
void get_info();
void print_ascii();
void print_info();
void print_image();
void usage(char*);

int main(int argc, char *argv[]) {
	int opt = 0, a_i_flag = 0;
	get_info();
	//sprintf(version_name, "%s", "debian"); // a debug thing

	while((opt = getopt(argc, argv, "ad:hi")) != -1) {
		switch(opt) {
			case 'a':
				a_i_flag = 0;
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
			default:
				break;
		}
	}
	if (argc == 1 || a_i_flag == 0) print_ascii();
	else if (a_i_flag) print_image();
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
	printf("\033[9A\033[17C %s%s%s@%s\n", NORMAL, BOLD, user, host);
	printf("\033[17C %s%sOWOS     %s%s\n", NORMAL, BOLD, NORMAL, version_name);
	printf("\033[17C %s%sKERNEL   %s%s %s\n", NORMAL, BOLD, NORMAL, sys_var.release, sys_var.machine);
	printf("\033[17C %s%sCPUWU    %s%s\n", NORMAL, BOLD, NORMAL, cpu_model);
	printf("\033[17C %s%sWAM      %s%ldM/%iM\n", NORMAL, BOLD, NORMAL, r_usage.ru_maxrss, ram_max);
	printf("\033[17C %s%sSHELL    %s%s\n", NORMAL, BOLD, NORMAL, shell);
	printf("\033[17C %s%sPKGS     %s%s%d %s\n", NORMAL, BOLD, NORMAL, NORMAL, pkgs, pkgman_name);
	printf("\033[17C %s%sUWUPTIME %s%lid, %lih, %lim\n", NORMAL, BOLD, NORMAL, sys.uptime/60/60/24, sys.uptime/60/60%24, sys.uptime/60%60);
	printf("\033[17C %s%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\n", BOLD, BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN,  WHITE, NORMAL);
}

void get_info() {	// get all necessary info

	// user name, host name and shell
	snprintf(user, 32, "%s", getenv("USER"));
	gethostname(host, 253);
	snprintf(shell, 16, "%s", getenv("SHELL"));
	memmove(&shell[0], &shell[5], 16);

	// os version
	FILE *fos_rel = popen("cat /etc/os-release | awk '/^ID=/' | awk -F  '=' '{print $2}'  2> /dev/null", "r");
	fscanf(fos_rel,"%[^\n]", version_name);
	fclose(fos_rel);

	// system info
	if (uname(&sys_var) == -1) printf("There was some kind of error while getting the username\n");
	if (sysinfo(&sys) == -1) printf("There was some kind of error while getting system info\n");
	
	// cpu and ram
	FILE *fcpu = popen("lscpu | grep 'Model name:' | cut -d ':' -f2 | sed 's/  //g' 2> /dev/null", "r");
	fscanf(fcpu, "%[^\n]", cpu_model);
	fclose(fcpu);
	ram_max = sys.totalram * sys.mem_unit / 1048576;
	getrusage(RUSAGE_SELF, &r_usage);
	pkgs = pkgman();
}

void print_ascii() {	// prints logo (as ascii art) of the given system. distributions listed alphabetically.
	
	// linux

	if (strcmp(version_name, "arch") == 0) {
		sprintf(version_name, "%s", "Nyarch Linuwu");
		printf(	"\033[1E\033[8C%s/\\\n"
				"       /  \\\n"
				"      /\\   \\\n"
				"     / > w <\\\n"
				"    /   __   \\\n"
				"   / __|  |__-\\\n"
				"  /_-''    ''-_\\\n\n", BLUE);
	} else if (strcmp(version_name, "artix") == 0) {
		sprintf(version_name, "%s", "Nyartix Linuwu");
		printf(	"\033[1E\033[8C%s/\\\n"
				"       /  \\\n"
				"      /`'.,\\\n"
				"     /\u2022 w \u2022 \\\n"
				"    /      ,`\\\n"
				"   /   ,.'`.  \\\n"
				"  /.,'`     `'.\\\n\n", BLUE);
	} else if (strcmp(version_name, "debian") == 0) {
		sprintf(version_name, "%s", "Debinyan");
		printf(	"\033[1E\033[6C%s______\n"
				"     /  ___ \\\n"
				"    |  / OwO |\n"
				"    |  \\____-\n"
				"    -_\n"
				"      --_\n\n\n", RED);
	} else if (strcmp(version_name, "fedora") == 0) {
		sprintf(version_name, "%s", "Fedowoa");
		printf(	"\033[1E\033[8C%s_____\n"
				"       /   __)%s\\\n"
				"     %s> %s|  / %s<%s\\ \\\n"
				"    __%s_| %sw%s|_%s_/ /\n"
				"   / %s(_    _)%s_/\n"
				"  / /  %s|  |\n"
				"  %s\\ \\%s__/  |\n"
				"   %s\\%s(_____/\n", BLUE, CYAN, WHITE, BLUE, WHITE, CYAN, BLUE, CYAN, BLUE, CYAN, BLUE, CYAN, BLUE, CYAN, BLUE, CYAN, BLUE);
	} else if (strcmp(version_name, "gentoo") == 0) {
		sprintf(version_name, "%s", "GentOwO");
		printf(	"\033[1E\033[3C%s_-----_\n"
				"  (       \\\n"
				"  \\   OwO   \\\n"
				"%s   \\         )\n"
				"   /       _/\n"
				"  (      _-\n"
				"  \\____-\n\n", MAGENTA, WHITE);
	} else if (strcmp(version_name, "manjaro") == 0) {
		sprintf(version_name, "%s", "Myanjawo");
		printf(	"\033[0E\033[1C\u25b3       \u25b3   \u25e0\u25e0\u25e0\u25e0\n"
				" \e[0;42m          \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m \e[0m\e[0;42m\e[1;30m > w < \e[0m\e[0;42m  \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m        \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n");
	} else if (strcmp(version_name, "\"manjaro-arm\"") == 0) { // we use \ to include the " as the ID on manjaro-arm returns "manjaro-arm" not just manjaro-arm.
		sprintf(version_name, "%s", "Myanjawo AWM");
		printf(	"\033[0E\033[1C\u25b3       \u25b3   \u25e0\u25e0\u25e0\u25e0\n"
				" \e[0;42m          \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m \e[0m\e[0;42m\e[1;30m > w < \e[0m\e[0;42m  \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m        \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n");
	}

	// BSD

	else if (strcmp(version_name, "openbsd") == 0) {
		sprintf(version_name, "%s", "OwOpenBSD");
		printf(	"\033[1E\033[3C%s  ______  \n"
					"   \\-      -/  %s\u2665  \n"
					"%s\\_/          \\  \n"
					"|        %s>  < %s|   \n"
					"|_  <  %s//  %sW %s//   \n"
					"%s/ \\          /   \n"
					"  /-________-\\   \n\n", YELLOW, RED, YELLOW, WHITE, YELLOW, PINK, WHITE, PINK, YELLOW);

 	} else if (strcmp(version_name, "freebsd") == 0) {
		sprintf(version_name, "%s", "FweeBSD");
		printf(	"\033[1E\033[3C%s\n"
				" /\\,-'''''-,/\\\n"
				" \\_)       (_/\n"
				" |   \\   /   |\n"
				" |   O w O   |\n"
				"  ;         ;\n"
				"   '-_____-'\n", RED);
	 }
}

void print_image() {	// prints logo (as an image) of the given system. distributions listed alphabetically.
	char command[256];
	sprintf(command, "viu -t -w 18 -h 8 /usr/lib/uwufetch/%s.png", version_name);
	system(command);

	if (strcmp(version_name, "arch") == 0) sprintf(version_name, "%s", "Nyarch Linuwu");
	if (strcmp(version_name, "artix") == 0) sprintf(version_name, "%s", "Nyartix Linuwu");
	if (strcmp(version_name, "debian") == 0) sprintf(version_name, "%s", "Debinyan");
	if (strcmp(version_name, "fedora") == 0) sprintf(version_name, "%s", "Fedowa");
	if (strcmp(version_name, "gentoo") == 0) sprintf(version_name, "%s", "GentOwO");
	if (strcmp(version_name, "manjaro") == 0) sprintf(version_name, "%s", "Myanjawo");
}

void usage(char* arg) {
	printf("Usage: %s <args>\n"
			"    -a, --ascii     prints logo as ascii text (default)\n"
			"    -d, --distro    %slets you choose the logo to print%s\n"
			"    -h, --help      prints this help page\n"
			"    -i, --image     prints logo as image\n"
			"                    %sworks in few terminals\n"
			"                    <cat res/IMAGES.md> for more info%s\n",
			arg, YELLOW, NORMAL, BLUE, NORMAL);
}
