# builds and runs the uwufetch tests. It is meant to be called by the main
# makefile, which builds the objects in ../build (and passes CFLAGS_DEBUG)
# first. The tests are built in debug mode, so actrie's invariant sweep
# (actrie_t_check_computed_links) runs on every trie.

PROJECT_ROOT ?= $(abspath ..)
BUILD_DIR := $(PROJECT_ROOT)/build
# the objects of src/*.c (uwufetch.o excluded: it holds the real main, and
# the libfetch objects are not needed by the tests)
TEST_OBJS = $(filter-out $(BUILD_DIR)/uwufetch.o,$(patsubst $(PROJECT_ROOT)/src/%.c,$(BUILD_DIR)/%.o,$(wildcard $(PROJECT_ROOT)/src/*.c)))
TEST_BINS = $(patsubst %.c,$(BUILD_DIR)/%,$(wildcard test_*.c))

# test_fetch links fetch.o (get_gpu_list); libpci on linux, frameworks on macos
ifeq ($(shell uname),Darwin)
FETCH_TEST_LIBS = -framework CoreFoundation -framework IOKit -framework CoreGraphics
else
FETCH_TEST_LIBS = -lpci
endif

$(BUILD_DIR)/test_fetch: test_fetch.c $(TEST_OBJS) $(BUILD_DIR)/fetch.o | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $< $(TEST_OBJS) $(BUILD_DIR)/fetch.o $(FETCH_TEST_LIBS)

$(BUILD_DIR)/test_%: test_%.c $(TEST_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $< $(TEST_OBJS)

$(BUILD_DIR)/fuzz: fuzz.c $(TEST_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $< $(TEST_OBJS)

$(BUILD_DIR)/run: run.c | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $<

FUZZ_SECONDS ?= 5

.PHONY: test

test: $(TEST_BINS) $(BUILD_DIR)/fuzz $(BUILD_DIR)/run
	@for test in $(TEST_BINS); do \
		echo "  TEST $$test"; \
		$$test || exit 1; \
	done
	$(BUILD_DIR)/fuzz $(FUZZ_SECONDS)
	cd $(PROJECT_ROOT) && $(BUILD_DIR)/run $(BUILD_DIR)/uwufetch
