#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
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

char user[32], host[253], shell[64], version_name[64];
int pkgman();

int main() {

	// get user name, host name and shell
	struct utsname sys_var;
	struct sysinfo sys;

	snprintf(user, 32, "%s", getenv("USER"));
	gethostname(host, 253);
	snprintf(shell, 16, "%s", getenv("SHELL"));
	memmove(&shell[0], &shell[5], 16);

	// get os version
	FILE *fos_rel = popen("lsb_release -a | grep Description", "r");
	fscanf(fos_rel,"%[^\n]", version_name);
	fclose(fos_rel);
	memmove(&version_name[0], &version_name[12], 32);

	if (uname(&sys_var) == -1) printf("Ah sh*t, an error\n");
	if (sysinfo(&sys) == -1) printf("Ah sh*t, an error\n");
	
	// get cpu info
	char cpu_model[256];
	FILE *fcpu = popen("lscpu | grep 'Model name:'", "r");
	fscanf(fcpu, "%[^\n]", cpu_model);
	fclose(fcpu);
	memmove(&cpu_model[0], &cpu_model[33], 128);

	// get ram info
	int ram_max = sys.totalram * sys.mem_unit / 1048576;
	int ram_used;
	FILE *framu = popen("grep -i MemAvailable /proc/meminfo  | awk '{print $2}' ", "r");
	fscanf(framu, "%i", &ram_used);
	fclose(framu);
	ram_used = ram_used / 1024;
	
	// print collected info
	if (strcmp(version_name, "Arch Linux")) {
		printf("%s                 %s@%s\n", BOLD, user, host);
		printf("%s        /\\       %s%sOS %s%s\n", BLUE, NORMAL, BOLD, NORMAL, version_name);
		printf("%s       /  \\      %s%sKERNEL %s%s %s\n", BLUE, NORMAL, BOLD, NORMAL, sys_var.release, sys_var.machine);
		printf("%s      /\\   \\     %s%sCPU    %s%s\n", BLUE, NORMAL, BOLD, NORMAL, cpu_model);
		printf("%s     /      \\    %s%sRAM    %s%iM/%iM\n", BLUE, NORMAL, BOLD, NORMAL, ram_used, ram_max);
		printf("%s    /   __   \\   %s%sSHELL  %s%s\n", BLUE, NORMAL, BOLD, NORMAL, shell);
		printf("%s   / __|  |__-\\  %s%sPKGS   %s%s%d\n", BLUE, NORMAL, BOLD, NORMAL, NORMAL, pkgman());
		printf("%s  /_-''    ''-_\\ %s%sUPTIME %s%lid, %lih, %lim\n", BLUE, NORMAL, BOLD, NORMAL, sys.uptime/60/60/24, sys.uptime/60/60%24, sys.uptime/60%60);
		printf("                 %s%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\n", BOLD, BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN,  WHITE, NORMAL);
	}
}

int pkgman() { // this is just a function that returns the total of installed packages
	int apt, dnf, emerge, flatpak, nix, pacman, rpm, snap, xbps, total = 0;

	FILE *file[9];
	file[0] = popen("dpkg-query -f '${binary:Package}\n' -W 2> /dev/null | wc -l", "r");
	file[1] = popen("dnf list installed 2> /dev/null | wc -l", "r");
	file[2] = popen("qlist -I 2> /dev/null | wc -l", "r");
	file[3] = popen("flatpak list 2> /dev/null | wc -l", "r");
	file[4] = popen("nix-store -q --requisites /run/current-sys_vartem/sw 2> /dev/null | wc -l", "r");
	file[5] = popen("pacman -Qq 2> /dev/null | wc -l", "r");
	file[6] = popen("rpm -qa --last 2> /dev/null | wc -l", "r");
	file[7] = popen("snap list 2> /dev/null | wc -l", "r");
	file[8] = popen("xbps-query -l 2> /dev/null | wc -l", "r");

	fscanf(file[0], "%d", &apt);
	fscanf(file[1], "%d", &dnf);
	fscanf(file[2], "%d", &emerge);
	fscanf(file[3], "%d", &flatpak);
	fscanf(file[4], "%d", &nix);
	fscanf(file[5], "%d", &pacman);
	fscanf(file[6], "%d", &rpm);
	fscanf(file[7], "%d", &snap);
	fscanf(file[8], "%d", &xbps);
	for (int i = 0; i < 9; i++) fclose(file[i]);

	if (apt > 0) total += apt;
	if (dnf > 0) total += dnf;
	if (emerge > 0) total += emerge;
	if (flatpak > 0) total += flatpak;
	if (nix > 0) total += nix;
	if (pacman > 0) total += pacman;
	if (rpm > 0) total += rpm;
	if (snap > 0) total += snap;
	if (xbps > 0) total += xbps;

	return total;	
}
