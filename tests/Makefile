# builds and runs the uwufetch tests. It is meant to be called by the main
# makefile, which builds the objects in ../build (and passes CFLAGS_DEBUG)
# first. The tests are built in debug mode, so actrie's invariant sweep
# (actrie_t_check_computed_links) runs on every trie.

PROJECT_ROOT ?= $(abspath ..)
BUILD_DIR := $(PROJECT_ROOT)/build
# the objects of src/*.c (uwufetch.o excluded: it holds the real main)
TEST_OBJS = $(filter-out $(BUILD_DIR)/uwufetch.o,$(patsubst $(PROJECT_ROOT)/src/%.c,$(BUILD_DIR)/%.o,$(wildcard $(PROJECT_ROOT)/src/*.c)))
FETCH_OBJ = $(BUILD_DIR)/fetch.o
TEST_BINS = $(patsubst %.c,$(BUILD_DIR)/%,$(wildcard test_*.c))

$(BUILD_DIR)/test_%: test_%.c $(TEST_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $< $(TEST_OBJS)

$(BUILD_DIR)/test_fetch: test_fetch.c $(TEST_OBJS) $(FETCH_OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $< $(TEST_OBJS) $(FETCH_OBJ) $(LDFLAGS)

$(BUILD_DIR)/test_fetch_parse: test_fetch_parse.c $(TEST_OBJS) $(FETCH_OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $< $(TEST_OBJS) $(FETCH_OBJ) $(LDFLAGS)

# the fixture test includes fetch.c with the paths overridden to read
# tests/fixtures/proc (see the test file), so it needs no fetch.o and no -D
# paths here, only the meminfo variant selection
$(BUILD_DIR)/test_fetch_fixtures: test_fetch_fixtures.c $(TEST_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $< $(TEST_OBJS) $(LDFLAGS)

$(BUILD_DIR)/test_fetch_fixtures_empty: test_fetch_fixtures.c $(TEST_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -DFETCH_FIXTURE_EMPTY -o $@ $< $(TEST_OBJS) $(LDFLAGS)

$(BUILD_DIR)/test_fetch_fixtures_no_total: test_fetch_fixtures.c $(TEST_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -DFETCH_FIXTURE_NO_TOTAL -o $@ $< $(TEST_OBJS) $(LDFLAGS)

FIXTURE_VARIANT_BINS = $(BUILD_DIR)/test_fetch_fixtures_empty $(BUILD_DIR)/test_fetch_fixtures_no_total

# the fuzzer exercises the libfetch parse helpers too, so it links fetch.o
$(BUILD_DIR)/fuzz: fuzz.c $(TEST_OBJS) $(FETCH_OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $< $(TEST_OBJS) $(FETCH_OBJ) $(LDFLAGS)

$(BUILD_DIR)/run: run.c | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $<

FUZZ_SECONDS ?= 5

.PHONY: test

test: $(TEST_BINS) $(FIXTURE_VARIANT_BINS) $(BUILD_DIR)/fuzz $(BUILD_DIR)/run
	@for test in $(TEST_BINS) $(FIXTURE_VARIANT_BINS); do \
		echo "  TEST $$test"; \
		$$test || exit 1; \
	done
	$(BUILD_DIR)/fuzz $(FUZZ_SECONDS)
	cd $(PROJECT_ROOT) && $(BUILD_DIR)/run $(BUILD_DIR)/uwufetch
