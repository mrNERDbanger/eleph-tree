# Makefile for FUSE v3 compatibility layer
# This builds the FUSE v3 API compatibility library for macFUSE

CC = clang
CFLAGS = -Wall -Wextra -O2 -fPIC -D_FILE_OFFSET_BITS=64
INCLUDES = -I. -I/usr/local/include/osxfuse -I/opt/homebrew/include
LDFLAGS = -shared
LIBS = -L/usr/local/lib -losxfuse.2

# Output library
LIBNAME = libfuse3_compat.dylib
SONAME = libfuse3_compat.1.dylib

# Source files
SOURCES = fuse3_compat.c
OBJECTS = $(SOURCES:.c=.o)

# Headers to install
HEADERS = fuse3.h

# Installation paths
PREFIX ?= /usr/local
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include/fuse3

.PHONY: all clean install uninstall

all: $(LIBNAME)

$(LIBNAME): $(OBJECTS)
	$(CC) $(LDFLAGS) -install_name $(LIBDIR)/$(SONAME) -o $@ $^ $(LIBS)
	ln -sf $(LIBNAME) $(SONAME)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(LIBNAME) $(SONAME)

install: $(LIBNAME)
	install -d $(LIBDIR)
	install -d $(INCLUDEDIR)
	install -m 755 $(LIBNAME) $(LIBDIR)/
	ln -sf $(LIBNAME) $(LIBDIR)/$(SONAME)
	ln -sf $(SONAME) $(LIBDIR)/libfuse3.dylib
	install -m 644 $(HEADERS) $(INCLUDEDIR)/

uninstall:
	rm -f $(LIBDIR)/$(LIBNAME)
	rm -f $(LIBDIR)/$(SONAME)
	rm -f $(LIBDIR)/libfuse3.dylib
	rm -rf $(INCLUDEDIR)

# Example build
hello_fuse3: hello_fuse3.c $(LIBNAME)
	$(CC) $(CFLAGS) $(INCLUDES) -L. -lfuse3_compat hello_fuse3.c -o hello_fuse3

.SUFFIXES: .c .o