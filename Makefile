# ==============================================================
# Structure:
#   src/      -> source files (.c)
#   include/  -> header files (.h)
#   build/    -> object files (.o) [auto-generated]
#   bin/      -> final executables [auto-generated]
# ==============================================================

# Compiler and flags
CC      := gcc
CFLAGS  := -Wall -Wextra -Werror -std=c11 -Iinclude -g
LDFLAGS := -pthread   # add libraries here if needed (e.g., -lm)

# Directories
SRC_DIR := src
INC_DIR := include
OBJ_DIR := build
BIN_DIR := bin

# Target executable name
TARGET  := broker

# Collect all .c files under src/
SRC     := $(wildcard $(SRC_DIR)/*.c)

# Generate corresponding .o files in build/
OBJ     := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

# Final executable path
EXEC    := $(BIN_DIR)/$(TARGET)

# ==============================================================
# Rules
# ==============================================================

# Default rule: build the executable
all: $(EXEC)

# Link objects into final executable
$(EXEC): $(OBJ) | $(BIN_DIR)
	@echo "🔗 Linking $@"
	$(CC) $(OBJ) -o $@ $(LDFLAGS) -luuid

# Compile .c into .o (with dependency on headers)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "⚙️  Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure build/ and bin/ directories exist
$(OBJ_DIR) $(BIN_DIR):
	@mkdir -p $@

# Run the program with optional arguments
run: $(EXEC)
	@echo "🚀 Running $(EXEC) $(filter-out $@,$(MAKECMDGOALS))"
	./$(EXEC) $(filter-out $@,$(MAKECMDGOALS))

# Trick to avoid make treating args as targets
%:
	@:

# Remove object files only
clean:
	@echo "🧹 Cleaning object files"
	@rm -rf $(OBJ_DIR)/*.o

# Remove all build artifacts (objects + binary)
distclean: clean
	@echo "🗑️  Removing binary files"
	@rm -rf $(BIN_DIR)/*

# Print variables (for debugging Makefile)
print-%:
	@echo $* = $($*)

# Phony targets (not real files)
.PHONY: all run clean distclean
