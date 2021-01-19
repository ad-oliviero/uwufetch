#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __linux__
    #include <sys/utsname.h>
    #include <sys/sysinfo.h>
#elif __ANDROID__
    #include <sys/utsname.h>
    #include <sys/sysinfo.h>
#elif _WIN32

#endif
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

void pkgman();

int main() {
	// init variables and other useful things
	char user[32], host[253], shell[64], version_name[64];

	// get user and host names
    #ifdef __linux__
        struct utsname sys_var;
        struct sysinfo sys;

        snprintf(user, 32, "%s", getenv("USER"));
        gethostname(host, 253);
        snprintf(shell, 16, "%s", getenv("SHELL"));
        memmove(&shell[0], &shell[5], 16);

        FILE *fos_rel = popen("lsb_release -a | grep Description", "r");
        fscanf(fos_rel,"%[^\n]", version_name);
        fclose(fos_rel);
        memmove(&version_name[0], &version_name[12], 64);
    #elif __ANDROID__
        FILE *fwhoami = popen("whoami", "r");
        fscanf(fwhoami,"%[^\n]", user);
        fclose(fwhoami);

        FILE *fos_rel = popen("getprop ro.system.build.version.release", "r");
        fscanf(fos_rel,"%[^\n]", version_name);
        fclose(fos_rel);
        snprintf(shell, 16, "%s", getenv("SHELL"));
        memmove(&shell[0], &shell[36], 64);
    #endif

	if (uname(&sys_var) == -1) printf("Ah sh-t, an error\n");
    if (sysinfo(&sys) == -1) printf("Ah sh-t, an error\n");
    
    char cpu_model[128];
    FILE *fcpu = popen("lscpu | grep 'Model name:'", "r");
    fscanf(fcpu, "%[^\n]", cpu_model);
    fclose(fcpu);
	memmove(&cpu_model[0], &cpu_model[33], 128);

    int ram_max = sys.totalram * sys.mem_unit / 1048576;

    int ram_used;
    FILE *framu = popen("grep -i MemAvailable /proc/meminfo  | awk '{print $2}' ", "r");
    fscanf(framu, "%i", &ram_used);
    fclose(framu);
    ram_used = ram_used / 1024;
    
    // Now we print the info and exit the program.
    //NORMAL, BOLD, BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN,  WHITE 
    if (strcmp(version_name, "Arch Linux")) {
        printf("%s                 %s@%s\n", BOLD, user, host);
        printf("%s        /\\       %s%sOS %s%s\n", BLUE, NORMAL, BOLD, NORMAL, version_name);
        printf("%s       /  \\      %s%sKERNEL %s%s %s\n", BLUE, NORMAL, BOLD, NORMAL, sys_var.release, sys_var.machine);
        printf("%s      /\\   \\     %s%sCPU    %s%s\n", BLUE, NORMAL, BOLD, NORMAL, cpu_model);
        printf("%s     /      \\    %s%sRAM    %s%iM/%iM\n", BLUE, NORMAL, BOLD, NORMAL, ram_used, ram_max);
        printf("%s    /   __   \\   %s%sSHELL  %s%s\n", BLUE, NORMAL, BOLD, NORMAL, shell);
        printf("%s   / __|  |__-\\  %s%sPKGS   %s%s", BLUE, NORMAL, BOLD, NORMAL, NORMAL); pkgman();
        printf("%s  /_-''    ''-_\\ %s%sUPTIME %s%lid, %lih, %lim\n", BLUE, NORMAL, BOLD, NORMAL, sys.uptime/60/60/24, sys.uptime/60/60%24, sys.uptime/60%60);
        printf("                 %s%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\u2587\u2587%s\n", BOLD, BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN,  WHITE, NORMAL);
    }
    

	return 0;
}

void pkgman() {
	int apt, dnf, emerge, flatpak, nix, pacman, rpm, snap, xbps, yay;

    FILE *fapt = popen("dpkg-query -f '${binary:Package}\n' -W 2>/dev/null | wc -l", "r");
    FILE *fdnf = popen("dnf list installed 2>/dev/null | wc -l", "r");
    FILE *femerge = popen("qlist -I 2>/dev/null | wc -l", "r");
    FILE *fflatpak = popen("flatpak list 2>/dev/null | wc -l", "r");
    FILE *fnix = popen("nix-store -q --requisites /run/current-sys_vartem/sw 2>/dev/null | wc -l", "r");
    FILE *fpacman = popen("pacman -Q 2>/dev/null | wc -l", "r");
    FILE *frpm = popen("rpm -qa --last 2>/dev/null | wc -l", "r");
    FILE *fsnap = popen("snap list 2>/dev/null | wc -l", "r");
    FILE *fxbps = popen("xbps-query -l 2>/dev/null | wc -l", "r");
    FILE *fyay = popen("yay -Q 2>/dev/null | wc -l", "r");
    fscanf(fapt, "%d", &apt);
    fscanf(fdnf, "%d", &dnf);
    fscanf(femerge, "%d", &emerge);
    fscanf(fflatpak, "%d", &flatpak);
    fscanf(fnix, "%d", &nix);
    fscanf(fpacman, "%d", &pacman);
    fscanf(frpm, "%d", &rpm);
    fscanf(fsnap, "%d", &snap);
    fscanf(fxbps, "%d", &xbps);
    fscanf(fyay, "%d", &yay);
    fclose(fapt);
    fclose(fdnf);
    fclose(femerge);
    fclose(fflatpak);
    fclose(fnix);
    fclose(fpacman);
    fclose(frpm);
    fclose(fsnap);
    fclose(fxbps);
    fclose(fyay);

    int pipe = 0;

    if (apt > 0) {
        if (pipe == 1) printf(" | ");
        printf("apt %d", apt);
        pipe = 1;
    }
    if (dnf > 0) {
        if(pipe == 1) printf(" | ");
        printf("dnf %d", dnf);
        pipe = 1;
    }
    if (emerge > 0) {
        if (pipe == 1) printf(" | ");
        printf("emerge %d", emerge);
        pipe = 1;
    }
    if (flatpak > 0) {
        if (pipe == 1) printf(" | ");
        printf("flatpak %d", flatpak);
        pipe = 1;
    }
    if (nix > 0) {
        if (pipe == 1) printf(" | ");
        printf("nix %d", nix);
        pipe = 1;
    }
    if (pacman > 0) {
        if (pipe == 1) printf(" | ");
        printf("pacman %d", pacman);
        pipe = 1;
    }
    if (rpm > 0) {
        if (pipe == 1) printf(" | ");
        printf("rpm %d", rpm);
        pipe = 1;
    }
    if (snap > 0) {
        if (pipe == 1) printf(" | ");
        printf("snap %d", snap);
        pipe = 1;
    }
    if (xbps > 0) {
        if (pipe == 1) printf(" | ");
        printf("xbps %d", xbps);
        pipe = 1;
    }
    if (yay > 0) {
        if (pipe == 1) printf(" | ");
        printf("yay %d", yay);
        pipe = 1;
    }
    printf("\n");
}
