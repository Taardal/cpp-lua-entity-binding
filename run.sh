#!/bin/bash

# Exit when any command fails
set -e

# Generate build files
cmake -S . -B build

# Compile binary
cmake --build build

# Install binary
cmake --install build

# Compile lua files
luac src/*.lua

# Run binary
./app
