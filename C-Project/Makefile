CC        = gcc
CFLAGS    = -Wall -Wextra -O3 -g
INCLUDES  = -I/usr/include       # add -I/path/to/libuv/includes if needed
LDFLAGS   = -L/usr/lib -lssl -lcrypto -pthread -lm -luv

OBJ       = main.o merge_sort.o merge_sort_uv.o
DEPS      = merge_sort.h merge_sort_uv.h
TARGET    = main

# Default rule
all: $(TARGET)

# Link the final binary
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile .c → .o, tracking headers
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Remove build artifacts
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
