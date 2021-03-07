NAME = uwufetch
FILES = uwufetch.c
FLAGS = -O3 -Wall -Wextra
INSTALL_DIR = /usr/bin/
all: build install

build: uwufetch.c
	gcc $(FLAGS) -o $(NAME) $(FILES)

install:
	sudo cp $(NAME) $(INSTALL_DIR)$(NAME)
	ls ~/.config/uwufetch 1> /dev/null || mkdir ~/.config/uwufetch/
	cp res/* ~/.config/uwufetch/

uninstall:
	sudo rm $(INSTALL_DIR)$(NAME)
	rm -rf ~/.config/uwufetch

debug: build install
	clear
	./uwufetch