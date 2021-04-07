NAME			= uwufetch
FILES			= uwufetch.c
CFLAGS			= -O3
CFLAGS_DEBUG	= -Wall -Wextra
PREFIX			= /usr/bin
CC				= cc

build: $(FILES)
	$(CC) $(CFLAGS) -o $(NAME) $(FILES)

debug:
	@clear
	$(CC) $(CFLAGS_DEBUG) -o $(NAME) $(FILES)
	./uwufetch

install:
	cp $(NAME) $(DESTDIR)$(INSTALL_DIR)/$(NAME)
	ls $(DESTDIR)/usr/lib/uwufetch/ 2> /dev/null || mkdir $(DESTDIR)/usr/lib/uwufetch/
	cp res/* $(DESTDIR)/usr/lib/uwufetch/

uninstall:
	rm -f $(DESTDIR)$(INSTALL_DIR)/$(NAME)
	rm -rf $(DESTDIR)/usr/lib/uwufetch/

termux: build
	cp $(NAME) $(DESTDIR)/data/data/com.termux/files$(INSTALL_DIR)/$(NAME)
	ls $(DESTDIR)/data/data/com.termux/files/usr/lib/uwufetch/ > /dev/null || mkdir $(DESTDIR)/data/data/com.termux/files/usr/lib/uwufetch/
	cp res/* /data/data/com.termux/files/usr/lib/uwufetch/

termux_uninstall:
	rm -rf $(DESTDIR)/data/data/com.termux/files$(INSTALL_DIR)/$(NAME)
	rm -rf $(DESTDIR)/data/data/com.termux/files/usr/lib/uwufetch/
