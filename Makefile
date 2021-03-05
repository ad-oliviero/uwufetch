NAME = uwufetch
FILES = main.c
FLAGS = -O3 -Wall -Wextra
INSTALL_DIR = /usr/bin/
all: build install

build: main.c
	gcc $(FLAGS) -o $(NAME) $(FILES)

install:
	sudo cp $(NAME) $(INSTALL_DIR)$(NAME)

uninstall:
	sudo rm $(INSTALL_DIR)$(NAME)
