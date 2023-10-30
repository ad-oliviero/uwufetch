NAME = uwutest
BIN_FILES = tests.c

ifndef UWUFETCH_VERSION
$(error This makefile should be called by the main makefile)
endif

build: $(BIN_FILES)
	$(CC) $(CFLAGS) -o $(NAME) $(BIN_FILES) $(LDFLAGS) $(SRC_DIR)/../../libfetch.a

clean:
	rm -f $(NAME)
