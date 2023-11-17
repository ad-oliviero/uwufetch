.PHONY: all debug clean
TARGET = uwufetch
UWUFETCH_VERSION = $(shell git describe --tags)

# project directories and files
PROJECT_ROOT = $(shell pwd)
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = $(SRC_DIR)/libfetch/tests
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SRCS:.c=.o))
HEADERS = $(wildcard $(SRC_DIR)/*.h) + $(SRC_DIR)/ascii_embed.h

# installation directories
PREFIX_DIR =
USR_DIR = $(PREFIX_DIR)/usr
ETC_DIR = $(PREFIX_DIR)/etc
BIN_DIR = $(USR_DIR)/bin
LIB_DIR = $(USR_DIR)/lib
MAN_DIR = $(USR_DIR)/share/man/man1
INC_DIR = $(USR_DIR)/include

RELEASE_SCRIPTS = release_scripts/*.sh

# compiler "settings"
CC = gcc
CSTD = gnu18
CFLAGS = -O3 -DUWUFETCH_VERSION=\"$(UWUFETCH_VERSION)\" -std=$(CSTD)
CFLAGS_DEBUG = -Wall -Wextra -Wpedantic -Wunused-result -Wconversion -Wshadow -Wnull-dereference -Wcast-align -g -pthread -DUWUFETCH_VERSION=\"$(UWUFETCH_VERSION)\" -D__DEBUG__
ifeq ($(shell $(CC) -v 2>&1 | grep clang >/dev/null; echo $$?), 0) # if the compiler is clang
	# macros give a lot of errors for ##__VA_ARGS__
	TEMP_CFLAG_FIXES = -Wno-c2x-extensions -Wno-unknown-pragmas -Wno-string-conversion -Wno-unused-function
	CFLAGS_DEBUG += -Wno-gnu-zero-variadic-macro-arguments $(TEMP_CFLAG_FIXES)
	CFLAGS += $(TEMP_CFLAG_FIXES)
endif

ifeq ($(OS), Windows_NT)
	PLATFORM = $(OS)
else
	PLATFORM = $(shell uname)
endif
PLATFORM_ABBR = $(PLATFORM)

include platform_fixes.mk
RELEASE_NAME := $(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR):
	@mkdir -pv $(BUILD_DIR)

$(BUILD_DIR)/$(TARGET): $(OBJS) $(HEADERS) $(BUILD_DIR)/libfetch.a | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(BUILD_DIR)/libfetch.a

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(SRC_DIR)/ascii_embed.h:
	@printf "#ifndef _ASCII_EMBED_H_\n#define _ASCII_EMBED_H_\n\n" >> $@
	@printf "struct logo_embed {unsigned char* content; unsigned int length; unsigned int id;};\n\n" >> $@
	@$(foreach f,$(wildcard res/ascii/*.txt),xxd -n "$(basename $f)" -i "$f" >> $@;)
	uchr=($$(grep '^unsigned char' $@ | sed 's/unsigned char //g;s/\[\] = {//g')); \
	uint=($$(grep '^unsigned int' $@ | awk '{ print $$5 }' | sed 's/;//g')); \
	printf "\nstruct logo_embed* logos = {" >> $@; \
	i=0;while [ $$i -lt $${#uchr} ]; do printf "{$${uchr[i]}, $${uint[i]}, $$i}," >> src/ascii_embed.h; i=$$((i+1));done
	sed -i 's/^unsigned/static unsigned/g' $@
	printf "};\n#endif\n" >> $@


libfetch: $(BUILD_DIR)/libfetch.a $(BUILD_DIR)/libfetch.so

export
$(BUILD_DIR)/libfetch.a: $(wildcard $(SRC_DIR)/libfetch/*.c)
	@$(MAKE) -C $(SRC_DIR)/libfetch $(PROJECT_ROOT)/$(BUILD_DIR)/libfetch.a

export
$(BUILD_DIR)/libfetch.so: $(wildcard $(SRC_DIR)/libfetch/*.c)
	@$(MAKE) -C $(SRC_DIR)/libfetch $(PROJECT_ROOT)/$(BUILD_DIR)/libfetch.so

debug: CFLAGS=$(CFLAGS_DEBUG)
debug: all

valgrind: debug # checks memory leak
	valgrind --leak-check=full --show-leak-kinds=all $(BUILD_DIR)/$(TARGET) $(ARGS)

gdb: debug
	gdb $(BUILD_DIR)/$(TARGET) -ex="set confirm off"

test: $(BUILD_DIR)/uwutest
	$(BUILD_DIR)/uwutest $(ARGS)
$(BUILD_DIR)/uwutest: CFLAGS=$(CFLAGS_DEBUG)
export
$(BUILD_DIR)/uwutest: $(TEST_DIR)/tests.c $(BUILD_DIR)/libfetch.a
	@$(MAKE) -C $(TEST_DIR)

run: $(BUILD_DIR)/$(TARGET)
	$(BUILD_DIR)/$(TARGET) $(ARGS)

ascii_debug: debug
	ls res/ascii/$(ASCII).txt | entr -c $(BUILD_DIR)/$(TARGET) -d $(ASCII)

man: $(BUILD_DIR)/$(TARGET).1.gz
$(BUILD_DIR)/$(TARGET).1.gz: $(BUILD_DIR) $(TARGET).1
	sed "s/{DATE}/$(shell date '+%d %B %Y')/g" $(TARGET).1 | sed "s/{UWUFETCH_VERSION}/$(UWUFETCH_VERSION)/g" | gzip > $(BUILD_DIR)/$(TARGET).1.gz

man_debug:
	@clear
	man -P cat ./$(TARGET).1

export
install: clean $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/libfetch.so man
	@install -CDvm 755 $(BUILD_DIR)/$(TARGET) -t $(BIN_DIR)
	@install -CDvm 644 $(shell find res/ -type f -print) -t $(LIB_DIR)/$(TARGET)
	@install -CDvm 644 $(shell find res/ascii -type f -print) -t $(LIB_DIR)/$(TARGET)/ascii
	@install -CDvm 644 default.config -t $(ETC_DIR)/$(TARGET)
	@install -CDvm 644 $(BUILD_DIR)/$(TARGET).1.gz -t $(MAN_DIR)
	@$(MAKE) -C $(SRC_DIR)/libfetch install

export
uninstall:
	@rm -rv $(ETC_DIR)/$(TARGET)
	@rm -v $(BIN_DIR)/$(TARGET)
	@rm -rv $(LIB_DIR)/$(TARGET)
	@rm -v $(MAN_DIR)/$(TARGET).1.gz
	@$(MAKE) -C $(SRC_DIR)/libfetch uninstall


release: clean $(BUILD_DIR)/$(TARGET) man
	mkdir -pv $(RELEASE_NAME)/libfetch
	cp $(RELEASE_SCRIPTS) $(RELEASE_NAME)
	cp -r res $(RELEASE_NAME)
	cp $(BUILD_DIR)/$(TARGET)$(EXT) $(RELEASE_NAME)
	cp $(BUILD_DIR)/$(TARGET).1.gz $(RELEASE_NAME)
	cp default.config $(RELEASE_NAME)
	@$(MAKE) -C $(SRC_DIR)/libfetch release
ifeq ($(PLATFORM), linux4win)
	zip -9r $(RELEASE_NAME).zip $(RELEASE_NAME)
else
	tar -czf $(RELEASE_NAME).tar.gz $(RELEASE_NAME)
endif

clean:
	@rm -rvf $(BUILD_DIR)
	@rm -rvf $(RELEASE_NAME)*
	@rm -fv $(SRC_DIR)/ascii_embed.h

