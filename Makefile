NAME			= uwufetch
FILES			= uwufetch.c
CFLAGS			= -O3
CFLAGS_DEBUG	= -Wall -Wextra -g -pthread
ifeq ($(shell uname), Linux)
	PREFIX		= /usr/bin
	LIBDIR		= /usr/lib
	MANDIR		= /usr/share/man/man1
else ifeq ($(shell uname), Darwin)
	PREFIX		= /usr/local/bin
	LIBDIR		= /usr/local/lib
	MANDIR		= /usr/local/share/man/man1
else ifeq ($(shell uname), FreeBSD)
	CFLAGS += -D__FREEBSD__
	CFLAGS_DEBUG += -D__FREEBSD__
	PREFIX		= /usr/bin
	LIBDIR		= /usr/lib
	MANDIR		= /usr/share/man/man1
endif

CC = cc


build: $(FILES)
	$(CC) $(CFLAGS) -o $(NAME) $(FILES)

debug:
	@clear
	$(CC) $(CFLAGS_DEBUG) -o $(NAME) $(FILES)
	UWUFETCH_CACHE_ENABLED=true ./uwufetch

install:
	mkdir -p $(DESTDIR)$(PREFIX) $(DESTDIR)$(LIBDIR)/uwufetch $(DESTDIR)$(MANDIR)
	cp $(NAME) $(DESTDIR)$(PREFIX)/$(NAME)
	cp -r res/* $(DESTDIR)$(LIBDIR)/uwufetch
	cp ./$(NAME).1.gz $(DESTDIR)$(MANDIR)/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/$(NAME)
	rm -rf $(DESTDIR)$(LIBDIR)/uwufetch
	rm -f $(DESTDIR)$(MANDIR)/$(NAME).1.gz

termux: build
	cp $(NAME) $(DESTDIR)/data/data/com.termux/files$(PREFIX)/$(NAME)
	ls $(DESTDIR)/data/data/com.termux/files/usr/lib/uwufetch/ > /dev/null || mkdir $(DESTDIR)/data/data/com.termux/files/usr/lib/uwufetch/
	cp -r res/* /data/data/com.termux/files/usr/lib/uwufetch/

termux_uninstall:
	rm -rf $(DESTDIR)/data/data/com.termux/files$(PREFIX)/$(NAME)
	rm -rf $(DESTDIR)/data/data/com.termux/files/usr/lib/uwufetch/

man:
	gzip --keep $(NAME).1

man_debug:
	@clear
	man -P cat ./uwufetch.1
