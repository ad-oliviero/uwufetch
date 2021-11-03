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

#define _GNU_SOURCE // for strcasestr

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#if defined(__APPLE__) || defined(__FREEBSD__)
#include <sys/sysctl.h>
#include <time.h>
#else // defined(__APPLE__) || defined(__FREEBSD__)
#ifdef __FREEBSD__
#else // defined(__FREEBSD__) || defined(__WINDOWS__)
#ifndef __WINDOWS__
#include <sys/sysinfo.h>
#else // __WINDOWS__
#include <sysinfoapi.h>
#endif // __WINDOWS__
#endif // defined(__FREEBSD__) || defined(__WINDOWS__)
#endif // defined(__APPLE__) || defined(__FREEBSD__)
#ifndef __WINDOWS__
#include <sys/utsname.h>
#include <sys/ioctl.h>
#else // __WINDOWS__
#include <windows.h>
CONSOLE_SCREEN_BUFFER_INFO csbi;
#endif // __WINDOWS__

// COLORS
#define NORMAL "\x1b[0m"
#define BOLD "\x1b[1m"
#define BLACK "\x1b[30m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define SPRING_GREEN "\x1b[38;5;120m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[0;35m"
#define CYAN "\x1b[36m"
#define WHITE "\x1b[37m"
#define PINK "\x1b[38;5;201m"
#define LPINK "\x1b[38;5;213m"

#ifdef __WINDOWS__
#define BLOCK_CHAR "\xdb"
#else // __WINDOWS__
#define BLOCK_CHAR "\u2587"
#endif // __WINDOWS__
#ifdef __APPLE__
// buffers where data fetched from sysctl are stored
// CPU
#define CPUBUFFERLEN 128

char cpu_buffer[CPUBUFFERLEN];
size_t cpu_buffer_len = CPUBUFFERLEN;

// Installed RAM
int64_t mem_buffer = 0;
size_t mem_buffer_len = sizeof(mem_buffer);

// uptime
struct timeval time_buffer;
size_t time_buffer_len = sizeof(time_buffer);
#endif // __APPLE__

struct package_manager
{
	char command_string[128]; // command to get number of packages installed
	char pkgman_name[16];	  // name of the package manager
};
#ifndef __WINDOWS__
struct utsname sys_var;
#endif // __WINDOWS__
#ifndef __APPLE__
#ifdef __linux__
struct sysinfo sys;
#else // __linux__
#ifdef __WINDOWS__
struct _SYSTEM_INFO sys;
#endif // __WINDOWS__
#endif // __linux__
#endif // __APPLE__
#ifndef __WINDOWS__
struct winsize win;
#else  // __WINDOWS__
int ws_col, ws_rows;
#endif // __WINDOWS__

// initialise the variables to store data, gpu array can hold up to 8 gpus
int target_width = 0, screen_width = 0, screen_height = 0, ram_total, ram_used = 0, pkgs = 0;
long uptime = 0;
// all flags available
int ascii_image_flag = 0, // when (0) ascii is printed, when (1) image is printed
	show_user_info = 1,
	show_os = 1,
	show_host = 1,
	show_kernel = 1,
	show_cpu = 1,
	show_gpu = 1,
	show_ram = 1,
	show_resolution = 1,
	show_shell = 1,
	show_pkgs = 1,
	show_uptime = 1,
	show_colors = 1;

char user[128], host[256], shell[64], host_model[256], kernel[256], version_name[64], cpu_model[256],
	gpu_model[64][256],
	pkgman_name[64], image_name[128], *config_directory = NULL, *cache_content = NULL;

char *terminal_cursor_move = "\033[18C";

// functions definitions, to use them in main()
int pkgman();
void parse_config();
void get_info();
void list();
void replace(char *original, char *search, char *replacer);
void replace_ignorecase(char *original, char *search, char *replacer);
void print_ascii();
void print_unknown_ascii();
void print_info();
void write_cache();
int read_cache();
void print_cache();
void print_image();
void usage(char *);
void uwu_kernel();
void uwu_hw(char *);
void uwu_name();
void truncate_name(char *name);
void remove_brackets(char *str);

int main(int argc, char *argv[])
{
	char *cache_env = getenv("UWUFETCH_CACHE_ENABLED");
	if (cache_env != NULL)
	{

		int cache_enabled = 0;
		char buffer[128];

		sscanf(cache_env, "%4[TRUEtrue1]", buffer);
		cache_enabled = (strcmp(buffer, "true") == 0 || strcmp(buffer, "TRUE") == 0 || strcmp(buffer, "1") == 0);
		if (cache_enabled)
		{
			// if no cache file found write to it
			if (!read_cache())
			{
				get_info();
				write_cache();
			}
			parse_config();
			print_cache();
			return 0;
		}
	}
#ifdef __WINDOWS__
	// packages disabled by default because chocolatey is slow
	show_pkgs = 0;
#endif

	int opt = 0;
	static struct option long_options[] = {
		{"ascii", no_argument, NULL, 'a'},
		{"config", required_argument, NULL, 'c'},
		// {"cache", no_argument, NULL, 'C'},
		{"distro", required_argument, NULL, 'd'},
		{"write-cache", no_argument, NULL, 'w'},
		{"help", no_argument, NULL, 'h'},
		{"image", optional_argument, NULL, 'i'},
		{"list", no_argument, NULL, 'l'},
		{NULL, 0, NULL, 0}};
	get_info();
	parse_config();
	while ((opt = getopt_long(argc, argv, "ac:d:hi::lw", long_options, NULL)) != -1)
	{
		switch (opt)
		{
		case 'a':
			ascii_image_flag = 0;
			break;
		case 'c':
			config_directory = optarg;
			break;
		case 'd':
			if (optarg)
				sprintf(version_name, "%s", optarg);
			break;
		case 'h':
			usage(argv[0]);
			return 0;
		case 'i':
			ascii_image_flag = 1;
			if (!optarg && argv[optind] != NULL && argv[optind][0] != '-')
				sprintf(image_name, "%s", argv[optind++]);
			else if (optarg)
				sprintf(image_name, "%s", optarg);
			break;
		case 'l':
			list(argv[0]);
			return 0;
		case 'w':
			write_cache();
			print_cache();
			return 0;
		default:
			break;
		}
	}
	if ((argc == 1 && ascii_image_flag == 0) || (argc > 1 && ascii_image_flag == 0))
	{
		printf("\n");	   // print a new line
		printf("\033[1A"); // go up one line if possible
		print_ascii();
	}
	else if (ascii_image_flag == 1)
		print_image();

	print_info();
}

