CC        = gcc
CFLAGS    = -Wall -Wextra -O3 -g -fopenmp
LDFLAGS   = -fopenmp

OBJ       = main.o merge_sort.o
DEPS      = merge_sort.h
TARGET    = main

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
