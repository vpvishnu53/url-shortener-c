# Makefile for url-shortener (multi-file build)

CC      = cc
CFLAGS  = -Wall -Wextra -Iinclude
TARGET  = shortener
SRCDIR  = src
SRCS    = $(SRCDIR)/main.c \
          $(SRCDIR)/hash.c \
          $(SRCDIR)/validate.c \
          $(SRCDIR)/storage.c \
          $(SRCDIR)/users.c \
          $(SRCDIR)/urls.c \
          $(SRCDIR)/http.c

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET) store.txt users.txt

.PHONY: clean
