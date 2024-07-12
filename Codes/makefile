# Makefile for compiling the mySnakeGame

# Compiler
CC = gcc

# Compiler flags
CFLAGS = `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`

# Target executable
TARGET = mysnake

# Source files
SRCS = mysnake.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Rule for building the target executable
$(TARGET): $(OBJS)
    $(CC) -o $@ $(OBJS) $(LDFLAGS)

# Rule for compiling C source files to object files
%.o: %.c
    $(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
    rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean