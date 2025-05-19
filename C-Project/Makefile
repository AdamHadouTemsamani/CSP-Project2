CC        = /usr/local/opt/gcc/bin/gcc-14
CFLAGS    = -Wall -Wextra -O3 -g -fopenmp
INCLUDES  = 
LDFLAGS   = -fopenmp

OBJ       = main.o merge_sort.o 
DEPS      = merge_sort.h
TARGET    = main

# Default rule
all: $(TARGET)

# Link the final binary
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile .c → .o
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Clean build artifacts
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
