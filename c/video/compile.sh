#!/bin/bash

# Define source files
SRC_FILES="bitmap.c main.c"

OUTPUT="vid"

LIBS="-lavformat -lavcodec -lavutil -lswscale"

INCLUDES="-I/usr/include -I/usr/local/include"

LIB_DIRS="-L/usr/lib -L/usr/local/lib"

CFLAGS="-Wno-incompatible-pointer-types"

# Compile and link the program
gcc $CFLAGS $SRC_FILES $INCLUDES $LIB_DIRS -o $OUTPUT $LIBS

# Check if the compilation succeeded
if [ $? -eq 0 ]; then
  echo "Compilation successful. Executable: $OUTPUT"
else
  echo "Compilation failed."
fi