TARGET = uwufetch
UWUFETCH_VERSION = $(shell git describe --tags)

SRC_DIR = src
BUILD_DIR = build
TEST_DIR = $(SRC_DIR)/tests
DEST_DIR = /usr

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SRCS:.c=.o))
LIB_FILES = AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

CC = gcc
CSTD = gnu18
CFLAGS = -O3 -DUWUFETCH_VERSION=\"$(UWUFETCH_VERSION)\" -std=$(CSTD)
CFLAGS_DEBUG = -Wall -Wextra -Wpedantic -Wunused-result -Wconversion -Warith-conversion -Wshadow -Warray-bounds=2 -ftree-vrp -Wnull-dereference -Wcast-align=strict -g -pthread -DUWUFETCH_VERSION=\"$(UWUFETCH_VERSION)\" -D__DEBUG__
LDFLAGS =

RELEASE_SCRIPTS = release_scripts/*.sh

ifeq ($(OS), Windows_NT)
	PLATFORM = $(OS)
else
	PLATFORM = $(shell uname)
endif
PLATFORM_ABBR = $(PLATFORM)

ifeq ($(shell $(CC) -v 2>&1 | grep clang >/dev/null; echo $$?), 0) # if the compiler is clang
	# macros give a lot of errors for ##__VA_ARGS__
	CFLAGS_DEBUG += -Wno-gnu-zero-variadic-macro-arguments
endif
include platform_fixes.mk
.PHONY: all clean tests libfetch

all: dirs $(TARGET)

dirs:
	@mkdir -pv $(BUILD_DIR)

$(TARGET): $(OBJS) libfetch
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$(TARGET) $(OBJS) $(LDFLAGS) $(BUILD_DIR)/libfetch.a

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

libfetch: BUILD_DIR := ../../$(BUILD_DIR)
export
libfetch:
	@$(MAKE) -C $(SRC_DIR)/libfetch

release: build man
	mkdir -pv $(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp $(RELEASE_SCRIPTS) $(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp -r res $(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp $(TARGET)$(EXT) $(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp $(TARGET).1.gz $(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp lib$(LIB_FILES:.c=.so) $(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp $(SRC_DIR)/$(LIB_FILES:.c=.h) $(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp default.config $(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
ifeq ($(PLATFORM), linux4win)
	zip -9r $(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR).zip $(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
else
	tar -czf $(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR).tar.gz $(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
endif

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: build

valgrind: # checks memory leak
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

gdb:
	gdb ./$(TARGET) -ex="set confirm off"

run:
	./$(TARGET) $(ARGS)

tests: debug
	@$(MAKE) -C $(TEST_DIR)

install: build man
	mkdir -pv $(DEST_DIR)/$(PREFIX) $(DEST_DIR)/$(LIBDIR)/$(TARGET) $(DEST_DIR)/$(MANDIR) $(ETC_DIR)/$(TARGET) $(DEST_DIR)/$(INCDIR)
	cp $(TARGET) $(DEST_DIR)/$(PREFIX)
	cp lib$(LIB_FILES:.c=.so) $(DEST_DIR)/$(LIBDIR)
	cp $(LIB_FILES:.c=.h) $(DEST_DIR)/$(INCDIR)
	cp -r res/* $(DEST_DIR)/$(LIBDIR)/$(TARGET)
	cp default.config $(ETC_DIR)/$(TARGET)/config
	cp ./$(TARGET).1.gz $(DEST_DIR)/$(MANDIR)

uninstall:
	rm -f $(DEST_DIR)/$(PREFIX)/$(TARGET)
	rm -rf $(DEST_DIR)/$(LIBDIR)/uwufetch
	rm -f $(DEST_DIR)/$(LIBDIR)/lib$(LIB_FILES:.c=.so)
	rm -f $(DEST_DIR)/include/$(LIB_FILES:.c=.h)
	rm -rf $(ETC_DIR)/uwufetch
	rm -f $(DEST_DIR)/$(MANDIR)/$(TARGET).1.gz

clean:
	$(MAKE) -f src/tests/tests.mk clean
	rm -rf $(TARGET) $(TARGET)_* *.o *.so *.a *.exe

ascii_debug: build
ascii_debug:
	ls res/ascii/$(ASCII).txt | entr -c ./$(TARGET) -d $(ASCII)

man:
	sed "s/{DATE}/$(shell date '+%d %B %Y')/g" $(TARGET).1 | sed "s/{UWUFETCH_VERSION}/$(UWUFETCH_VERSION)/g" | gzip > $(TARGET).1.gz

man_debug:
	@clear
	man -P cat ./$(TARGET).1
