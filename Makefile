NAME			= uwufetch
FILES			= uwufetch.c
CFLAGS			= 
CFLAGS_DEBUG	= -Wall -Wextra
PREFIX			= /usr/bin
CC				= cc
MAN_COMPILER	= pandoc

build: $(FILES)
	$(CC) $(CFLAGS) -o $(NAME) $(FILES)
	$(MAN_COMPILER) $(NAME)_man.md -st man -o $(NAME).1
	@#gzip $(NAME).1

debug:
	@clear
	$(CC) $(CFLAGS_DEBUG) -o $(NAME) $(FILES)
	./uwufetch

install:
	cp $(NAME) $(DESTDIR)$(PREFIX)/$(NAME)
	ls $(DESTDIR)/usr/lib/uwufetch/ 2> /dev/null || mkdir $(DESTDIR)/usr/lib/uwufetch/
	cp res/* $(DESTDIR)/usr/lib/uwufetch/
	cp ./$(NAME).1.gz $(DESTDIR)/usr/share/man/man1/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/$(NAME)
	rm -rf $(DESTDIR)/usr/lib/uwufetch/
	rm -rf $(DESTDIR)/usr/share/man/man1/$(NAME).1.gz

termux: build
	cp $(NAME) $(DESTDIR)/data/data/com.termux/files$(PREFIX)/$(NAME)
	ls $(DESTDIR)/data/data/com.termux/files/usr/lib/uwufetch/ > /dev/null || mkdir $(DESTDIR)/data/data/com.termux/files/usr/lib/uwufetch/
	cp res/* /data/data/com.termux/files/usr/lib/uwufetch/

termux_uninstall:
	rm -rf $(DESTDIR)/data/data/com.termux/files$(PREFIX)/$(NAME)
	rm -rf $(DESTDIR)/data/data/com.termux/files/usr/lib/uwufetch/

man_debug: build
	@clear
	@man -P cat ./uwufetch.1