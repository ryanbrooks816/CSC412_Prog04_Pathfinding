#!/bin/bash

# Check if exactly one argument is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <executable prefix>"
    exit 1
fi

EXE_PREFIX=$1

# Compile the programs for all versions
for version in {1..3}; do
    g++ -Wall -std=c++20 ./Programs/Version$version/*.cpp -o "${EXE_PREFIX}${version}"
    if [ $? -ne 0 ]; then
        echo "Build failed for Version$version"
        exit 1
    fi
done
