NAME = uwufetch
FILES = uwufetch.c
FLAGS = -O3
FLAGS_DEBUG = -Wall -Wextra
INSTALL_DIR = /usr/bin/
all: build install

build: uwufetch.c
	gcc $(FLAGS) -o $(NAME) $(FILES)

debug:
	clear
	gcc $(FLAGS_DEBUG) -o $(NAME) $(FILES)
	./uwufetch -id asd

install:
	sudo cp $(NAME) $(INSTALL_DIR)$(NAME)
	ls /usr/lib/uwufetch/ > /dev/null || sudo mkdir /usr/lib/uwufetch/
	sudo cp res/* /usr/lib/uwufetch/

uninstall:
	sudo rm $(INSTALL_DIR)$(NAME)
	sudo rm -rf /usr/lib/uwufetch/

termux: build
	cp $(NAME) /data/data/com.termux/files$(INSTALL_DIR)$(NAME)
	ls /data/data/com.termux/files/usr/lib/uwufetch/ > /dev/null || mkdir /data/data/com.termux/files/usr/lib/uwufetch/
	cp res/* /data/data/com.termux/files/usr/lib/uwufetch/
	
termux_uninstall:
	rm -rf /data/data/com.termux/files$(INSTALL_DIR)$(NAME)
	rm -rf /data/data/com.termux/files/usr/lib/uwufetch/