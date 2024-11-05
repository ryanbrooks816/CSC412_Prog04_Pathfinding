#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 8 ]; then
    echo "Usage: $0 <base_exe_name> <rows> <cols> <num_nodes> <start_node> <end_node> <scrap_folder> <output_folder>"
    exit 1
fi

# Assign arguments to variables
BASE_EXE_NAME=$1
ROWS=$2
COLS=$3
NUM_NODES=$4
START_NODE=$5
END_NODE=$6
SCRAP_FOLDER=$7
OUTPUT_FOLDER=$8

# Check if rows and columns are non-negative
if [ "$ROWS" -lt 0 ] || [ "$COLS" -lt 0 ]; then
    echo "Error: Rows and columns must be non-negative."
    exit 2
fi

# Check if the number of nodes is non-negative and less than the number of cells in the grid
if [ "$NUM_NODES" -lt 0 ] || [ "$NUM_NODES" -ge $(($ROWS * $COLS)) ]; then
    echo "Error: Number of nodes must be non-negative and less than the number of cells in the grid."
    exit 3
fi

# Check if start and end nodes are non-negative and within the number of nodes
if [ "$START_NODE" -lt 0 ] || [ "$START_NODE" -ge "$NUM_NODES" ] || [ "$END_NODE" -lt 0 ] || [ "$END_NODE" -ge "$NUM_NODES" ]; then
    echo "Error: Start and end nodes must be non-negative and within the range of the number of nodes."
    exit 4
fi

# Generate grid file
GRID_FILE="$OUTPUT_FOLDER/grid.txt"
echo "Generating grid with ${ROWS} rows and ${COLS} columns..."

# Write to a text file first the number of columns and rows and then the grid values
# The grid values are floats generated randomly between 0 and 10 with 5 decimal places
echo "${COLS} ${ROWS}" > $GRID_FILE
for i in $(seq 1 $ROWS); do
    for j in $(seq 1 $COLS); do
        printf "%.5f " $(echo "scale=5; $RANDOM/32768*10" | bc)
    done
    echo
done >> $GRID_FILE

# Generate node list file
NODES_FILE="$OUTPUT_FOLDER/nodes.txt"
echo "Generating ${NUM_NODES} nodes..."

# Write to a text file first the number of nodes and then the node positions
# Node positions are generated randomly but unique and must be within the grid
echo "${NUM_NODES} " > $NODES_FILE
declare -A used_positions
for i in $(seq 1 $NUM_NODES); do
    while : ; do
        row=$(($RANDOM % $ROWS))
        col=$(($RANDOM % $COLS))
        pos="${row}_${col}"
        if [ -z "${used_positions[$pos]}" ]; then
            used_positions[$pos]=1
            echo -n "${row} ${col} "
            break
        fi
    done
done >> $NODES_FILE

# Perform the search for the lowest-cost path
echo "Searching for the lowest-cost path between nodes ${START_NODE} and ${END_NODE}..."
./Scripts/run.sh $BASE_EXE_NAME $GRID_FILE $NODES_FILE $START_NODE $END_NODE $SCRAP_FOLDER $OUTPUT_FOLDER

echo "Script execution completed."
