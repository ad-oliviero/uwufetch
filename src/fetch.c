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

#ifdef __APPLE__
	#include <TargetConditionals.h> // for checking iOS
#endif
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if defined(__APPLE__) || defined(__BSD__)
	#include <sys/sysctl.h>
	#if defined(__OPENBSD__)
		#include <sys/time.h>
	#else
		#include <time.h>
	#endif // defined(__OPENBSD__)
#else		 // defined(__APPLE__) || defined(__BSD__)
	#ifdef __BSD__
	#else // defined(__BSD__) || defined(_WIN32)
		#ifndef _WIN32
			#ifndef __OPENBSD__
				#include <sys/sysinfo.h>
			#else	 // __OPENBSD__
			#endif // __OPENBSD__
		#else		 // _WIN32
			#include <sysinfoapi.h>
		#endif // _WIN32
	#endif	 // defined(__BSD__) || defined(_WIN32)
#endif		 // defined(__APPLE__) || defined(__BSD__)
#ifndef _WIN32
	#include <sys/ioctl.h>
	#include <sys/utsname.h>
	#include <pthread.h> // linux only right now
#else									 // _WIN32
	#include <windows.h>
CONSOLE_SCREEN_BUFFER_INFO csbi;
#endif // _WIN32

#define LIBFETCH_INTERNAL // to do certain things only when included from the library itself
#include "fetch.h"
#define BUFFER_SIZE 256
#ifdef __DEBUG__
	#define LOGGING_ENABLED
#endif
#include "logging.h"

// Retrieves system information
struct info* get_all() {
	struct info* user_info = (struct info*)malloc(sizeof(struct info));
	LOG_I("Getting system information");
	return user_info;
}
