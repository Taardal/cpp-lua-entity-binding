#!/bin/bash

# Exit when any command fails
set -e

# Compile lua files
luac -o scripts *.lua

# Generate build files
cmake -S . -B build

# Compile binary
cmake --build build

# Run binary
./build/luapoc