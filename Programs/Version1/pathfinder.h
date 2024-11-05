#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>
#include "graph.h"
#include "testing.h"

/**
 * Reads a grid from a file and constructs a matrix of floats representing the cost grid
 *
 * @param gridPath The path to the file containing the grid
 * @return std::vector<std::vector<float>> A matrix of floats representing the cost grid
 */
std::vector<std::vector<float>> createCostGrid(std::string gridPath);

/**
 * Throws an error if the graph is out of bounds based on the cost grid.
 *
 * Invariants: graph contains a list of nodes at non-negative positions.
 *
 * @param graph The graph to overlay on the cost grid
 * @param grid The cost grid to provide the bounds and weights for the graph
 * @return bool True if the graph is within the bounds of the cost grid, false otherwise
 */
bool overlayGraph(Graph &graph, std::vector<std::vector<float>> &grid);

/**
 * Find the cheapest path between the starting and destination node by trying all the possible
 * paths in the adjacency list and outputs each chain of nodes to a file in the scrap folder.
 *
 * Invariants: graph and nodes are valid, and the index of node1 and node2 correspond to the same node
 * index order as in the adjacency list.
 *
 * @param graph The graph to search for the path
 * @param start The index of the starting node
 * @param dest The index of the ending node
 * @param scrapFolderPath The path to the folder where scrap files will be stored
 */
void findCheapestPath(Graph &graph, int start, int dest, std::string scrapFolderPath);

/**
 * Explores all possible paths from the current node to the destination node and stores the
 * valid paths in the validPaths vector. A path must contain  contains at least minNodes nodes and
 * at most maxNodes nodes.
 *
 * @param graph The graph to search for paths.
 * @param path A vector to store the current path being explored.
 * @param validPaths A vector of vectors to store all valid paths found.
 * @param current The current node being explored.
 * @param dest The destination node.
 * @param minNodes The minimum number of nodes a valid path must contain.
 * @param maxNodes The maximum number of nodes a valid path can contain.
 */
void findValidPaths(Graph &graph, std::vector<int> &path, std::vector<std::vector<int>> &validPaths, int current, int dest, unsigned int minNodes, unsigned int maxNodes);

/**
 * Output the final results of the best path found. For this version it's blank.
 *
 * @param outputFilePath The path to the file where the results will be written
 */
void outputLowestCostPath(const std::string &outputFilePath);

/**
 * Outputs the graph's paths to a file in the format of a list of node indices separated by spaces.
 * Each path is written to a new line.
 *
 * @param paths A vector of vectors containing the possible paths in the graph.
 * @param filename The name of the file to write the paths to.
 */
#ifdef DEBUG
void testValidPaths(std::vector<std::vector<int>> &paths, std::string filename);
#endif

#endif // PATHFINDER_H
