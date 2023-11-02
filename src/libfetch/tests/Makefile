TARGET = uwutest
SRCS = tests.c

override BUILD_DIR := $(PROJECT_ROOT)/$(BUILD_DIR)

ifndef UWUFETCH_VERSION
$(error This makefile must be called by the main makefile)
endif

$(TARGET):
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$(TARGET) $(SRCS) $(LDFLAGS) $(BUILD_DIR)/libfetch.a
