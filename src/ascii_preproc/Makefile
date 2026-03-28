override BUILD_DIR := $(PROJECT_ROOT)/$(BUILD_DIR)

RES_ASCII_DIR_NAME = $(PROJECT_ROOT)/res/ascii

TARGET := ap

ifndef UWUFETCH_VERSION
$(warning This makefile should be called by the main makefile)
STANDALONE_COMPILATION = 1
endif

$(TARGET): $(TARGET).c
	$(CC) -DOUT_FILE_NAME=\""$(PROJECT_ROOT)/$(ASCII_EMBED_HEADER)"\" \
		-DRES_ASCII_DIR_NAME=\""$(RES_ASCII_DIR_NAME)/"\" \
		$(CFLAGS_DEBUG) \
		-o $@ $< $(BUILD_DIR)/common.o $(BUILD_DIR)/actrie.o

run: $(TARGET)
	./$(TARGET) $(ARGS)

clean:
	@rm -fv $(TARGET)
