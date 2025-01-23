CC = clang          # Compiler
CFLAGS = -g -Wall   # Flags for debugging and warnings
TARGET = clean-node-venv       # Output executable name

# Default target
all: $(TARGET)

# How to build the target
$(TARGET): clean.c
	$(CC) $(CFLAGS) clean.c -o $(TARGET)

# Clean up build artifacts
clean:
	rm -f $(TARGET)