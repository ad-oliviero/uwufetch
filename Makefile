NAME = uwufetch
FILES = uwufetch.c
CFLAGS = -O3
CFLAGS_DEBUG = -Wall -Wextra -g -pthread
CC = cc
DESTDIR = /usr
PLATFORM = $(shell uname)

ifeq ($(PLATFORM), Linux)
	PREFIX		= bin
	LIBDIR		= lib
	MANDIR		= share/man/man1
	ifeq ($(shell uname -o), Android)
		DESTDIR	= /data/data/com.termux/file
	endif
else ifeq ($(PLATFORM), Darwin)
	PREFIX		= local/bin
	LIBDIR		= local/lib
	MANDIR		= local/share/man/man1
else ifeq ($(PLATFORM), FreeBSD)
	CFLAGS		+= -D__FREEBSD__
	CFLAGS_DEBUG += -D__FREEBSD__
	PREFIX		= bin
	LIBDIR		= lib
	MANDIR		= share/man/man1
else ifeq ($(PLATFORM), windows32)
	CC		= gcc
	PREFIX		= "C:\Program Files"
	LIBDIR		=
	MANDIR		=
endif

build: $(FILES)
	$(CC) $(CFLAGS) -o $(NAME) $(FILES)

debug:
	$(CC) $(CFLAGS_DEBUG) -o $(NAME) $(FILES)
	./$(NAME)

install: build
	mkdir -p $(DESTDIR)/$(PREFIX) $(DESTDIR)/$(LIBDIR)/uwufetch $(DESTDIR)/$(MANDIR)
	cp $(NAME) $(DESTDIR)/$(PREFIX)/$(NAME)
	cp -r res/* $(DESTDIR)/$(LIBDIR)/uwufetch
	cp ./$(NAME).1.gz $(DESTDIR)/$(MANDIR)/

uninstall:
	rm -f $(DESTDIR)/$(PREFIX)/$(NAME)
	rm -rf $(DESTDIR)/$(LIBDIR)/uwufetch
	rm -f $(DESTDIR)/$(MANDIR)/$(NAME).1.gz

clean:
	rm $(NAME)

man:
	gzip --keep $(NAME).1

man_debug:
	@clear
	man -P cat ./uwufetch.1
