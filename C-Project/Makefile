CC = gcc
CFLAGS = -Wall -Wextra -O3 -g
LDFLAGS = -L/usr/lib -lssl -lcrypto -pthread -lm
INCLUDES = -I/usr/include
OBJ = main.o merge_sort.o
DEPS = main.h merge_sort.h
TARGET = main

# Default rule
all: $(TARGET)

# Compile the main binary
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile C files into object files
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Clean up compiled files
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
