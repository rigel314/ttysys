# Commands
CC = gcc
LD = gcc

#Flags
options = -lncurses -lm
flags = -O2 --std=gnu11 -Wall

# Build directories
BUILD_DIR = build
TARGET_DIR = $(BUILD_DIR)/target
OBJECT_DIR = $(BUILD_DIR)/object

# Objects
objects = $(patsubst src/%.c,$(OBJECT_DIR)/%.o,$(wildcard src/*.c))
out = ttyload

# Make all of the build directories
$(shell mkdir -p $(TARGET_DIR) 2> /dev/null)
$(shell mkdir -p $(OBJECT_DIR) 2> /dev/null)

# Some targets don't create files
.PHONY : all clean

# Build all
all : $(TARGET_DIR)/ttyload

$(TARGET_DIR)/ttyload : $(objects)
	$(LD) $(options) $(objects) -o $@

# Compile source
$(OBJECT_DIR)/%.o : src/%.c
	$(CC) $(flags) -c $< -o $@

# Clean up the build directories
clean :
	-rm -rf $(TARGET_DIR) $(OBJECT_DIR) 2> /dev/null

# Install the executables
install : all

# Uninstall the executables
uninstall :
