# builds and runs the uwufetch tests. It is meant to be called by the main
# makefile, which builds the objects in ../build (and passes CFLAGS_DEBUG)
# first. The tests are built in debug mode, so actrie's invariant sweep
# (actrie_t_check_computed_links) runs on every trie.

PROJECT_ROOT ?= $(abspath ..)
BUILD_DIR := $(PROJECT_ROOT)/build
# the objects of src/*.c (uwufetch.o excluded: it holds the real main);
# the libfetch tests link fetch.o on top of it (the explicit rules below
# override the generic pattern rule)
TEST_OBJS = $(filter-out $(BUILD_DIR)/uwufetch.o,$(patsubst $(PROJECT_ROOT)/src/%.c,$(BUILD_DIR)/%.o,$(wildcard $(PROJECT_ROOT)/src/*.c)))
FETCH_OBJ = $(BUILD_DIR)/fetch.o
TEST_BINS = $(patsubst %.c,$(BUILD_DIR)/%,$(wildcard test_*.c))

$(BUILD_DIR)/test_%: test_%.c $(TEST_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $< $(TEST_OBJS)

$(BUILD_DIR)/test_fetch: test_fetch.c $(TEST_OBJS) $(FETCH_OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $< $(TEST_OBJS) $(FETCH_OBJ) $(LDFLAGS)

$(BUILD_DIR)/test_fetch_parse: test_fetch_parse.c $(TEST_OBJS) $(FETCH_OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $< $(TEST_OBJS) $(FETCH_OBJ) $(LDFLAGS)

# the fixture objects compile fetch.c with the file paths redirected into
# tests/fixtures/proc (the paths are #ifndef-guarded, so -D overrides them)
FIXTURE_DIR = $(PROJECT_ROOT)/tests/fixtures/proc
FIXTURE_PATH_FLAGS = -DPROC_CPUINFO_PATH='"$(FIXTURE_DIR)/cpuinfo"' \
                     -DFB0_VIRTUAL_SIZE_PATH='"$(FIXTURE_DIR)/fb0"' \
                     -DOS_RELEASE_PATH='"$(FIXTURE_DIR)/os_release"' \
                     -DHOSTNAME_PATH='"$(FIXTURE_DIR)/hostname"'
FETCH_FIXTURE_BINS = $(BUILD_DIR)/test_fetch_fixtures $(BUILD_DIR)/test_fetch_fixtures_empty $(BUILD_DIR)/test_fetch_fixtures_no_total

$(BUILD_DIR)/fetch_fixture_full.o: $(PROJECT_ROOT)/src/libfetch/fetch.c | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -DPROC_MEMINFO_PATH='"$(FIXTURE_DIR)/meminfo"' $(FIXTURE_PATH_FLAGS) -c -o $@ $<

$(BUILD_DIR)/fetch_fixture_empty_meminfo.o: $(PROJECT_ROOT)/src/libfetch/fetch.c | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -DPROC_MEMINFO_PATH='"$(FIXTURE_DIR)/meminfo_empty"' $(FIXTURE_PATH_FLAGS) -c -o $@ $<

$(BUILD_DIR)/fetch_fixture_no_meminfo_total.o: $(PROJECT_ROOT)/src/libfetch/fetch.c | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -DPROC_MEMINFO_PATH='"$(FIXTURE_DIR)/meminfo_no_total"' $(FIXTURE_PATH_FLAGS) -c -o $@ $<

$(BUILD_DIR)/test_fetch_fixtures: test_fetch_fixtures.c $(BUILD_DIR)/fetch_fixture_full.o | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $< $(BUILD_DIR)/fetch_fixture_full.o $(LDFLAGS)

$(BUILD_DIR)/test_fetch_fixtures_empty: test_fetch_fixtures.c $(BUILD_DIR)/fetch_fixture_empty_meminfo.o | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -DFETCH_EMPTY_MEMINFO -o $@ $< $(BUILD_DIR)/fetch_fixture_empty_meminfo.o $(LDFLAGS)

$(BUILD_DIR)/test_fetch_fixtures_no_total: test_fetch_fixtures.c $(BUILD_DIR)/fetch_fixture_no_meminfo_total.o | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -DFETCH_NO_TOTAL_MEMINFO -o $@ $< $(BUILD_DIR)/fetch_fixture_no_meminfo_total.o $(LDFLAGS)

# the fuzzer exercises the libfetch parse helpers too, so it links fetch.o
$(BUILD_DIR)/fuzz: fuzz.c $(TEST_OBJS) $(FETCH_OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $< $(TEST_OBJS) $(FETCH_OBJ) $(LDFLAGS)

$(BUILD_DIR)/run: run.c | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $<

FUZZ_SECONDS ?= 5

.PHONY: test

test: $(TEST_BINS) $(FETCH_FIXTURE_BINS) $(BUILD_DIR)/fuzz $(BUILD_DIR)/run
	@for test in $(TEST_BINS) $(FETCH_FIXTURE_BINS); do \
		echo "  TEST $$test"; \
		$$test || exit 1; \
	done
	$(BUILD_DIR)/fuzz $(FUZZ_SECONDS)
	cd $(PROJECT_ROOT) && $(BUILD_DIR)/run $(BUILD_DIR)/uwufetch
