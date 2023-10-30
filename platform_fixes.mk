# platform (os) specific variable modifications

ifeq ($(PLATFORM), Linux)
	PREFIX		= bin
	LIBDIR		= lib
	INCDIR		= include
	ETC_DIR		= /etc
	MANDIR		= share/man/man1
	PLATFORM_ABBR = linux
	ifeq ($(shell uname -o), Android)
		CFLAGS				+= -D__ANDROID__
		CFLAGS_DEBUG	+= -D__ANDROID__
		DEST_DIR				= /data/data/com.termux/files/usr
		ETC_DIR				= $(DEST_DIR)/etc
		PLATFORM_ABBR	= android
	endif
else ifeq ($(PLATFORM), Darwin)
	PREFIX		= local/bin
	LIBDIR		= local/lib
	INCDIR		= local/include
	ETC_DIR		= /etc
	MANDIR		= local/share/man/man1
	PLATFORM_ABBR = macos
else ifeq ($(PLATFORM), FreeBSD)
	CFLAGS		+= -D__FREEBSD__ -D__BSD__
	CFLAGS_DEBUG += -D__FREEBSD__ -D__BSD__
	PREFIX		= bin
	LIBDIR		= lib
	INCDIR		= include
	ETC_DIR		= /etc
	MANDIR		= share/man/man1
	PLATFORM_ABBR = freebsd
else ifeq ($(PLATFORM), OpenBSD)
	CFLAGS		+= -D__OPENBSD__ -D__BSD__
	CFLAGS_DEBUG += -D__OPENBSD__ -D__BSD__
	PREFIX		= bin
	LIBDIR		= lib
	INCDIR		= include
	ETC_DIR		= /etc
	MANDIR		= share/man/man1
	PLATFORM_ABBR = openbsd
else ifeq ($(PLATFORM), Windows_NT)
	CC					= gcc
	PREFIX			= "C:\Program Files"
	LIBDIR			=
	INCDIR			=
	MANDIR			=
	RELEASE_SCRIPTS = release_scripts/*.ps1
	PLATFORM_ABBR	= win64
	EXT				= .exe
else ifeq ($(PLATFORM), linux4win)
	CC				= x86_64-w64-mingw32-gcc
	PREFIX			=
	CFLAGS			+= -D_WIN32
	LIBDIR			=
	INCDIR		    =
	MANDIR			=
	RELEASE_SCRIPTS = release_scripts/*.ps1
	PLATFORM_ABBR	= win64
	EXT				= .exe
endif