void parse_config()
{
	char line[256];

	FILE *config = NULL;
	if (config_directory == NULL)
	{
		if (getenv("HOME") != NULL)
		{
			char homedir[512];
			sprintf(homedir, "%s/.config/uwufetch/config", getenv("HOME"));
			config = fopen(homedir, "r");
		}
	}
	else
		config = fopen(config_directory, "r");
	if (config == NULL)
		return;

	while (fgets(line, sizeof(line), config))
	{
		char buffer[128] = {0};

		sscanf(line, "distro=%s", version_name);
		if (sscanf(line, "ascii=%[truefalse]", buffer))
			ascii_image_flag = !strcmp(buffer, "false");
		if (sscanf(line, "image=\"%[^\"]\"", image_name))
		{
			if (image_name[0] == '~')
			{ // image name with ~ does not work
				memmove(&image_name[0], &image_name[1], strlen(image_name));
				char temp[128] = "/home/";
				strcat(temp, user);
				strcat(temp, image_name);
				sprintf(image_name, "%s", temp);
			}
			ascii_image_flag = 1;
		}
		if (sscanf(line, "user=%[truefalse]", buffer))
			show_user_info = !strcmp(buffer, "true");
		if (sscanf(line, "os=%[truefalse]", buffer))
			show_os = strcmp(buffer, "false");
		if (sscanf(line, "host=%[truefalse]", buffer))
			show_host = strcmp(buffer, "false");
		if (sscanf(line, "kernel=%[truefalse]", buffer))
			show_kernel = strcmp(buffer, "false");
		if (sscanf(line, "cpu=%[truefalse]", buffer))
			show_cpu = strcmp(buffer, "false");
		if (sscanf(line, "gpu=%[truefalse]", buffer))
			show_gpu = strcmp(buffer, "false");
		if (sscanf(line, "ram=%[truefalse]", buffer))
			show_ram = strcmp(buffer, "false");
		if (sscanf(line, "resolution=%[truefalse]", buffer))
			show_resolution = strcmp(buffer, "false");
		if (sscanf(line, "shell=%[truefalse]", buffer))
			show_shell = strcmp(buffer, "false");
		if (sscanf(line, "pkgs=%[truefalse]", buffer))
			show_pkgs = strcmp(buffer, "false");
		if (sscanf(line, "uptime=%[truefalse]", buffer))
			show_uptime = strcmp(buffer, "false");
		if (sscanf(line, "colors=%[truefalse]", buffer))
			show_colors = strcmp(buffer, "false");
	}
	fclose(config);
}

int pkgman()
{ // this is just a function that returns the total of installed packages
	int total = 0;

#ifndef __APPLE__ // this function is not used on mac os because it causes lots of problems
#ifndef __WINDOWS__
	struct package_manager pkgmans[] = {
		{"apt list --installed 2> /dev/null | wc -l", "(apt)"},
		{"apk info 2> /dev/null | wc -l", "(apk)"},
		{"dnf list installed 2> /dev/null | wc -l", "(dnf)"},
		{"qlist -I 2> /dev/null | wc -l", "(emerge)"},
		{"flatpak list 2> /dev/null | wc -l", "(flatpak)"},
		{"snap list 2> /dev/null | wc -l", "(snap)"},
		{"guix package --list-installed 2> /dev/null | wc -l", "(guix)"},
		{"nix-store -q --requisites /run/current-system/sw 2> /dev/null | wc -l", "(nix)"},
		{"pacman -Qq 2> /dev/null | wc -l", "(pacman)"},
		{"pkg info 2>/dev/null | wc -l", "(pkg)"},
		{"port installed 2> /dev/null | tail -n +2 | wc -l", "(port)"},
		{"rpm -qa --last 2> /dev/null | wc -l", "(rpm)"},
		{"xbps-query -l 2> /dev/null | wc -l", "(xbps)"},
		{"zypper -q se --installed-only 2> /dev/null | wc -l", "(zypper)"}};
	const unsigned long pkgman_count = sizeof(pkgmans) / sizeof(pkgmans[0]);
	//	to format the pkgman_name string properly
	int comma_separator = 0;
	for (long unsigned int i = 0; i < pkgman_count; i++)
	{ // long unsigned int instead of int because of -Wsign-compare
		struct package_manager *current = &pkgmans[i];

		FILE *fp = popen(current->command_string, "r");
		unsigned int pkg_count;

		if (fscanf(fp, "%u", &pkg_count) == 3)
			continue;
		fclose(fp);

		total += pkg_count;
		if (pkg_count > 0)
		{
			if (comma_separator)
				strcat(pkgman_name, ", ");
			comma_separator++;

			char spkg_count[16];
			sprintf(spkg_count, "%u", pkg_count);
			strcat(pkgman_name, spkg_count);
			strcat(pkgman_name, " ");
			strcat(pkgman_name, current->pkgman_name); // this is the line that breaks mac os, but something strange happens before
		}
	}
#else  // __WINDOWS__
	if (show_pkgs)
	{
		FILE *fp = popen("choco list -l --no-color 2> nul", "r");
		unsigned int pkg_count;
		char buffer[7562] = {0};
		while (fgets(buffer, sizeof(buffer), fp))
		{
			sscanf(buffer, "%u packages installed.", &pkg_count);
		}
		if (fp)
			pclose(fp);

		total = pkg_count;
		char spkg_count[16];
		sprintf(spkg_count, "%u", pkg_count);
		strcat(pkgman_name, spkg_count);
		strcat(pkgman_name, " ");
		strcat(pkgman_name, "(chocolatey)");
	}
#endif // __WINDOWS__

#endif
	return total;
}

#ifdef __APPLE__
int uptime_mac()
{
	int mib[2] = {CTL_KERN, KERN_BOOTTIME};
	sysctl(mib, 2, &time_buffer, &time_buffer_len, NULL, 0);

	time_t bsec = time_buffer.tv_sec;
	time_t csec = time(NULL);

	return difftime(csec, bsec);
}
#endif

#ifdef __FREEBSD__
int uptime_freebsd()
{ // this code is from coreutils uptime: https://github.com/coreutils/coreutils/blob/master/src/uptime.c
	int boot_time = 0;
	static int request[2] = {CTL_KERN, KERN_BOOTTIME};
	struct timeval result;
	size_t result_len = sizeof result;

	if (sysctl(request, 2, &result, &result_len, NULL, 0) >= 0)
		boot_time = result.tv_sec;
	int time_now = time(NULL);
	return time_now - boot_time;
}
#endif

