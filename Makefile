TARGET = uwufetch
UWUFETCH_VERSION = $(shell git describe --tags)

# project directories and files
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = $(SRC_DIR)/tests
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SRCS:.c=.o))
PROJECT_ROOT = $(PWD)

# installation directories
PREFIX_DIR =
USR_DIR = $(PREFIX_DIR)/usr
ETC_DIR = $(PREFIX_DIR)/etc
BIN_DIR = $(USR_DIR)/bin
LIB_DIR = $(USR_DIR)/lib
MAN_DIR = $(USR_DIR)/share/man/man1
INC_DIR = $(USR_DIR)/include

# compiler "settings"
CC = gcc
CSTD = gnu18
CFLAGS = -O3 -DUWUFETCH_VERSION=\"$(UWUFETCH_VERSION)\" -std=$(CSTD)
CFLAGS_DEBUG = -Wall -Wextra -Wpedantic -Wunused-result -Wconversion -Warith-conversion -Wshadow -Warray-bounds=2 -ftree-vrp -Wnull-dereference -Wcast-align=strict -g -pthread -DUWUFETCH_VERSION=\"$(UWUFETCH_VERSION)\" -D__DEBUG__
LDFLAGS =
ifeq ($(shell $(CC) -v 2>&1 | grep clang >/dev/null; echo $$?), 0) # if the compiler is clang
	# macros give a lot of errors for ##__VA_ARGS__
	CFLAGS_DEBUG += -Wno-gnu-zero-variadic-macro-arguments
endif

RELEASE_SCRIPTS = release_scripts/*.sh

ifeq ($(OS), Windows_NT)
	PLATFORM = $(OS)
else
	PLATFORM = $(shell uname)
endif
PLATFORM_ABBR = $(PLATFORM)

include platform_fixes.mk
RELEASE_NAME := $(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET)

debug: CFLAGS=$(CFLAGS_DEBUG)
debug: all

valgrind: debug # checks memory leak
	valgrind --leak-check=full --show-leak-kinds=all $(BUILD_DIR)/$(TARGET)

gdb: debug
	gdb $(BUILD_DIR)/$(TARGET) -ex="set confirm off"

tests: CFLAGS=$(CFLAGS_DEBUG)
tests: libfetch
export
tests:
	@$(MAKE) -C $(TEST_DIR)

run:
	$(BUILD_DIR)/$(TARGET) $(ARGS)

ascii_debug: debug
	ls res/ascii/$(ASCII).txt | entr -c $(BUILD_DIR)/$(TARGET) -d $(ASCII)

man:
	sed "s/{DATE}/$(shell date '+%d %B %Y')/g" $(TARGET).1 | sed "s/{UWUFETCH_VERSION}/$(UWUFETCH_VERSION)/g" | gzip > $(BUILD_DIR)/$(TARGET).1.gz

man_debug:
	@clear
	man -P cat ./$(TARGET).1

dirs:
	@mkdir -pv $(BUILD_DIR)

$(BUILD_DIR)/$(TARGET): dirs $(OBJS) libfetch
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(BUILD_DIR)/libfetch.a

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

libfetch: dirs $(BUILD_DIR)/libfetch.a $(BUILD_DIR)/libfetch.so

$(BUILD_DIR)/libfetch.a:
export
$(BUILD_DIR)/libfetch.a:
	@$(MAKE) -C $(SRC_DIR)/libfetch $(PROJECT_ROOT)/$(BUILD_DIR)/libfetch.a

$(BUILD_DIR)/libfetch.so:
export
$(BUILD_DIR)/libfetch.so:
	@$(MAKE) -C $(SRC_DIR)/libfetch $(PROJECT_ROOT)/$(BUILD_DIR)/libfetch.so

export
install: clean $(BUILD_DIR)/$(TARGET) man
	@install -CDvm 755 $(BUILD_DIR)/$(TARGET) -t $(BIN_DIR)
	@install -CDvm 644 $(shell find res/ -type f -print) -t $(LIB_DIR)/$(TARGET)
	@install -CDvm 644 $(shell find res/ascii -type f -print) -t $(LIB_DIR)/$(TARGET)/ascii
	@install -CDvm 644 default.config -t $(ETC_DIR)/$(TARGET)/config
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

