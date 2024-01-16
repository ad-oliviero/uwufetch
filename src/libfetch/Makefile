TARGET_NAME = libfetch
TARGET_STATIC = $(TARGET_NAME).a
TARGET_SHARED = $(TARGET_NAME).so
CSTD ?= gnu18
CFLAGS ?= -O3 -std=$(CSTD)

PROJECT_ROOT ?= .
ifneq ($(PREFIX_DIR),) # if PREFIX_DIR is not system root
USR_DIR := $(PREFIX_DIR)/usr
ETC_DIR := $(PREFIX_DIR)/etc
BIN_DIR := $(USR_DIR)/bin
LIB_DIR := $(USR_DIR)/lib
MAN_DIR := $(USR_DIR)/share/man/man1
INC_DIR := $(USR_DIR)/include
endif
BUILD_DIR ?= build
override BUILD_DIR := $(PROJECT_ROOT)/$(BUILD_DIR)

SOURCES = $(wildcard *.c)
OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SOURCES))
HEADERS = $(wildcard *.h)
AR = ar
.PHONY: all clean

ifndef UWUFETCH_VERSION
$(warning This makefile should be called by the main makefile)
STANDALONE_COMPILATION = 1
endif

all: $(BUILD_DIR)/$(TARGET_STATIC) $(BUILD_DIR)/$(TARGET_SHARED)

$(BUILD_DIR)/$(TARGET_STATIC): $(BUILD_DIR) $(OBJECTS)
	$(AR) rcs $(BUILD_DIR)/$(TARGET_STATIC) $(OBJECTS)

$(BUILD_DIR)/$(TARGET_SHARED): $(BUILD_DIR) $(OBJECTS)
	$(CC) $(CFLAGS) -shared -o $(BUILD_DIR)/$(TARGET_SHARED) $(OBJECTS)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

$(BUILD_DIR):
	@mkdir -pv $(BUILD_DIR)

ifneq ($(STANDALONE_COMPILATION), 1)
release: clean $(BUILD_DIR)/$(TARGET_STATIC) $(BUILD_DIR)/$(TARGET_SHARED)
	cp $(BUILD_DIR)/$(TARGET_STATIC) ../../$(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp $(BUILD_DIR)/$(TARGET_SHARED) ../../$(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)
	cp $(HEADERS) ../../$(TARGET)_$(UWUFETCH_VERSION)-$(PLATFORM_ABBR)/$(TARGET_NAME)
endif

ifeq ($(STANDALONE_COMPILATION), 1)
install: clean $(BUILD_DIR)/$(TARGET_STATIC) $(BUILD_DIR)/$(TARGET_SHARED)
else
install:
endif
	@install -CDvm 755 $(BUILD_DIR)/$(TARGET_STATIC) -t $(LIB_DIR)
	@install -CDvm 755 $(BUILD_DIR)/$(TARGET_SHARED) -t $(LIB_DIR)
	@install -CDvm 644 $(HEADERS) -t $(INC_DIR)/$(TARGET_NAME)

uninstall:
	@rm -rv $(INC_DIR)/$(TARGET_NAME)
	@rm -v $(LIB_DIR)/$(TARGET_STATIC)
	@rm -v $(LIB_DIR)/$(TARGET_SHARED)

ifeq ($(STANDALONE_COMPILATION), 1)
clean:
	@rm -rvf $(BUILD_DIR)
endif