void print_info()
{
#ifdef __WINDOWS__
#define responsively_printf(buf, format, ...) \
	{                                         \
		sprintf(buf, format, __VA_ARGS__);    \
		printf("%.*s\n", ws_col - 1, buf);    \
	}
#else // __WINDOWS__
#define responsively_printf(buf, format, ...)  \
	{                                          \
		sprintf(buf, format, __VA_ARGS__);     \
		printf("%.*s\n", win.ws_col - 1, buf); \
	}
#endif					  // __WINDOWS__
	char print_buf[1024]; // for responsively print

	// print collected info - from host to cpu info
	printf("\033[9A"); // to align info text
	if (show_user_info)
		responsively_printf(print_buf, "%s%s%s%s@%s", terminal_cursor_move, NORMAL, BOLD, user, host);
	uwu_name();
	if (show_os)
		responsively_printf(print_buf, "%s%s%sOWOS        %s%s", terminal_cursor_move, NORMAL, BOLD, NORMAL, version_name);
	if (show_host)
		responsively_printf(print_buf, "%s%s%sHOWOST      %s%s", terminal_cursor_move, NORMAL, BOLD, NORMAL, host_model);
	if (show_kernel)
		responsively_printf(print_buf, "%s%s%sKEWNEL      %s%s", terminal_cursor_move, NORMAL, BOLD, NORMAL, kernel);
	if (show_cpu)
		responsively_printf(print_buf, "%s%s%sCPUWU       %s%s", terminal_cursor_move, NORMAL, BOLD, NORMAL, cpu_model);

	// print the gpus
	if (show_gpu)
		for (int i = 0; gpu_model[i][0]; i++)
			responsively_printf(print_buf,
								"%s%s%sGPUWU       %s%s",
								terminal_cursor_move, NORMAL, BOLD, NORMAL, gpu_model[i]);

	// print ram to uptime and colors
	if (show_ram)
		responsively_printf(print_buf,
							"%s%s%sWAM         %s%i MiB/%i MiB",
							terminal_cursor_move, NORMAL, BOLD, NORMAL, (ram_used), ram_total);
	if (show_resolution)
		if (screen_width != 0 || screen_height != 0)
			responsively_printf(print_buf,
								"%s%s%sRESOWUTION%s  %dx%d",
								terminal_cursor_move, NORMAL, BOLD, NORMAL, screen_width, screen_height);
	if (show_shell)
		responsively_printf(print_buf,
							"%s%s%sSHEWW       %s%s",
							terminal_cursor_move, NORMAL, BOLD, NORMAL, shell);
#ifdef __APPLE__
	if (show_pkgs)
		system("ls $(brew --cellar) | wc -l | awk -F' ' '{print \"  \x1b[34mw         w     \x1b[0m\x1b[1mPKGS\x1b[0m        \"$1 \" (brew)\"}'");
#else
	if (show_pkgs)
		responsively_printf(print_buf,
							"%s%s%sPKGS        %s%s%d: %s",
							terminal_cursor_move, NORMAL, BOLD, NORMAL, NORMAL, pkgs, pkgman_name);
#endif
	if (show_uptime)
	{
		if (uptime == 0)
		{

#ifdef __APPLE__
			uptime = uptime_mac();
#else
#ifdef __FREEBSD__
			uptime = uptime_freebsd();
#else
#ifdef __WINDOWS__
			uptime = GetTickCount() / 1000;
#else  // __WINDOWS__
			uptime = sys.uptime;
#endif // __WINDOWS__
#endif
#endif
		}
		switch (uptime)
		{
		case 0 ... 3599:
			responsively_printf(print_buf,
								"%s%s%sUWUPTIME    %s%lim",
								terminal_cursor_move, NORMAL, BOLD, NORMAL, uptime / 60 % 60);
			break;
		case 3600 ... 86399:
			responsively_printf(print_buf,
								"%s%s%sUWUPTIME    %s%lih, %lim",
								terminal_cursor_move, NORMAL, BOLD, NORMAL, uptime / 3600, uptime / 60 % 60);
			break;
		default:
			responsively_printf(print_buf,
								"%s%s%sUWUPTIME    %s%lid, %lih, %lim",
								terminal_cursor_move, NORMAL, BOLD, NORMAL, uptime / 86400, uptime / 3600 % 24, uptime / 60 % 60);
		}
	}
	if (show_colors)
		printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
			   terminal_cursor_move, BOLD, BLACK, BLOCK_CHAR, BLOCK_CHAR, RED, BLOCK_CHAR, BLOCK_CHAR, GREEN, BLOCK_CHAR, BLOCK_CHAR, YELLOW, BLOCK_CHAR, BLOCK_CHAR, BLUE, BLOCK_CHAR, BLOCK_CHAR, MAGENTA, BLOCK_CHAR, BLOCK_CHAR, CYAN, BLOCK_CHAR, BLOCK_CHAR, WHITE, BLOCK_CHAR, BLOCK_CHAR, NORMAL);
}

void write_cache()
{
	char cache_file[512];
	sprintf(cache_file, "%s/.cache/uwufetch.cache", getenv("HOME"));
	FILE *cache_fp = fopen(cache_file, "w");
	if (cache_fp == NULL)
		return;
		// writing all info to the cache file
#ifdef __APPLE__
	uptime = uptime_mac();
#else
#ifdef __FREEBSD__
	uptime = uptime_freebsd();
#else
#ifndef __WINDOWS__
	uptime = sys.uptime;
#endif // __WINDOWS__
#endif
#endif
	fprintf(cache_fp, "user=%s\nhost=%s\nversion_name=%s\nhost_model=%s\nkernel=%s\ncpu=%s\nscreen_width=%d\nscreen_height=%d\nshell=%s\npkgs=%d\npkgman_name=%s\n", user, host, version_name, host_model, kernel, cpu_model, screen_width, screen_height, shell, pkgs, pkgman_name);

	for (int i = 0; gpu_model[i][0]; i++)
		fprintf(cache_fp, "gpu=%s\n", gpu_model[i]);

#ifdef __APPLE__
		/* char brew_command[2048];
	sprintf(brew_command, "ls $(brew --cellar) | wc -l | awk -F' ' '{print \"  \x1b[34mw         w     \x1b[0m\x1b[1mPKGS\x1b[0m        \"$1 \" (brew)\"}' > %s", cache_file);
	system(brew_command); */
#endif
	fclose(cache_fp);
	return;
}

