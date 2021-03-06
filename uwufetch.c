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
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define WHITE "\x1b[37m"

struct rusage r_usage;
struct utsname sys_var;
struct sysinfo sys;
int ram_max, pkgs;
char user[32], host[253], shell[64], version_name[64], cpu_model[256];

int pkgman();
void get_info();
void print_ascii();
void print_info();
void print_image();

int main(int argc, char *argv[]) {

	get_info();
	//sprintf(version_name, "%s", "manjaro"); // a debug thing
	//char c = getopt(argc, argv, "adhi"); // things for the future
	print_ascii();
	//system("viu -t -w 18 -h 8 $HOME/.config/uwufetch/arch.png"); // other things for the future
	print_info(user, host, version_name, sys_var.release, sys_var.machine, cpu_model, &r_usage.ru_maxrss, &ram_max, shell, &sys.uptime);
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

	if (apt > 0) total += apt;
	if (dnf > 0) total += dnf;
	if (emerge > 0) total += emerge;
	if (flatpak > 0) total += flatpak;
	if (nix > 0) total += nix;
	if (pacman > 0) total += pacman;
	if (rpm > 0) total += rpm;
	if (xbps > 0) total += xbps;

	return total;	
}

void print_info() {	// print collected info
	printf("\033[2;18H %s%s%s@%s\n", NORMAL, BOLD, user, host);
	printf("\033[3;18H %s%sOWOS     %s%s\n", NORMAL, BOLD, NORMAL, version_name);
	printf("\033[4;18H %s%sKERNEL   %s%s %s\n", NORMAL, BOLD, NORMAL, sys_var.release, sys_var.machine);
	printf("\033[5;18H %s%sCPUWU    %s%s\n", NORMAL, BOLD, NORMAL, cpu_model);
	printf("\033[6;18H %s%sWAM      %s%ldM/%iM\n", NORMAL, BOLD, NORMAL, r_usage.ru_maxrss, ram_max);
	printf("\033[7;18H %s%sSHELL    %s%s\n", NORMAL, BOLD, NORMAL, shell);
	printf("\033[8;18H %s%sPKGS     %s%s%d\n", NORMAL, BOLD, NORMAL, NORMAL, pkgs);
	printf("\033[9;18H %s%sUWUPTIME %s%lid, %lih, %lim\n", NORMAL, BOLD, NORMAL, sys.uptime/60/60/24, sys.uptime/60/60%24, sys.uptime/60%60);
	printf("\033[10;18H %s%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\n", BOLD, BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN,  WHITE, NORMAL);
}

void get_info() {	// get all necessary info

	// user name, host name and shell
	snprintf(user, 32, "%s", getenv("USER"));
	gethostname(host, 253);
	snprintf(shell, 16, "%s", getenv("SHELL"));
	memmove(&shell[0], &shell[5], 16);

	// os version
	FILE *fos_rel = popen("cut -d '=' -f2 <<< $(cat /etc/os-release | grep ID=) 2> /dev/null", "r");
	fscanf(fos_rel,"%[^\n]", version_name);
	fclose(fos_rel);

	if (uname(&sys_var) == -1) printf("There was some kind of error while getting the username\n");
	if (sysinfo(&sys) == -1) printf("There was some kind of error while getting system info\n");
	
	// cpu and ram
	FILE *fcpu = popen("sed 's/  //g' <<< $(cut -d ':' -f2 <<< $(lscpu | grep 'Model name:')) 2> /dev/null", "r");
	fscanf(fcpu, "%[^\n]", cpu_model);
	fclose(fcpu);
	ram_max = sys.totalram * sys.mem_unit / 1048576;
	getrusage(RUSAGE_SELF, &r_usage);
	pkgs = pkgman();
}

void print_ascii() {
	if (strcmp(version_name, "arch") == 0) {
		sprintf(version_name, "%s", "Nyarch Linuwu");
		printf(	"\033[3;9H%s/\\\n"
				"       /  \\\n"
				"      /\\   \\\n"
				"     / > w <\\\n"
				"    /   __   \\\n"
				"   / __|  |__-\\\n"
				"  /_-''    ''-_\\\n", BLUE);
	} else if (strcmp(version_name, "artix") == 0) {
		sprintf(version_name, "%s", "Nyartix Linuwu");
		printf(	"\033[3;9H%s/\\\n"
				"       /  \\\n"
				"      /`'.,\\\n"
				"     /\u2022 w \u2022 \\\n"
				"    /      ,`\\\n"
				"   /   ,.'`.  \\\n"
				"  /.,'`     `'.\\\n", BLUE);
	} else if (strcmp(version_name, "debian") == 0) {
		sprintf(version_name, "%s", "Debinyan");
		printf(	"\033[3;7H%s______\n"
				"     /  ___ \\\n"
				"    |  / OwO |\n"
				"    |  \\____-\n"
				"    -_\n"
				"      --_\n", RED);
	} else if (strcmp(version_name, "fedora") == 0) {
		sprintf(version_name, "%s", "Fedowora");
		printf(	"\033[2;9H%s_____\n"
				"       /   __)%s\\\n"
				"     %s> %s|  / %s<%s\\ \\\n"
				"    __%s_| %sw%s|_%s_/ /\n"
				"   / %s(_    _)%s_/\n"
				"  / /  %s|  |\n"
				"  %s\\ \\%s__/  |\n"
				"   %s\\%s(_____/\n", BLUE, CYAN, WHITE, BLUE, WHITE, CYAN, BLUE, CYAN, BLUE, CYAN, BLUE, CYAN, BLUE, CYAN, BLUE, CYAN, BLUE);
	} else if (strcmp(version_name, "manjaro") == 0) {
		sprintf(version_name, "%s", "Myanjaro");
		printf(	" \u25b3       \u25b3   \u25e0\u25e0\u25e0\u25e0\n"
				" \e[0;42m          \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m \e[0m\e[0;42m\e[1;30m > w < \e[0m\e[0;42m  \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m        \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n"
				" \e[0;42m    \e[0m  \e[0;42m    \e[0m  \e[0;42m    \e[0m\n");
	}
}