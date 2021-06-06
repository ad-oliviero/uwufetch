NAME			= uwufetch
FILES			= uwufetch.c
CFLAGS			= -O3
CFLAGS_DEBUG	= -Wall -Wextra
ifeq ($(shell uname), Linux)
	PREFIX		= /usr/bin
	LIBDIR		= /usr/lib
else ifeq ($(shell uname), Darwin)
	PREFIX		= /usr/local/bin
	LIBDIR		= /usr/local/lib
endif
CC				= cc
MAN_COMPILER	= pandoc

build: $(FILES)
	$(CC) $(CFLAGS) -o $(NAME) $(FILES)

debug:
	@clear
	$(CC) $(CFLAGS_DEBUG) -o $(NAME) $(FILES)
	./uwufetch

install: build man
	cp $(NAME) $(DESTDIR)$(PREFIX)/$(NAME)
	ls $(DESTDIR)/$(LIBDIR)/ 2> /dev/null || mkdir $(DESTDIR)/$(LIBDIR)/
	cp res/* $(DESTDIR)/$(LIBDIR)/
	cp ./$(NAME).1.gz $(DESTDIR)/usr/share/man/man1/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/$(NAME)
	rm -rf $(DESTDIR)/$(LIBDIR)/
	rm -rf $(DESTDIR)/usr/share/man/man1/$(NAME).1.gz

termux: build
	cp $(NAME) $(DESTDIR)/data/data/com.termux/files$(PREFIX)/$(NAME)
	ls $(DESTDIR)/data/data/com.termux/files/usr/lib/uwufetch/ > /dev/null || mkdir $(DESTDIR)/data/data/com.termux/files/usr/lib/uwufetch/
	cp res/* /data/data/com.termux/files/usr/lib/uwufetch/

termux_uninstall:
	rm -rf $(DESTDIR)/data/data/com.termux/files$(PREFIX)/$(NAME)
	rm -rf $(DESTDIR)/data/data/com.termux/files/usr/lib/uwufetch/

man:
	$(MAN_COMPILER) $(NAME)_man.md -st man -o $(NAME).1
	@gzip $(NAME).1

man_debug:
	$(MAN_COMPILER) $(NAME)_man.md -st man -o $(NAME).1
	@clear
	@man -P cat ./uwufetch.1
