NAME = uwutest
SRC_DIR ?= .
BIN_FILES = $(SRC_DIR)/tests.c
CFLAGS ?= -Wall -Wextra -Wpedantic -g -pthread -DUWUFETCH_VERSION=\"$(UWUFETCH_VERSION)\" -D__DEBUG__
CC ?= cc

build: $(BIN_FILES)
	$(CC) $(CFLAGS) -o $(NAME) $(BIN_FILES) $(LDFLAGS) $(SRC_DIR)/../../libfetch.a

clean:
	rm -f $(NAME)
