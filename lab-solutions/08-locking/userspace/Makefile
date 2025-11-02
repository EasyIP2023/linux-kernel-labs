CC ?= gcc
CFLAGS ?= -Wall -Wextra -g -O0 -static

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

BUILD_DIR = build

all: $(OBJECTS)

%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $(BUILD_DIR)/$(basename $(<))

clean:
	@rm -rf $(BUILD_DIR)

.PHONY: all clean

