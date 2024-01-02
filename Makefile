NAME = uwufetch
BIN_FILES = uwufetch.c
LIB_FILES = fetch.c
UWUFETCH_VERSION = $(shell git describe --tags)
COMMON_FLAGS = -pthread -DUWUFETCH_VERSION=\"$(UWUFETCH_VERSION)\"
ALL_CFLAGS = -O3 $(COMMON_FLAGS) $(CFLAGS)
ALL_CFLAGS_DEBUG = -Wall -Wextra -g $(COMMON_FLAGS) -D__DEBUG__ $(CFLAGS)

INSTALL ?= install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644
prefix ?= /usr/local
exec_prefix ?= $(prefix)
bindir ?= $(exec_prefix)/bin
datarootdir ?= $(prefix)/share
sysconfdir ?= $(prefix)/etc
includedir ?= $(prefix)/include
libdir ?= $(exec_prefix)/lib
mandir ?= $(datarootdir)/man
man1dir ?= $(mandir)/man1
manext ?= .1

RELEASE_SCRIPTS = release_scripts/*.sh
ifeq ($(OS), Windows_NT)
	PLATFORM = $(OS)
else
	PLATFORM = $(shell uname)
endif
PLATFORM_ABBR = $(PLATFORM)

ifeq ($(PLATFORM), Linux)
	PLATFORM_ABBR = linux
	ifeq ($(shell uname -o), Android)
		COMMON_FLAGS += -DPKGPATH=\"/data/data/com.termux/files/usr/bin/\"
		PLATFORM_ABBR	= android
	endif
else ifeq ($(PLATFORM), Darwin)
	PLATFORM_ABBR = macos
else ifeq ($(PLATFORM), FreeBSD)
	COMMON_FLAGS += -D__FREEBSD__ -D__BSD__
	PLATFORM_ABBR = freebsd
else ifeq ($(PLATFORM), OpenBSD)
	COMMON_FLAGS += -D__OPENBSD__ -D__BSD__
	PLATFORM_ABBR = openbsd
else ifeq ($(PLATFORM), Windows_NT)
	CC		= gcc
	bindir 		= "C:\Program Files"
	libdir		=
	includedir	=
	man1dir		=
	RELEASE_SCRIPTS = release_scripts/*.ps1
	PLATFORM_ABBR	= win64
	EXT		= .exe
else ifeq ($(PLATFORM), linux4win)
	CC		= x86_64-w64-mingw32-gcc
	COMMON_FLAGS	+= -D_WIN32
	bindir		=
	libdir		=
	includedir	=
	man1dir		=
	RELEASE_SCRIPTS = release_scripts/*.ps1
	PLATFORM_ABBR	= win64
	EXT		= .exe
endif

all: $(NAME) libs
libs: libfetch.a libfetch.so

$(NAME): $(BIN_FILES) libfetch.a
	$(CC) $(ALL_CFLAGS) $(LDFLAGS) -o $@ $^
fetch.o: $(LIB_FILES)
	$(CC) -fPIC -c $(ALL_CFLAGS) -o $@ $^
libfetch.so: fetch.o
	$(CC) $(ALL_CFLAGS) -shared -Wl,-soname,$@.$(UWUFETCH_VERSION) $(LDFLAGS) -o $@ $^
libfetch.a: fetch.o
	$(AR) rcs $@ $^

release: all man
	mkdir -pv $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp $(RELEASE_SCRIPTS) $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp -r res $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp $(NAME)$(EXT) $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp $(NAME)$(manext) $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp lib$(LIB_FILES:.c=.so) $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp $(LIB_FILES:.c=.h) $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp default.config $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
ifeq ($(PLATFORM), linux4win)
	zip -9r $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR).zip $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
else
	tar -czf $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR).tar.gz $(NAME)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
endif

debug: ALL_CFLAGS = $(ALL_CFLAGS_DEBUG)
debug: $(NAME)
	./$(NAME) $(ARGS)

# Make sure all installation directories (e.g. $(bindir))
# actually exist by making them if necessary.
installdirs:
	$(INSTALL) -d \
	$(DESTDIR)$(bindir) $(DESTDIR)$(datadir) \
	$(DESTDIR)$(libdir)/$(NAME)/ascii $(DESTDIR)$(includedir) \
	$(DESTDIR)$(man1dir) $(DESTDIR)$(sysconfdir)/$(NAME)

install: $(NAME) libfetch.so installdirs man
	$(INSTALL_PROGRAM) $(NAME) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) libfetch.so $(DESTDIR)$(libdir)/libfetch.so.$(UWUFETCH_VERSION)
	$(INSTALL_DATA) res/*.png $(DESTDIR)$(libdir)/$(NAME)
	$(INSTALL_DATA) res/ascii/* $(DESTDIR)$(libdir)/$(NAME)/ascii
	$(INSTALL_DATA) $(LIB_FILES:.c=.h) $(DESTDIR)$(includedir)
	$(INSTALL_DATA) default.config $(DESTDIR)$(sysconfdir)/$(NAME)/config
	$(INSTALL_DATA) $(NAME)$(manext) $(DESTDIR)$(man1dir)

uninstall:
	$(RM) $(DESTDIR)$(bindir)/$(NAME)
	$(RM) -r $(DESTDIR)$(libdir)/$(NAME)
	$(RM) $(DESTDIR)$(libdir)/libfetch.so.$(UWUFETCH_VERSION)
	$(RM) $(DESTDIR)$(includedir)/$(LIB_FILES:.c=.h)
	$(RM) -r $(DESTDIR)$(sysconfdir)/$(NAME)
	$(RM) $(DESTDIR)$(man1dir)/$(NAME)$(manext)

clean:
	$(RM) -r $(NAME) $(NAME)_* *.o *.so *.a *.exe

ascii_debug: build
ascii_debug:
	ls res/ascii/$(ASCII).txt | entr -c ./$(NAME) -d $(ASCII)

man:
	sed -i -e "s/{DATE}/$(shell date '+%d %B %Y')/g" \
		-e "s/{UWUFETCH_VERSION}/$(UWUFETCH_VERSION)/g" $(NAME)$(manext)

man_debug:
	@clear
	man -P cat ./$(NAME)$(manext)
