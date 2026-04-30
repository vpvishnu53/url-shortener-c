# Makefile for url-shortener (Windows, MinGW/GCC)

CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude
LDFLAGS = -lws2_32
TARGET  = shortener.exe
SRCDIR  = src

SRCS = $(SRCDIR)/main.c     \
       $(SRCDIR)/hash.c     \
       $(SRCDIR)/validate.c \
       $(SRCDIR)/storage.c  \
       $(SRCDIR)/users.c    \
       $(SRCDIR)/urls.c     \
       $(SRCDIR)/http.c

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	del /Q $(TARGET) store.txt users.txt 2>nul || true

.PHONY: clean
