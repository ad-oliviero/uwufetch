NAME = uwufetch
BIN_FILES = uwufetch.c
LIB_FILES = fetch.c
UWUFETCH_VERSION = $(shell git describe --tags)
CFLAGS = -O3 -pthread -DUWUFETCH_VERSION=\"$(UWUFETCH_VERSION)\"
CFLAGS_DEBUG = -Wall -Wextra -g -pthread -DUWUFETCH_VERSION=\"$(UWUFETCH_VERSION)\" -D__DEBUG__
CC = cc
AR = ar
DESTDIR = /usr
RELEASE_SCRIPTS = release_scripts/*.sh
ifeq ($(OS), Windows_NT)
	PLATFORM = $(OS)
else
	PLATFORM = $(shell uname)
endif
PLATFORM_ABBR = $(PLATFORM)

ifeq ($(PLATFORM), Linux)
	PREFIX		= bin
	LIBDIR		= lib
	INCDIR		= include
	ETC_DIR		= /etc
	MANDIR		= share/man/man1
	PLATFORM_ABBR = linux
	ifeq ($(shell uname -o), Android)
		CFLAGS				+= -DPKGPATH=\"/data/data/com.termux/files/usr/bin/\"
		CFLAGS_DEBUG	+= -DPKGPATH=\"/data/data/com.termux/files/usr/bin/\"
		DESTDIR				= /data/data/com.termux/files/usr
		ETC_DIR				= $(DESTDIR)/etc
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

build: $(BIN_FILES) lib
	$(CC) $(CFLAGS) -o $(NAME) $(BIN_FILES) lib$(LIB_FILES:.c=.a)

lib: $(LIB_FILES)
	$(CC) $(CFLAGS) -fPIC -c -o $(LIB_FILES:.c=.o) $(LIB_FILES)
	$(AR) rcs lib$(LIB_FILES:.c=.a) $(LIB_FILES:.c=.o)
	$(CC) $(CFLAGS) -shared -o lib$(LIB_FILES:.c=.so) $(LIB_FILES:.c=.o)

release: build man
	mkdir -pv $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp $(RELEASE_SCRIPTS) $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp -r res $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp $(NAME)$(EXT) $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp $(NAME).1.gz $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp lib$(LIB_FILES:.c=.so) $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp $(LIB_FILES:.c=.h) $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp default.config $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
ifeq ($(PLATFORM), linux4win)
	zip -9r $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR).zip $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
else
	tar -czf $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR).tar.gz $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
endif

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: build
	./$(NAME) $(ARGS)

install: build man
	mkdir -pv $(DESTDIR)/$(PREFIX) $(DESTDIR)/$(LIBDIR)/$(NAME) $(DESTDIR)/$(MANDIR) $(ETC_DIR)/$(NAME)
	cp $(NAME) $(DESTDIR)/$(PREFIX)
	cp lib$(LIB_FILES:.c=.so) $(DESTDIR)/$(LIBDIR)
	cp $(LIB_FILES:.c=.h) $(DESTDIR)/$(INCDIR)
	cp -r res/* $(DESTDIR)/$(LIBDIR)/$(NAME)
	cp default.config $(ETC_DIR)/$(NAME)/config
	cp ./$(NAME).1.gz $(DESTDIR)/$(MANDIR)

uninstall:
	rm -f $(DESTDIR)/$(PREFIX)/$(NAME)
	rm -rf $(DESTDIR)/$(LIBDIR)/uwufetch
	rm -f $(DESTDIR)/$(LIBDIR)/lib$(LIB_FILES:.c=.so)
	rm -f $(DESTDIR)/include/$(LIB_FILES:.c=.h)
	rm -rf $(ETC_DIR)/uwufetch
	rm -f $(DESTDIR)/$(MANDIR)/$(NAME).1.gz

clean:
	rm -rf $(NAME) $(NAME)_* *.o *.so *.a *.exe

ascii_debug: build
ascii_debug:
	ls res/ascii/$(ASCII).txt | entr -c ./$(NAME) -d $(ASCII)

man:
	sed "s/{DATE}/$(shell date '+%d %B %Y')/g" $(NAME).1 | sed "s/{UWUFETCH_VERSION}/$(UWUFETCH_VERSION)/g" | gzip > $(NAME).1.gz

man_debug:
	@clear
	man -P cat ./$(NAME).1
