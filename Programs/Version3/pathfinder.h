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
#include <unordered_map>
#include <limits>
#include <filesystem>
#include <mutex>
#include <functional>
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
 * Struct to store the information found for the lowest cost path.
 */
struct LowestCostPath
{
    std::vector<int> nodes;                // The nodes in the path
    std::vector<std::pair<int, int>> path; // The positions of the cells traveled in the path
    float cost;                            // The total cost of the path
};

/**
 * Find the cheapest path between the starting and destination node along the cost grid. First all valid paths of nodes is
 * found. Then for each valid path, fork a child process to output the nodes traversed to a scrap file. Each child process
 * forks grandchild processes to compute the lowest cost subpath between each node pairing using Dijkstra's algorithm.
 * The cells of the grid traversed are written to scrap files. Finally, the cost of each path is computed and the lowest
 * cost path is outputed via the parent process.
 *
 * @param graph The graph to search for the path
 * @param grid The cost grid to provide the bounds and weights for the graph
 * @param startingNode The index of the starting node
 * @param validPaths A vector of vectors containing the valid paths found
 * @param scrapFolderPath The path to the folder where scrap files will be stored
 * @return LowestCostPath The information found for the lowest cost path, including the nodes in the path, the cost of the path, and the positions of the cells traveled in the path
 */
LowestCostPath findCheapestPath(Graph &graph, std::vector<std::vector<float>> &grid, std::vector<std::vector<int>> validPaths, int startingNode, std::string scrapFolderPath);

/** Custom hash function for pairs to be used in the subpath cache
 * Combines the hash values of the two elements in the pair
 * 
 * Reference: https://ianyepan.github.io/posts/cpp-custom-hash/
 */
struct PairHash
{
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const
    {
        // Combine the hash values of the two elements in the pair
        std::size_t h1 = std::hash<T1>{}(p.first);
        std::size_t h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1); // Bitwise combine with a shift
    }

    template <typename T1, typename T2, typename T3, typename T4>
    std::size_t operator()(const std::pair<std::pair<T1, T2>, std::pair<T3, T4>> &p) const
    {
        // Hash the inner pairs individually and combine
        std::size_t h1 = (*this)(p.first);
        std::size_t h2 = (*this)(p.second);
        return h1 ^ (h2 << 1); // Bitwise combine with a shift
    }
};

/**
 * Given a one of the valid paths on the graph, fork a grandchild process for each node pairing in the path
 * and compute the lowest cost subpath between each node pairing and write the results to a scrap file.
 *
 * @param startPos The starting position of the subpath
 * @param endPos The ending position of the subpath
 * @param grid The cost grid to provide the bounds and weights for the graph
 * @param scrapFolderPath The path to the folder where scrap files will be stored.
 * @param pathIndex The index of the current path being processed.
 * @param subPathIndex The index of the current subpath (nodes in the path) being processed.
 */
void findCheapestSubpath(std::pair<int, int> startPos, std::pair<int, int> endPos, const std::vector<std::vector<float>> &grid, const std::string &scrapFolderPath, size_t pathIndex, size_t subPathIndex);

// Define direction vectors for moving in 8 possible directions on the cost grid
const std::vector<std::pair<int, int>> directions = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

/**
 * Implements the A* pathfinding algorithm to find the lowest cost path between two positions in a grid.
 * The algorithm uses a priority queue to visit cells in order of lowest cost and tracks the cost of the lowest
 * cost path to each cell. The path is reconstructed by backtracking from the end position to the start position.
 * Outputs debug information at each step.
 *
 * @param grid A 2D vector representing the grid with costs for each cell.
 * @param path A vector to store the resulting path as a sequence of (row, col) pairs.
 * @param startPos The starting position as a pair of (row, col).
 * @param endPos The ending position as a pair of (row, col).
 * @param startRow The starting row index of the subgrid to consider.
 * @param endRow The ending row index of the subgrid to consider.
 * @param startCol The starting column index of the subgrid to consider.
 * @param endCol The ending column index of the subgrid to consider.
 * @param scrapFolderPath The path to the folder where scrap files will be stored.
 * @param pathIndex The index of the path (used for debugging purposes).
 * @param subPathIndex The index of the subpath (used for debugging purposes).
 * @return The total cost of the lowest cost path found.
 */
float aStar(const std::vector<std::vector<float>> &grid, std::vector<std::pair<int, int>> &path, std::pair<int, int> startPos, std::pair<int, int> endPos, int startRow, int endRow, int startCol, int endCol, std::string scrapFolderPath, size_t pathIndex, size_t subPathIndex);

/**
 * Determine the lowest cost path's information by first going through the current child that represents a valid path of
 * nodes and then for each pair of nodes, read the grandchild file to find the positions on the cost grid it traveled and
 * sum the cost of each grandchild to finally determine which path was the cheapest.
 *
 * @param scrapFolderPath The path to the folder where scrap files are stored.
 * @param pathIndex The index of the current path being processed.
 * @param startPos The starting position of the subpath due to not being included in the grandchild files.
 * @return LowestCostPath The information found for the lowest cost path, including the nodes in the path, the cost of the path, and the positions of the cells traveled in the path
 */
LowestCostPath computePathCost(const std::string &scrapFolderPath, size_t pathIndex, std::pair<int, int> startPos);

/**
 * Reads a child file to get the nodes along the path
 *
 * @param filePath The path to the child file in the scrap folder
 * @return std::vector<int> The nodes along the path
 */
std::vector<int> readChildPath(const std::string &filePath);

/**
 * Reads a grandchild file to get the positions of the cells traversed in the subpath
 *
 * @param filePath The path to the grandchild file in the scrap folder
 * @return std::vector<std::pair<int, int>> The positions of the cells traversed in the subpath
 */
float readGrandchildSubpath(const std::string &filePath, std::vector<std::pair<int, int>> &path);

/**
 * Output the final results of the best path found, which includes
 * - the length of the best node path found and a list of nodes that make up the path
 * - the length of the best grid path found and a list of coordinates that this path traverses
 * - the total cost of the best path found
 *
 * @param bestPath The information found for the lowest cost path, including the nodes in the path, the cost of the path, and the positions of the cells traveled in the path
 * @param outputFilePath The path to the file where the results will be written
 */
void outputLowestCostPath(LowestCostPath bestPath, const std::string &outputFilePath);

/**
 * Removes all files in the scrap folder
 *
 * @param scrapFolderPath The path to the folder where scrap files are stored
 */
void removeScrapFiles(const std::string &scrapFolderPath);

#ifdef DEBUG
/**
 * Test the graph by writing the nodes and adjacency list to output files.
 *
 * @param graph The graph to test
 */
void testGraph(Graph &graph);

/**
 * Test the graph's paths by writing them to a file and then generate all the possible paths and write
 * them to a file
 *
 * @param graph The graph to test
 * @param start The index of the starting node
 * @param dest The index of the ending node
 */
void outputAllGraphPaths(Graph &graph, int start, int dest);

/**
 * Outputs the graph's paths to a file in the format of a list of node indices separated by spaces.
 * Each path is written to a new line.
 *
 * @param paths A vector of vectors containing the possible paths in the graph.
 * @param filename The name of the file to write the paths to.
 */
void writePathsToFile(std::vector<std::vector<int>> &paths, std::string filename);
#endif // DEBUG

#endif // PATHFINDER_H
