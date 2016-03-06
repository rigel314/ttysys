# Commands
CC = gcc
LD = gcc

#Flags
options = -lncurses -lm -ldl
flags = -O2 --std=gnu99 -Wall

# Build directories
BUILD_DIR = build
TARGET_DIR = $(BUILD_DIR)/target
OBJECT_DIR = $(BUILD_DIR)/object
LIB_DIR = $(OBJECT_DIR)/libs

# Objects
objects = $(patsubst src/%.c,$(OBJECT_DIR)/%.o,$(wildcard src/*.c))
default_libs = $(patsubst src/libs/%.c,$(LIB_DIR)/%.so,$(wildcard src/libs/*.c))
out = ttysys

# Make all of the build directories
$(shell mkdir -p $(TARGET_DIR) 2> /dev/null)
$(shell mkdir -p $(OBJECT_DIR) 2> /dev/null)
$(shell mkdir -p $(LIB_DIR) 2> /dev/null)

# Some targets don't create filesobjects
.PHONY : all run clean install uninstall

# Build all
all : $(TARGET_DIR)/ttysys

$(TARGET_DIR)/ttysys : $(objects) $(default_libs)
	$(LD) $(objects) -o $@  $(options)

# Compile source
$(OBJECT_DIR)/%.o : src/%.c src/*.h
	$(CC) $(flags) -c $< -o $@

# Generate default plugin libraries
$(LIB_DIR)/%.so : src/libs/%.c src/libs/%.h
	$(CC) $(flags) -shared -fPIC $< -o $@

# Run ttysys for testing
run :
	make clean
	make all
	$(TARGET_DIR)/$(out)

# Clean up the build directories
clean :
	-rm -rf $(TARGET_DIR) $(OBJECT_DIR) $(LIB_DIR) 2> /dev/null

# Install the executables
install : all
	cp $(TARGET_DIR)/ttysys /usr/local/bin/

# Uninstall the executables
uninstall :
	-rm /usr/local/bin/ttysys 2> /dev/null
