# platform (os) specific variable modifications


ifeq ($(PLATFORM), Linux)
	PLATFORM_ABBR = linux
	ifeq ($(shell uname -o), Android)
		CFLAGS += -D__ANDROID__
		CFLAGS_DEBUG += -D__ANDROID__
		PREFIX_DIR = /data/data/com.termux/files/usr
		PLATFORM_ABBR	= android
	endif
else ifeq ($(PLATFORM), Darwin)
	USR_DIR = $(PREFIX_DIR)/usr/local
	BIN_DIR = $(USR_DIR)/bin
	LIB_DIR = $(USR_DIR)/lib
	MAN_DIR = $(USR_DIR)/share/man/man1
	INC_DIR = $(USR_DIR)/include
	PLATFORM_ABBR = macos
else ifeq ($(PLATFORM), FreeBSD)
	CFLAGS += -D__FREEBSD__ -D__BSD__
	CFLAGS_DEBUG += -D__FREEBSD__ -D__BSD__
	PLATFORM_ABBR = freebsd
else ifeq ($(PLATFORM), OpenBSD)
	CFLAGS += -D__OPENBSD__ -D__BSD__
	CFLAGS_DEBUG += -D__OPENBSD__ -D__BSD__
	PLATFORM_ABBR = openbsd
else ifeq ($(PLATFORM), Windows_NT)
	CC = gcc
	PREFIX_DIR = "C:\Program Files\uwufetch"
	USR_DIR = $(PREFIX_DIR)\usr
	ETC_DIR = $(PREFIX_DIR)\etc
	BIN_DIR = $(USR_DIR)\bin
	LIB_DIR = $(USR_DIR)\lib
	MAN_DIR = $(USR_DIR)\share\man\man1
	INC_DIR = $(USR_DIR)\include
	RELEASE_SCRIPTS = release_scripts\*.ps1
	PLATFORM_ABBR	= win64
	EXT = .exe
else ifeq ($(PLATFORM), linux4win)
	CC				= x86_64-w64-mingw32-gcc
	CFLAGS			+= -D_WIN32
	RELEASE_SCRIPTS = release_scripts/*.ps1
	PLATFORM_ABBR	= win64
	EXT				= .exe
endif

