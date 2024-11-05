#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 7 ]; then
    echo "Usage: $0 <base_exe_name> <grid_file> <nodes_file> <start_node> <end_node> <scrap_folder> <output_folder>"
    exit 1
fi

# Assign arguments to variables
BASE_EXE_NAME=$1
GRID_FILE=$2
NODES_FILE=$3
START_NODE=$4
END_NODE=$5
SCRAP_FOLDER=$6
OUTPUT_FOLDER=$7

# Build the executables if they don't exist
for version in 1 2 3; do
    EXECUTABLE="${BASE_EXE_NAME}${version}"
    if [ ! -f "${EXECUTABLE}" ]; then
        echo "Building ${EXECUTABLE}..."
        ./Scripts/build.sh "${BASE_EXE_NAME}"
        break
    fi
done

# Launch the executables with the proper arguments
for version in 1 2 3; do
    EXECUTABLE="${BASE_EXE_NAME}${version}"
    SCRAP_DIR="${SCRAP_FOLDER}${version}"
    OUTPUT_FILE="${OUTPUT_FOLDER}/${version}.txt"
    
    echo "Running ${EXECUTABLE}..."
    ./"${EXECUTABLE}" "${GRID_FILE}" "${NODES_FILE}" "${START_NODE}" "${END_NODE}" "${SCRAP_DIR}" "${OUTPUT_FILE}"
done