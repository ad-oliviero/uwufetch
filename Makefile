.PHONY: all debug clean
TARGET = uwufetch

# project directories and files
PROJECT_ROOT = $(shell pwd)
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = $(SRC_DIR)/libfetch/tests
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SRCS:.c=.o))
ASCII_EMBED_HEADER = $(SRC_DIR)/ascii_embed.h
HEADERS = $(wildcard $(SRC_DIR)/*.h) $(ASCII_EMBED_HEADER)

# installation directories
PREFIX_DIR =
USR_DIR = $(PREFIX_DIR)/usr
ETC_DIR = $(PREFIX_DIR)/etc
BIN_DIR = $(USR_DIR)/bin
LIB_DIR = $(USR_DIR)/lib
MAN_DIR = $(USR_DIR)/share/man/man1
INC_DIR = $(USR_DIR)/include

RELEASE_SCRIPTS = release_scripts/*.sh
RELEASE_NAME := $(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)

ifeq ($(OS), Windows_NT)
	PLATFORM = $(OS)
else
	PLATFORM = $(shell uname)
endif
PLATFORM_ABBR = $(PLATFORM)
include platform_fixes.mk

# compiler "settings" and compile-time info flags
CC ?= gcc
CSTD ?= gnu18
UWUFETCH_VERSION := -DUWUFETCH_VERSION=\""2.1"\"
UWUFETCH_COMPILER_VERSION := -DUWUFETCH_COMPILER_VERSION=\""$(shell $(CC) --version | head -n1)"\"
VERSION_FLAGS := $(UWUFETCH_VERSION) $(UWUFETCH_COMPILER_VERSION) $(UWUFETCH_GIT_COMMIT) $(UWUFETCH_GIT_BRANCH)
ifneq ($(wildcard .git),)
ifneq ($(shell command -v git 2> /dev/null),)
UWUFETCH_GIT_COMMIT := -DUWUFETCH_GIT_COMMIT=\""$(shell git describe --tags)"\"
UWUFETCH_GIT_BRANCH := -DUWUFETCH_GIT_BRANCH=\""$(shell git branch --show-current 2>/dev/null || git rev-parse --abbrev-ref HEAD)"\"
VERSION_FLAGS += $(UWUFETCH_GIT_COMMIT) $(UWUFETCH_GIT_COMMIT)
endif
endif
CFLAGS += -O3 -std=$(CSTD) $(VERSION_FLAGS)
CFLAGS_DEBUG += -Wall -Wextra -Wpedantic -Wunused-result -Wconversion -Wshadow -Wnull-dereference -Wcast-align -g -pthread $(VERSION_FLAGS) -D__DEBUG__
ifeq ($(shell $(CC) -v 2>&1 | grep clang >/dev/null; echo $$?), 0) # if the compiler is clang
	# macros give a lot of errors for ##__VA_ARGS__
	TEMP_CFLAG_FIXES = -Wno-c2x-extensions -Wno-unknown-pragmas -Wno-string-conversion -Wno-unused-function
	CFLAGS_DEBUG += -Wno-gnu-zero-variadic-macro-arguments $(TEMP_CFLAG_FIXES)
	CFLAGS += $(TEMP_CFLAG_FIXES)
endif


all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(TARGET): $(HEADERS) $(OBJS) $(BUILD_DIR)/libfetch.a | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(BUILD_DIR)/libfetch.a

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(ASCII_EMBED_HEADER): $(BUILD_DIR)/common.o $(BUILD_DIR)/actrie.o
	@$(MAKE) -C $(SRC_DIR)/ascii_preproc run

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

ascii_debug:
	ls res/ascii/$(LOGO).txt | entr -c $(MAKE) -C $(SRC_DIR)/ascii_preproc run ARGS="$(LOGO)"

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
	mkdir -p $(RELEASE_NAME)/libfetch
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
	@rm -fv $(ASCII_EMBED_HEADER)
	@rm -fv $(SRC)/ascii_preproc/ap

