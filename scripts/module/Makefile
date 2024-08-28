CC = gcc
CFLAGS = -w -Wall -Wextra

SRCDIR = .
LIBDIR = hacl-20
SOURCES = $(wildcard $(SRCDIR)/*.c)
LIB_SOURCES = $(wildcard $(LIBDIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)
LIB_OBJECTS = $(LIB_SOURCES:.c=.o)
EXECUTABLE = ../tools/sign_mod

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) $(LIB_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIBDIR)/%.o: $(LIBDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(LIB_OBJECTS) $(EXECUTABLE)

.PHONY: all clean
