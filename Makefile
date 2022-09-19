NAME = uwufetch
BIN_FILES = uwufetch.c
LIB_FILES = fetch.c
UWUFETCH_VERSION = UWUFETCH_VERSION="\"$(shell git describe --tags)\""
CFLAGS = -O3 -D$(UWUFETCH_VERSION)
CFLAGS_DEBUG = -Wall -Wextra -g -pthread -D$(UWUFETCH_VERSION)
CC = cc
AR = ar
DESTDIR = /usr
PLATFORM = $(shell uname)

ifeq ($(PLATFORM), Linux)
	PREFIX		= bin
	LIBDIR		= lib
	ETC_DIR		= /etc
	MANDIR		= share/man/man1
	ifeq ($(shell uname -o), Android)
		DESTDIR	= /data/data/com.termux/files/usr
		ETC_DIR = $(DESTDIR)/etc
	endif
else ifeq ($(PLATFORM), Darwin)
	PREFIX		= local/bin
	LIBDIR		= local/lib
	ETC_DIR		= /etc
	MANDIR		= local/share/man/man1
else ifeq ($(PLATFORM), FreeBSD)
	CFLAGS		+= -D__FREEBSD__ -D__BSD__
	CFLAGS_DEBUG += -D__FREEBSD__ -D__BSD__
	PREFIX		= bin
	LIBDIR		= lib
	ETC_DIR		= /etc
	MANDIR		= share/man/man1
else ifeq ($(PLATFORM), OpenBSD)
	CFLAGS		+= -D__OPENBSD__ -D__BSD__
	CFLAGS_DEBUG += -D__OPENBSD__ -D__BSD__
	PREFIX		= bin
	LIBDIR		= lib
	ETC_DIR		= /etc
	MANDIR		= share/man/man1
else ifeq ($(PLATFORM), windows32)
	CC		= gcc
	PREFIX		= "C:\Program Files"
	LIBDIR		=
	MANDIR		=
endif

build: $(BIN_FILES) lib
	$(CC) $(CFLAGS) -o $(NAME) $(BIN_FILES) lib$(LIB_FILES:.c=.a)

lib: $(LIB_FILES)
	$(CC) $(CFLAGS) -fPIC -c -o $(LIB_FILES:.c=.o) $(LIB_FILES)
	$(AR) rcs lib$(LIB_FILES:.c=.a) $(LIB_FILES:.c=.o)
	$(CC) $(CFLAGS) -shared -o lib$(LIB_FILES:.c=.so) $(LIB_FILES:.c=.o)

debug:
	$(CC) $(CFLAGS_DEBUG) -o $(NAME) $(BIN_FILES)
	./$(NAME)

install: build
	mkdir -p $(DESTDIR)/$(PREFIX) $(DESTDIR)/$(LIBDIR)/uwufetch $(DESTDIR)/$(MANDIR) $(ETC_DIR)/uwufetch
	cp $(NAME) $(DESTDIR)/$(PREFIX)/$(NAME)
	cp -r res/* $(DESTDIR)/$(LIBDIR)/uwufetch
	cp default.config $(ETC_DIR)/uwufetch/config
	cp ./$(NAME).1.gz $(DESTDIR)/$(MANDIR)/

uninstall:
	rm -f $(DESTDIR)/$(PREFIX)/$(NAME)
	rm -rf $(DESTDIR)/$(LIBDIR)/uwufetch
	rm -rf $(ETC_DIR)/uwufetch
	rm -f $(DESTDIR)/$(MANDIR)/$(NAME).1.gz

clean:
	rm $(NAME) *.o *.so *.a

man:
	gzip --keep $(NAME).1

man_debug:
	@clear
	man -P cat ./uwufetch.1
