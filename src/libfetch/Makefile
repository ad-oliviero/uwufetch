TARGET = libfetch
TARGET_STATIC = $(TARGET).a
TARGET_SHARED = $(TARGET).so
SOURCES := $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
AR = ar
.PHONY: all clean

ifndef UWUFETCH_VERSION
$(error This makefile should be called by the main makefile)
endif

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) -fPIC -c -o $(BUILD_DIR)/$(OBJECTS) $(SOURCES)
	$(AR) rcs $(BUILD_DIR)/$(TARGET_STATIC) $(BUILD_DIR)/$(OBJECTS)
	$(CC) $(CFLAGS) -shared -o $(BUILD_DIR)/$(TARGET_SHARED) $(BUILD_DIR)/$(OBJECTS)

install: $(TARGET)
	mkdir -pv $(DESTDIR)/$(PREFIX) $(DESTDIR)/$(LIBDIR)/$(TARGET) $(DESTDIR)/$(MANDIR) $(ETC_DIR)/$(TARGET) $(DESTDIR)/$(INCDIR)
	cp $(TARGET_SHARED) $(DESTDIR)/$(LIBDIR)
	cp $(TARGET_STATIC) $(DESTDIR)/$(LIBDIR)
	cp $(LIB_FILES:.c=.h) $(DESTDIR)/$(INCDIR)

uninstall:
	rm -f $(DESTDIR)/$(LIBDIR)/$(TARGET_SHARED)
	rm -f $(DESTDIR)/$(LIBDIR)/$(TARGET_STATIC)
	rm -f $(DESTDIR)/include/$(LIB_FILES:.c=.h)

clean:
	rm -rf $(TARGET) $(OBJECTS) $(TARGET_STATIC) $(TARGET_SHARED)