// return whether the cache file is found
int read_cache()
{
	char cache_file[512];
	sprintf(cache_file, "%s/.cache/uwufetch.cache", getenv("HOME"));
	FILE *cache_fp = fopen(cache_file, "r");
	if (cache_fp == NULL)
		return 0;

	char line[256];

	int gpun = 0;

	while (fgets(line, sizeof(line), cache_fp))
	{
		sscanf(line, "user=%99[^\n]", user);
		sscanf(line, "host=%99[^\n]", host);
		sscanf(line, "version_name=%99[^\n]", version_name);
		sscanf(line, "host_model=%99[^\n]", host_model);
		sscanf(line, "kernel=%99[^\n]", kernel);
		sscanf(line, "cpu=%99[^\n]", cpu_model);
		if (sscanf(line, "gpu=%99[^\n]", gpu_model[gpun]) != 0)
			gpun++;
		sscanf(line, "screen_width=%i", &screen_width);
		sscanf(line, "screen_height=%i", &screen_height);
		sscanf(line, "shell=%99[^\n]", shell);
		sscanf(line, "pkgs=%i", &pkgs);
		sscanf(line, "pkgman_name=%99[^\n]", pkgman_name);
	}

	fclose(cache_fp);
	return 1;
}

void print_cache()
{
#ifndef __APPLE__
#ifndef __WINDOWS__
	sysinfo(&sys); // to get uptime
#endif			   // __WINDOWS__
	FILE *meminfo;

#ifdef __FREEBSD__
	meminfo = popen("LANG=EN_us freecolor -om 2> /dev/null", "r");
#else  // __FREEBSD__
	meminfo = popen("LANG=EN_us free -m 2> /dev/null", "r");
#endif // __FREEBSD__
	char line[256];
	while (fgets(line, sizeof(line), meminfo))
		// free command prints like this: "Mem:" total     used    free    shared  buff/cache      available
		sscanf(line, "Mem: %d %d", &ram_total, &ram_used);
	fclose(meminfo);
#elif defined(__WINDOWS__)
	// wmic OS get FreePhysicalMemory

	FILE *mem_used_fp;
	mem_used_fp = popen("wmic OS GET FreePhysicalMemory", "r");
	char mem_used_ch[2137];
	printf("\n\n\n\\\n");
	while (fgets(mem_used_ch, sizeof(mem_used_ch), mem_used_fp) != NULL)
	{
		printf("%s\n", mem_used_ch);
	}
	pclose(mem_used_fp);

	int mem_used = atoi(mem_used_ch);

	ram_used = mem_used / 1024;

#else // __APPLE__
	// Used ram
	FILE *mem_wired_fp, *mem_active_fp, *mem_compressed_fp;
	mem_wired_fp = popen("vm_stat | awk '/wired/ { printf $4 }' | cut -d '.' -f 1", "r");
	mem_active_fp = popen("vm_stat | awk '/active/ { printf $3 }' | cut -d '.' -f 1", "r");
	mem_compressed_fp = popen("vm_stat | awk '/occupied/ { printf $5 }' | cut -d '.' -f 1", "r");
	char mem_wired_ch[2137], mem_active_ch[2137], mem_compressed_ch[2137];
	while (fgets(mem_wired_ch, sizeof(mem_wired_ch), mem_wired_fp) != NULL)
	{
		while (fgets(mem_active_ch, sizeof(mem_active_ch), mem_active_fp) != NULL)
		{
			while (fgets(mem_compressed_ch, sizeof(mem_compressed_ch), mem_compressed_fp) != NULL)
			{
			}
		}
	}

	pclose(mem_wired_fp);
	pclose(mem_active_fp);
	pclose(mem_compressed_fp);

	int mem_wired = atoi(mem_wired_ch);
	int mem_active = atoi(mem_active_ch);
	int mem_compressed = atoi(mem_compressed_ch);

	// Total ram
	sysctlbyname("hw.memsize", &mem_buffer, &mem_buffer_len, NULL, 0);
	ram_used = ((mem_wired + mem_active + mem_compressed) * 4 / 1024);

#endif // __APPLE__
	print_ascii();
	print_info();
	return;
}

