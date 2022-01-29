NAME = uwufetch
FILES = uwufetch.c
CFLAGS = -O3
CFLAGS_DEBUG = -Wall -Wextra -g -pthread
CC = cc
DESTDIR = /usr

ifeq ($(shell uname), Linux)
	PREFIX		= local/bin
	LIBDIR		= local/lib
	MANDIR		= share/man/man1
else ifeq ($(shell uname), Darwin)
	PREFIX		= local/bin
	LIBDIR		= local/lib
	MANDIR		= local/share/man/man1
else ifeq ($(shell uname), FreeBSD)
	CFLAGS += -D__FREEBSD__
	CFLAGS_DEBUG += -D__FREEBSD__
	PREFIX		= local/bin
	LIBDIR		= local/lib
	MANDIR		= share/man/man1
else ifeq ($(shell uname), windows32)
	CC 			= gcc
	PREFIX		= "C:\Program Files"
	LIBDIR		=
	MANDIR		=
endif

build: $(FILES)
	$(CC) $(CFLAGS) -o $(NAME) $(FILES)

debug:
	$(CC) $(CFLAGS_DEBUG) -o $(NAME) $(FILES)
	./$(NAME)

install:
	mkdir -p $(DESTDIR)/$(PREFIX) $(DESTDIR)/$(LIBDIR)/uwufetch $(DESTDIR)/$(MANDIR)
	cp $(NAME) $(DESTDIR)/$(PREFIX)/$(NAME)
	cp -r res/* $(DESTDIR)/$(LIBDIR)/uwufetch
	cp ./$(NAME).1.gz $(DESTDIR)/$(MANDIR)/

uninstall:
	rm -f $(DESTDIR)/$(PREFIX)/$(NAME)
	rm -rf $(DESTDIR)/$(LIBDIR)/uwufetch
	rm -f $(DESTDIR)/$(MANDIR)/$(NAME).1.gz

termux: build
	cp $(NAME) $(DESTDIR)/data/data/com.termux/files$(PREFIX)/$(NAME)
	ls $(DESTDIR)/data/data/com.termux/files/usr/lib/uwufetch/ > /dev/null || mkdir $(DESTDIR)/data/data/com.termux/files/usr/lib/uwufetch/
	cp -r res/* /data/data/com.termux/files/usr/lib/uwufetch/

termux_uninstall:
	rm -rf $(DESTDIR)/data/data/com.termux/files$(PREFIX)/$(NAME)
	rm -rf $(DESTDIR)/data/data/com.termux/files/usr/lib/uwufetch/

clean:
	rm $(NAME)

man:
	gzip --keep $(NAME).1

man_debug:
	@clear
	man -P cat ./uwufetch.1
