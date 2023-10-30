TARGET_STATIC = libfetch.a
TARGET_SHARED = libfetch.so
SOURCES = $(wildcard *.c)
OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SOURCES))
AR = ar

ifndef UWUFETCH_VERSION
$(error This makefile should be called by the main makefile)
endif

$(BUILD_DIR)/$(TARGET_STATIC): $(OBJECTS)
	$(AR) rcs $(BUILD_DIR)/$(TARGET_STATIC) $(OBJECTS)

$(BUILD_DIR)/$(TARGET_SHARED): $(OBJECTS)
	$(CC) $(CFLAGS) -shared -o $(BUILD_DIR)/$(TARGET_SHARED) $(OBJECTS)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

install: $(TARGET)
	mkdir -pv $(DESTDIR)/$(PREFIX) $(DESTDIR)/$(LIBDIR)/$(TARGET) $(DESTDIR)/$(MANDIR) $(ETC_DIR)/$(TARGET) $(DESTDIR)/$(INCDIR)
	cp $(TARGET_SHARED) $(DESTDIR)/$(LIBDIR)
	cp $(TARGET_STATIC) $(DESTDIR)/$(LIBDIR)
	cp $(LIB_FILES:.c=.h) $(DESTDIR)/$(INCDIR)

uninstall:
	rm -f $(DESTDIR)/$(LIBDIR)/$(TARGET_SHARED)
	rm -f $(DESTDIR)/$(LIBDIR)/$(TARGET_STATIC)
	rm -f $(DESTDIR)/include/$(LIB_FILES:.c=.h)