void get_info()
{					// get all necessary info
	char line[256]; // var to scan file lines

// terminal width used to truncate long names
#ifndef __WINDOWS__
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
	target_width = win.ws_col - 30;
#else  // __WINDOWS__
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	ws_col = csbi.srWindow.Right - csbi.srWindow.Left - 29;
	ws_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#endif // __WINDOWS__

	// os version, cpu and board info
	FILE *os_release = fopen("/etc/os-release", "r");
#ifndef __FREEBSD__
	FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
#else
	FILE *cpuinfo = popen("sysctl -a | egrep -i 'hw.model'", "r");
#endif
        FILE *host_model_info = fopen("/etc/hostname","r");
        if (!host_model_info)
          host_model_info = fopen("/sys/devices/virtual/dmi/id/board_name", "r"); // first try open the file
	if (!host_model_info) // if failed
          host_model_info = fopen("/sys/devices/virtual/dmi/id/product_name", "r"); // try open another file
	if(host_model_info) {
          fgets(line, 256, host_model_info);
          line[strlen(line)-1] = '\0';
          sprintf(host_model,"%s",line);
          fclose(host_model_info);
        }
#ifdef __WINDOWS__
	host_model_info = popen("wmic computersystem get model", "r");
	while (fgets(line, sizeof(line), host_model_info))
	{
		if (strstr(line, "Model") != 0)
			continue;
		else
		{
			sprintf(host_model, "%s", line);
			host_model[strlen(host_model) - 2] = '\0';
			break;
		}
	}
#elif defined(__FREEBSD__)
	host_model_info = popen("sysctl -a hw.hv_vendor", "r");
	while (fgets(line, sizeof(line), host_model_info))
		if (sscanf(line, "hw.hv_vendor: %[^\n]", host_model))
			break;
#endif // __WINDOWS__
	FILE *host_model_version = fopen("/sys/devices/virtual/dmi/id/product_version", "r");

	if (os_release)
	{ // get normal vars
		while (fgets(line, sizeof(line), os_release))
			if (sscanf(line, "\nID=\"%s\"", version_name) || sscanf(line, "\nID=%s", version_name))
				break;

		// trying to detect amogos because in its os-release file ID value is just "debian"
		if (strcmp(version_name, "debian") == 0)
		{
			DIR *amogos_plymouth = opendir("/usr/share/plymouth/themes/amogos");
			if (amogos_plymouth)
			{
				closedir(amogos_plymouth);
				sprintf(version_name, "amogos");
			}
		}
		if (host_model_info)
		{
			while (fgets(line, sizeof(line), host_model_info))
				if (sscanf(line, "%[^\n]", host_model))
					break;
			if (host_model_version)
			{
				char version[32];
				while (fgets(line, sizeof(line), host_model_version))
				{
					if (sscanf(line, "%[^\n]", version))
					{
						strcat(host_model, " ");
						strcat(host_model, version);
						break;
					}
				}
			}
		}
		while (fgets(line, sizeof(line), cpuinfo))
		{
#ifdef __FREEBSD__
			if (sscanf(line, "hw.model: %[^\n]", cpu_model))
#else
			if (sscanf(line, "model name    : %[^\n]", cpu_model))
				break;
#endif // __FREEBSD__
		}
		char *tmp_user = getenv("USER");
		if (tmp_user == NULL)
			sprintf(user, "%s", "");
		else
			sprintf(user, "%s", tmp_user);
		fclose(os_release);
	}
	else
	{ // try for android vars, next for macOS var, or unknown system
		DIR *system_app = opendir("/system/app/");
		DIR *system_priv_app = opendir("/system/priv-app/");
		DIR *library = opendir("/Library/");
		if (system_app && system_priv_app)
		{ // android
			closedir(system_app);
			closedir(system_priv_app);
			sprintf(version_name, "android");
			// android vars
			FILE *whoami = popen("whoami", "r");
			if (fscanf(whoami, "%s", user) == 3)
				sprintf(user, "unknown");
			fclose(whoami);
			host_model_info = popen("getprop ro.product.model", "r");
			while (fgets(line, sizeof(line), host_model_info))
				if (sscanf(line, "%[^\n]", host_model))
					break;
#ifndef __FREEBSD__
			while (fgets(line, sizeof(line), cpuinfo))
				if (sscanf(line, "Hardware        : %[^\n]", cpu_model))
					break;
#endif
		}
		else if (library) // macOS
		{
			closedir(library);
#ifdef __APPLE__
			sysctlbyname("machdep.cpu.brand_string", &cpu_buffer, &cpu_buffer_len, NULL, 0);

			sprintf(version_name, "macos");
			sprintf(cpu_model, "%s", cpu_buffer);
#endif
		}
		else
			sprintf(version_name, "unknown");
	}
#ifndef __FREEBSD__
	fclose(cpuinfo);
#endif
#ifndef __WINDOWS__
	gethostname(host, 256);
	// #endif // __WINDOWS__
	char *tmp_shell = getenv("SHELL");
	if (tmp_shell == NULL)
		sprintf(shell, "%s", "");
	else
		sprintf(shell, "%s", tmp_shell);
	if (strlen(shell) > 16)
		memmove(&shell, &shell[27], strlen(shell)); // android shell was too long, this works only for termux
#else
	cpuinfo = popen("wmic cpu get caption", "r");
	while (fgets(line, sizeof(line), cpuinfo))
	{
		if (strstr(line, "Caption") != 0)
			continue;
		else
		{
			sprintf(cpu_model, "%s", line);
			cpu_model[strlen(cpu_model) - 2] = '\0';
			break;
		}
	}
	FILE *user_host_fp = popen("wmic computersystem get username", "r");
	while (fgets(line, sizeof(line), user_host_fp))
	{
		if (strstr(line, "UserName") != 0)
			continue;
		else
		{
			sscanf(line, "%[^\\]%s", host, user);
			memmove(user, user + 1, sizeof(user) - 1);
			break;
		}
	}
	FILE *shell_fp = popen("powershell $PSVersionTable", "r");
	sprintf(shell, "PowerShell ");
	char tmp_shell[64];
	while (fgets(line, sizeof(line), shell_fp))
		if (sscanf(line, "PSVersion                      %s", tmp_shell) != 0)
			break;
	strcat(shell, tmp_shell);
#endif // __WINDOWS__
	// truncate CPU name
	truncate_name(cpu_model);

// system resources
#ifndef __WINDOWS__
	uname(&sys_var);
#endif // __WINDOWS__
#ifndef __APPLE__
#ifndef __FREEBSD__
#ifndef __WINDOWS__
	sysinfo(&sys); // somehow this function has to be called again in print_info()
#else			   // __WINDOWS__
	GetSystemInfo(&sys);
#endif			   // __WINDOWS__
#endif
#endif

#ifndef __WINDOWS__
	truncate_name(sys_var.release);
	sprintf(kernel, "%s %s %s", sys_var.sysname, sys_var.release, sys_var.machine);
	truncate_name(kernel);
#else  // __WINDOWS__
	sprintf(version_name, "windows");
	FILE *kernel_fp = popen("wmic computersystem get systemtype", "r");
	while (fgets(line, sizeof(line), kernel_fp))
	{
		if (strstr(line, "SystemType") != 0)
			continue;
		else
		{
			sprintf(kernel, "%s", line);
			kernel[strlen(kernel) - 2] = '\0';
			break;
		}
	}
	if (kernel_fp != NULL)
		pclose(kernel_fp);
#endif // __WINDOWS__

// ram
#ifndef __APPLE__
#ifdef __WINDOWS__
	FILE *mem_used_fp = popen("wmic os get freevirtualmemory", "r");
	FILE *mem_total_fp = popen("wmic os get totalvirtualmemorysize", "r");
	char mem_used_ch[2137] = {0}, mem_total_ch[2137] = {0};

	while (fgets(mem_total_ch, sizeof(mem_total_ch), mem_total_fp) != NULL)
	{
		if (strstr(mem_total_ch, "TotalVirtualMemorySize") != 0)
			continue;
		else if (strstr(mem_total_ch, "  ") == 0)
			continue;
		else
			ram_total = atoi(mem_total_ch) / 1024;
	}
	while (fgets(mem_used_ch, sizeof(mem_used_ch), mem_used_fp) != NULL)
	{
		if (strstr(mem_used_ch, "FreeVirtualMemory") != 0)
			continue;
		else if (strstr(mem_used_ch, "  ") == 0)
			continue;
		else
			ram_used = ram_total - (atoi(mem_used_ch) / 1024);
	}
	pclose(mem_used_fp);
	pclose(mem_total_fp);
#else
	FILE *meminfo;

#ifdef __FREEBSD__
	meminfo = popen("LANG=EN_us freecolor -om 2> /dev/null", "r");
#else
	meminfo = popen("LANG=EN_us free -m 2> /dev/null", "r");
#endif
	while (fgets(line, sizeof(line), meminfo))
		// free command prints like this: "Mem:" total     used    free    shared  buff/cache      available
		sscanf(line, "Mem: %d %d", &ram_total, &ram_used);
	fclose(meminfo);
#endif
#else
	// Used
	FILE *mem_wired_fp, *mem_active_fp, *mem_compressed_fp;
	mem_wired_fp = popen("vm_stat | awk '/wired/ { printf $4 }' | cut -d '.' -f 1", "r");
	mem_active_fp = popen("vm_stat | awk '/active/ { printf $3 }' | cut -d '.' -f 1", "r");
	mem_compressed_fp = popen("vm_stat | awk '/occupied/ { printf $5 }' | cut -d '.' -f 1", "r");
	char mem_wired_ch[2137], mem_active_ch[2137], mem_compressed_ch[2137];
	while (fgets(mem_wired_ch, sizeof(mem_wired_ch), mem_wired_fp) != NULL)
	{
		while (fgets(mem_active_ch, sizeof(mem_active_ch), mem_active_fp) != NULL)
		{
			while (fgets(mem_compressed_ch, sizeof(mem_compressed_ch), mem_compressed_fp) != NULL)
			{
			}
		}
	}

	pclose(mem_wired_fp);
	pclose(mem_active_fp);
	pclose(mem_compressed_fp);

	int mem_wired = atoi(mem_wired_ch);
	int mem_active = atoi(mem_active_ch);
	int mem_compressed = atoi(mem_compressed_ch);

	// Total
	sysctlbyname("hw.memsize", &mem_buffer, &mem_buffer_len, NULL, 0);

	ram_used = ((mem_wired + mem_active + mem_compressed) * 4 / 1024);
	ram_total = mem_buffer / 1024 / 1024;
#endif

	/* ---------- gpu ---------- */
	int gpun = 0; // number of the gpu that the program is searching for to put in the array
#ifndef __WINDOWS__
	setenv("LANG", "en_US", 1); // force language to english
#endif							// __WINDOWS__
	FILE *gpu;
#ifndef __WINDOWS__
	gpu = popen("lshw -class display 2> /dev/null", "r");

	// add all gpus to the array gpu_model
	while (fgets(line, sizeof(line), gpu))
		if (sscanf(line, "    product: %[^\n]", gpu_model[gpun]))
			gpun++;
#endif // __WINDOWS__

	if (strlen(gpu_model[0]) < 2)
	{
		// get gpus with lspci command
		if (strcmp(version_name, "android") != 0)
		{
#ifndef __APPLE__
#ifdef __WINDOWS__
			gpu = popen("wmic PATH Win32_VideoController GET Name", "r");
#else
			gpu = popen("lspci -mm 2> /dev/null | grep \"VGA\" | awk -F '\"' '{print $4 $5 $6}'", "r");
#endif
#else
			gpu = popen("system_profiler SPDisplaysDataType | awk -F ': ' '/Chipset Model: /{ print $2 }'", "r");
#endif
		}
		else
			gpu = popen("getprop ro.hardware.vulkan 2> /dev/null", "r");
	}

	// get all the gpus
	while (fgets(line, sizeof(line), gpu))
	{
		if (strstr(line, "Name"))
			continue;
		else if (strlen(line) == 2)
			continue;
		// ^^^ for windows
		else if (sscanf(line, "%[^\n]", gpu_model[gpun]))
			gpun++;
	}
	fclose(gpu);

	// truncate GPU name and remove square brackets
	for (int i = 0; i < gpun; i++)
	{
		remove_brackets(gpu_model[i]);
		truncate_name(gpu_model[i]);
	}

// Resolution
#ifndef __WINDOWS__
	FILE *resolution = popen("xwininfo -root 2> /dev/null | grep -E 'Width|Height'", "r");
	while (fgets(line, sizeof(line), resolution))
	{
		sscanf(line, "  Width: %d", &screen_width);
		sscanf(line, "  Height: %d", &screen_height);
	}
#endif // __WINDOWS__

	if (strcmp(version_name, "windows"))
		terminal_cursor_move = "\033[21C";

	// package count
	pkgs = pkgman();

	uwu_kernel();

	for (int i = 0; gpu_model[i][0]; i++)
		uwu_hw(gpu_model[i]);
	uwu_hw(cpu_model);
	uwu_hw(host_model);
}

