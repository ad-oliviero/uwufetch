NAME = uwufetch
FILES = uwufetch.c
FLAGS = -O3 -Wall -Wextra
INSTALL_DIR = /usr/bin/
all: build install

build: uwufetch.c
	gcc $(FLAGS) -o $(NAME) $(FILES)

install:
	sudo cp $(NAME) $(INSTALL_DIR)$(NAME)
	ls /usr/lib/uwufetch/ > /dev/null || sudo mkdir /usr/lib/uwufetch/
	sudo cp res/* /usr/lib/uwufetch/

uninstall:
	sudo rm $(INSTALL_DIR)$(NAME)
	sudo rm -rf /usr/lib/uwufetch/

debug: build install
	clear
	./uwufetch