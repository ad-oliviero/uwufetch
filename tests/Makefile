# builds and runs the uwufetch tests. It is meant to be called by the main
# makefile, which builds the objects in ../build (and passes CFLAGS_DEBUG)
# first.

PROJECT_ROOT ?= ..
BUILD_DIR := $(PROJECT_ROOT)/build
# the objects of src/*.c (uwufetch.o excluded: it holds the real main, and
# the libfetch objects are not needed by the tests)
TEST_OBJS = $(filter-out $(BUILD_DIR)/uwufetch.o,$(patsubst $(PROJECT_ROOT)/src/%.c,$(BUILD_DIR)/%.o,$(wildcard $(PROJECT_ROOT)/src/*.c)))
TEST_BINS = $(patsubst %.c,$(BUILD_DIR)/%,$(wildcard test_*.c))

$(BUILD_DIR)/test_%: test_%.c $(TEST_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $< $(TEST_OBJS)

.PHONY: test

test: $(TEST_BINS)
	@for test in $(TEST_BINS); do \
		echo "  TEST $$test"; \
		$$test || exit 1; \
	done