void list(char *arg)
{ // prints distribution list
	// distributions are listed by distribution branch
	// to make the output easier to understand by the user.
	printf("%s -d <options>\n"
		   "  Available distributions:\n"
		   "    %sArch linux %sbased:\n"
		   "      %sarch, arcolinux, %sartix, endeavouros %smanjaro, manjaro-arm, %sxerolinux\n\n"
		   "    %sDebian/%sUbuntu %sbased:\n"
		   "      %samogos, debian, %slinuxmint, neon %spop, %sraspbian %subuntu\n\n"
		   "    %sBSD %sbased:\n"
		   "      %sfreebsd, %sopenbsd, %sm%sa%sc%so%ss\n\n"
		   "    %sOther/spare distributions:\n"
		   "      %salpine, %sfedora, %sgentoo, %sslackware, %ssolus, %svoid, opensuse-leap, android, %sgnu, guix, %swindows, %sunknown\n\n",
		   arg,
		   BLUE, NORMAL, BLUE, MAGENTA, GREEN, BLUE,							  // Arch based colors
		   RED, YELLOW, NORMAL, RED, GREEN, BLUE, RED, YELLOW,					  // Debian based colors
		   RED, NORMAL, RED, YELLOW, GREEN, YELLOW, RED, PINK, BLUE,			  // BSD colors
		   NORMAL, BLUE, BLUE, PINK, MAGENTA, WHITE, GREEN, YELLOW, BLUE, WHITE); // Other/spare distributions colors
}

/*
 This replaces all terms in a string with another term.
replace("Hello World!", "World", "everyone")
 This returns "Hello everyone!".
*/
void replace(char *original, char *search, char *replacer)
{
	char *ch;
	char buffer[1024];
	while ((ch = strstr(original, search)))
	{
		ch = strstr(original, search);
		strncpy(buffer, original, ch - original);
		buffer[ch - original] = 0;
		sprintf(buffer + (ch - original), "%s%s", replacer, ch + strlen(search));

		original[0] = 0;
		strcpy(original, buffer);
	}
}

void replace_ignorecase(char *original, char *search, char *replacer)
{
	char *ch;
	char buffer[1024];
#ifndef __WINDOWS__
	while ((ch = strcasestr(original, search)))
#else
	while ((ch = strstr(original, search)))
#endif // __WINDOWS__
	{
		strncpy(buffer, original, ch - original);
		buffer[ch - original] = 0;
		sprintf(buffer + (ch - original), "%s%s", replacer, ch + strlen(search));

		original[0] = 0;
		strcpy(original, buffer);
	}
}

