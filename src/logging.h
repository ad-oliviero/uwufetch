#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <stdio.h>

#ifdef LOGGING_ENABLED

enum LOG_LEVELS {
	LEVEL_DISABLE,
	LEVEL_ERROR,
	LEVEL_WARNING,
	LEVEL_INFO,
	LEVEL_VAR,
	LEVEL_MAX = LEVEL_VAR
};
	#define SET_LOG_LEVEL(level, additional_info) set_logging_level(level, additional_info)
	#define LOG_I(format, ...) \
		if (logging_level >= LEVEL_INFO) LOG(LEVEL_INFO, format, ##__VA_ARGS__)
	#define LOG_W(format, ...) \
		if (logging_level >= LEVEL_WARNING) LOG(LEVEL_WARNING, format, ##__VA_ARGS__)
	#define LOG_E(format, ...) \
		if (logging_level >= LEVEL_ERROR) LOG(LEVEL_ERROR, format, ##__VA_ARGS__)
	#define LOG_V(var)                       \
		if (logging_level >= LEVEL_VAR) {      \
			static char format[1024] = "";       \
			sprintf(format, "%s = %s", #var,     \
							_Generic((var), int          \
											 : "%d", float       \
											 : "%f", char*       \
											 : "\"%s\"", default \
											 : "%p"));           \
			LOG(LEVEL_VAR, format, var);         \
		}
	#define LOG(type, format, ...)                                  \
		{                                                             \
			static char buf[2048] = "";                                 \
			if (sizeof(#__VA_ARGS__) == sizeof(""))                     \
				sprintf(buf, "%s", format);                               \
			else                                                        \
				sprintf(buf, format, ##__VA_ARGS__);                      \
			fprintf(stderr, "[%s]: %s in %s:%d: %s\n",                  \
							type == LEVEL_INFO			? "\033[32mINFO\033[0m"     \
							: type == LEVEL_WARNING ? "\033[33mWARNING\033[0m"  \
							: type == LEVEL_ERROR		? "\033[31mERROR\033[0m"    \
							: type == LEVEL_VAR			? "\033[37mVARIABLE\033[0m" \
																			: "",                           \
							__func__, __FILE__, __LINE__, buf);                 \
		}
static int logging_level = 0;
static __attribute__((unused)) void set_logging_level(int level, char* additional_info) {
	if (level < LEVEL_DISABLE || level > LEVEL_MAX) {
		logging_level = LEVEL_ERROR;
		LOG_E("%s; invalid logging level: %d", additional_info, level);
		return;
	}
	logging_level = level;
	LOG(LEVEL_INFO, "%s; logging level set to %d", additional_info, level);
}
#else
	#define SET_LOG_LEVEL(level, additional_info)
	#define LOG_I(format, ...)
	#define LOG_E(format, ...)
	#define LOG_V(var)
	#define LOG(type, format, ...)
#endif // LOGGING_ENABLED

#endif // _LOGGING_H_
