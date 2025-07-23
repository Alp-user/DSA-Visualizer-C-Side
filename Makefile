# Compiler settings
CC = gcc
CXX = g++
CFLAGS = -g -Wall -Wextra -std=c99 -Iinclude
CXXFLAGS = -g -Wall -Wextra -std=c++17 -Iinclude
LDFLAGS = -lGL -lglfw

# Directories
SRCDIR = src
INCDIR = include
BUILDDIR = build

# Target executable name
TARGET = program

# Find all source files
CSRC = $(wildcard $(SRCDIR)/*.c)
CXXSRC = $(wildcard $(SRCDIR)/*.cpp)

# Generate object file names
COBJ = $(CSRC:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
CXXOBJ = $(CXXSRC:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
OBJ = $(COBJ) $(CXXOBJ)

# Default target
all: $(TARGET)

# Link the target executable
$(TARGET): $(OBJ) | $(BUILDDIR)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

# Compile C source files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ source files
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create build directory if it doesn't exist
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR) $(TARGET)

# Rebuild everything
rebuild: clean all 

# Show help
help:
	@echo "Available targets:"
	@echo "  all      - Build the project (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  rebuild  - Clean and build"
	@echo "  help     - Show this help message"

# Declare phony targets
.PHONY: all clean rebuild help