void print_ascii()
{ // prints logo (as ascii art) of the given system. distributions listed alphabetically.
	printf("\n");
	FILE *file;
	char ascii_file[1024];
	// First tries to get ascii art file from local directory. Good when modifying these files.
	sprintf(ascii_file, "./res/ascii/%s.txt", version_name);
	file = fopen(ascii_file, "r");
	// Now tries to get file from normal directory
	if (!file)
	{
		if (strcmp(version_name, "android") == 0)
		{
			sprintf(ascii_file, "/data/data/com.termux/files/usr/lib/uwufetch/ascii/%s.txt", version_name);
		}
		else
		{
			sprintf(ascii_file, "/usr/lib/uwufetch/ascii/%s.txt", version_name);
		}
		file = fopen(ascii_file, "r");
		if (!file)
		{
			// Prevent infinite loops
			if (strcmp(version_name, "unknown") == 0)
			{
				printf("No\nunknown\nascii\nfile\n\n\n\n");
				return;
			}
			sprintf(version_name, "unknown");
			return print_ascii();
		}
	}
	char line[256];
	while (fgets(line, 256, file))
	{
		replace(line, "{NORMAL}", NORMAL);
		replace(line, "{BOLD}", BOLD);
		replace(line, "{BLACK}", BLACK);
		replace(line, "{RED}", RED);
		replace(line, "{GREEN}", GREEN);
		replace(line, "{SPRING_GREEN}", SPRING_GREEN);
		replace(line, "{YELLOW}", YELLOW);
		replace(line, "{BLUE}", BLUE);
		replace(line, "{MAGENTA}", MAGENTA);
		replace(line, "{CYAN}", CYAN);
		replace(line, "{WHITE}", WHITE);
		replace(line, "{PINK}", PINK);
		replace(line, "{LPINK}", LPINK);
// For manjaro and amogos and windows
#ifdef __WINDOWS__
		replace(line, "{BLOCK}", "\xdc");
		replace(line, "{BLOCK_VERTICAL}", "\xdb");
#else  // __WINDOWS__
		replace(line, "{BLOCK}", "\u2584");
		replace(line, "{BLOCK_VERTICAL}", "\u2587");
#endif // __WINDOWS__
		replace(line, "{BACKGROUND_GREEN}", "\e[0;42m");
		replace(line, "{BACKGROUND_RED}", "\e[0;41m");
		replace(line, "{BACKGROUND_WHITE}", "\e[0;47m");
		printf("%s", line);
	}
	// Always set color to NORMAL, so there's no need to do this in every ascii file.
	printf(NORMAL);
	fclose(file);
}

void print_image()
{ // prints logo (as an image) of the given system. distributions listed alphabetically.
	char command[256];
	if (strlen(image_name) > 1)
		sprintf(command, "viu -t -w 18 -h 8 %s 2> /dev/null", image_name);
	else
	{
		if (strcmp(version_name, "android") == 0)
			sprintf(command, "viu -t -w 18 -h 8 /data/data/com.termux/files/usr/lib/uwufetch/%s.png 2> /dev/null", version_name);
		else
			sprintf(command, "viu -t -w 18 -h 8 /usr/lib/uwufetch/%s.png 2> /dev/null", version_name);
	}
	printf("\n");
	if (system(command) != 0)
	{ // if viu is not installed or the image is missing
		printf("\033[0E\033[3C%s\n"
			   "   There was an\n"
			   "    error: viu\n"
			   " is not installed\n"
			   "   or the image\n"
			   "   is not found\n"
			   "  Read IMAGES.md\n"
			   "   for more info.\n\n",
			   RED);
	}
}

void usage(char *arg)
{
	printf("Usage: %s <args>\n"
		   "    -a, --ascii         prints logo as ascii text (default)\n"
		   "    -c  --config        use custom config path\n"
		   "    -d, --distro        lets you choose the logo to print\n"
		   "    -h, --help          prints this help page\n"
		   "    -i, --image         prints logo as image and use a custom image if provided\n"
		   "                        %sworks in most terminals\n"
		   "                        read README.md for more info%s\n"
		   "    -l, --list          lists all supported distributions\n"
		   "    -w, --write-cache   writes to the cache file (~/.cache/uwufetch.cache)\n"
		   "    using the cache     set $UWUFETCH_CACHE_ENABLED to TRUE, true or 1\n",
		   arg, BLUE, NORMAL);
}

#ifdef __WINDOWS__
// windows sucks and hasn't a strstep, so I copied one from https://stackoverflow.com/questions/8512958/is-there-a-windows-variant-of-strsep-function
char *strsep(char **stringp, const char *delim)
{
	char *start = *stringp;
	char *p;

	p = (start != NULL) ? strpbrk(start, delim) : NULL;

	if (p == NULL)
		*stringp = NULL;
	else
	{
		*p = '\0';
		*stringp = p + 1;
	}

	return start;
}
#endif

