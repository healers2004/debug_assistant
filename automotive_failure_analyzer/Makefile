CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
TARGET = auto_analyze
SRCDIR = src
INCDIR = include
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/signal_analyzer.c $(SRCDIR)/errno_mapper.c $(SRCDIR)/log_parser.c $(SRCDIR)/failure_rules.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