void uwu_kernel()
{
#define KERNEL_TO_UWU(str, original, uwufied) \
	if (strcmp(str, original) == 0)           \
	sprintf(str, "%s", uwufied)

	char *temp_kernel = kernel;
	char *token;
	char splitted[16][128] = {};

	int count = 0;
	while ((token = strsep(&temp_kernel, " ")))
	{
		strcpy(splitted[count], token);
		count++;
	}
	strcpy(kernel, "");
	for (int i = 0; i < 16; i++)
	{
		// kernel name
		KERNEL_TO_UWU(splitted[i], "Linux", "Linuwu");
		else KERNEL_TO_UWU(splitted[i], "linux", "linuwu");
		else KERNEL_TO_UWU(splitted[i], "alpine", "Nyalpine");
		else KERNEL_TO_UWU(splitted[i], "amogos", "AmogOwOS");
		else KERNEL_TO_UWU(splitted[i], "arch", "Nyarch Linuwu");
		else KERNEL_TO_UWU(splitted[i], "artix", "Nyartix Linuwu");
		else KERNEL_TO_UWU(splitted[i], "debian", "Debinyan");
		else KERNEL_TO_UWU(splitted[i], "endeavouros", "endeavOwO");
		else KERNEL_TO_UWU(splitted[i], "EndeavourOS", "endeavOwO");
		else KERNEL_TO_UWU(splitted[i], "fedora", "Fedowa");
		else KERNEL_TO_UWU(splitted[i], "gentoo", "GentOwO");
		else KERNEL_TO_UWU(splitted[i], "gnu", "gnUwU");
		else KERNEL_TO_UWU(splitted[i], "guix", "gnUwU gUwUix");
		else KERNEL_TO_UWU(splitted[i], "linuxmint", "LinUWU Miwint");
		else KERNEL_TO_UWU(splitted[i], "manjaro", "Myanjawo");
		else KERNEL_TO_UWU(splitted[i], "manjaro-arm", "Myanjawo AWM");
		else KERNEL_TO_UWU(splitted[i], "neon", "KDE NeOwOn");
		else KERNEL_TO_UWU(splitted[i], "nixos", "nixOwOs");
		else KERNEL_TO_UWU(splitted[i], "opensuse-leap", "OwOpenSUSE Leap");
		else KERNEL_TO_UWU(splitted[i], "opensuse-tumbleweed", "OwOpenSUSE Tumbleweed");
		else KERNEL_TO_UWU(splitted[i], "pop", "PopOwOS");
		else KERNEL_TO_UWU(splitted[i], "raspbian", "RaspNyan");
		else KERNEL_TO_UWU(splitted[i], "slackware", "Swackwawe");
		else KERNEL_TO_UWU(splitted[i], "solus", "sOwOlus");
		else KERNEL_TO_UWU(splitted[i], "ubuntu", "Uwuntu");
		else KERNEL_TO_UWU(splitted[i], "void", "OwOid");
		else KERNEL_TO_UWU(splitted[i], "xerolinux", "xuwulinux");
		else KERNEL_TO_UWU(splitted[i], "android", "Nyandroid"); // android at the end because it could be not considered as an actual distribution of gnu/linux

		// BSD
		else KERNEL_TO_UWU(splitted[i], "freebsd", "FweeBSD");
		else KERNEL_TO_UWU(splitted[i], "openbsd", "OwOpenBSD");

		else KERNEL_TO_UWU(splitted[i], "macos", "macOwOS");

		// Windows
		else KERNEL_TO_UWU(splitted[i], "windows", "WinyandOwOws");

		if (i != 0)
			strcat(kernel, " ");
		strcat(kernel, splitted[i]);
	}
#undef KERNEL_TO_UWU
}

void uwu_hw(char *hwname)
{
#define HW_TO_UWU(original, uwuified) \
	replace_ignorecase(hwname, original, uwuified);

	HW_TO_UWU("lenovo", "LenOwO")
	HW_TO_UWU("cpu", "CC\bPUwU"); // for some reasons this caused a segfault, using a \b char fixes it
	HW_TO_UWU("gpu", "GG\bPUwU")
	HW_TO_UWU("graphics", "Gwaphics")
	HW_TO_UWU("corporation", "COwOpowation")
	HW_TO_UWU("nvidia", "NyaVIDIA")
	HW_TO_UWU("mobile", "Mwobile")
	HW_TO_UWU("intel", "Inteww")
	HW_TO_UWU("radeon", "Radenyan")
	HW_TO_UWU("geforce", "GeFOwOce")
	HW_TO_UWU("raspberry", "Nyasberry")
	HW_TO_UWU("broadcom", "Bwoadcom")
	HW_TO_UWU("motorola", "MotOwOwa")
	HW_TO_UWU("proliant", "ProLinyant")
	HW_TO_UWU("poweredge", "POwOwEdge")
	HW_TO_UWU("apple", "Nyaa\bpple")
	HW_TO_UWU("electronic", "ElectrOwOnic")

#undef HW_TO_UWU
}

void uwu_name()
{ // uwufies distro name

#define STRING_TO_UWU(original, uwufied)     \
	if (strcmp(version_name, original) == 0) \
	sprintf(version_name, "%s", uwufied)

	// linux
	STRING_TO_UWU("alpine", "Nyalpine");
	else STRING_TO_UWU("amogos", "AmogOwOS");
	else STRING_TO_UWU("arch", "Nyarch Linuwu");
	else STRING_TO_UWU("arcolinux", "ArcOwO Linuwu");
	else STRING_TO_UWU("artix", "Nyartix Linuwu");
	else STRING_TO_UWU("debian", "Debinyan");
	else STRING_TO_UWU("endeavouros", "endeavOwO");
	else STRING_TO_UWU("EndeavourOS", "endeavOwO");
	else STRING_TO_UWU("fedora", "Fedowa");
	else STRING_TO_UWU("gentoo", "GentOwO");
	else STRING_TO_UWU("gnu", "gnUwU");
	else STRING_TO_UWU("guix", "gnUwU gUwUix");
	else STRING_TO_UWU("linuxmint", "LinUWU Miwint");
	else STRING_TO_UWU("manjaro", "Myanjawo");
	else STRING_TO_UWU("manjaro-arm", "Myanjawo AWM");
	else STRING_TO_UWU("neon", "KDE NeOwOn");
	else STRING_TO_UWU("nixos", "nixOwOs");
	else STRING_TO_UWU("opensuse-leap", "OwOpenSUSE Leap");
	else STRING_TO_UWU("opensuse-tumbleweed", "OwOpenSUSE Tumbleweed");
	else STRING_TO_UWU("pop", "PopOwOS");
	else STRING_TO_UWU("raspbian", "RaspNyan");
	else STRING_TO_UWU("slackware", "Swackwawe");
	else STRING_TO_UWU("solus", "sOwOlus");
	else STRING_TO_UWU("ubuntu", "Uwuntu");
	else STRING_TO_UWU("void", "OwOid");
	else STRING_TO_UWU("xerolinux", "xuwulinux");
	else STRING_TO_UWU("android", "Nyandroid"); // android at the end because it could be not considered as an actual distribution of gnu/linux

	// BSD
	else STRING_TO_UWU("freebsd", "FweeBSD");
	else STRING_TO_UWU("openbsd", "OwOpenBSD");

	else STRING_TO_UWU("macos", "macOwOS");

	// Windows
	else STRING_TO_UWU("windows", "WinyandOwOws");

	else
	{
		sprintf(version_name, "%s", "unknown");
		if (ascii_image_flag == 1)
		{
			print_image();
			printf("\n");
		}
	}
#undef STRING_TO_UWU
}

void truncate_name(char *name)
{
	char arr[target_width];

	for (int i = 0; i < target_width; i++)
		arr[i] = name[i];
	name = arr;
}

// remove square brackets (for gpu names)
void remove_brackets(char *str)
{
	int i = 0, j;
	while (i < (int)strlen(str))
	{
		if (str[i] == '[' || str[i] == ']')
		{
			for (j = i; j < (int)strlen(str); j++)
				str[j] = str[j + 1];
		}
		else
			i++;
	}
}
