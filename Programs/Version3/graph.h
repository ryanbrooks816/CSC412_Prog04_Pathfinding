#ifndef GRAPH_H
#define GRAPH_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include "testing.h"

struct Node
{
    int idx;
    std::pair<int, int> pos;
};

class Graph
{
private:
    const int numClosestNodes = 3;
    std::vector<Node> nodes;
    std::vector<std::unordered_set<int>> adjList;

public:
    /**
     * Constructs a Graph object with the specified nodes.
     *
     * This constructor reads a file containing node information and initializes
     * the graph with the specified nodes. The file should have the following format:
     * - The first line contains the number of nodes.
     * - The second line contains the row and column indices of each node, separated by spaces.
     *
     * @param nodesPath The path to the file containing the node information.
     */
    Graph(std::string nodesPath);

    /**
     * Creates an adjacency list to represent the graph's edges by connecting the numClosestNodes closest
     * nodes based on Manhattan distance. The adjacency list is a vector of nodes by in order of node idx
     * and an inner set that contains the indices of the closest nodes.
     *
     * Invariants: The graph contains at least 2 nodes and the nodes list is valid.
     *
     * @return void, simply updates the adjList member variable
     */
    void findClosestNodes();

    /**
     * Find all valid paths from the starting node to the destination node. A valid path must contain at least minNodes nodes
     * and at most maxNodes nodes.
     *
     * @param start The index of the starting node
     * @param dest The index of the ending node
     * @return std::vector<std::vector<int>> A vector of vectors containing the valid paths found
     */
    std::vector<std::vector<int>> findValidPaths(int start, int dest);

    /**
     * Recursive function to be called by findValidPaths to explore all possible paths from the current node to the destination node.
     *
     * @param path A vector to store the current path being explored.
     * @param validPaths A vector of vectors to store all valid paths found.
     * @param current The current node being explored.
     * @param dest The destination node.
     * @param minNodes The minimum number of nodes a valid path must contain.
     * @param maxNodes The maximum number of nodes a valid path can contain.
     */
    void findValidPath(std::vector<int> &path, std::vector<std::vector<int>> &validPaths, int current, int dest, unsigned int minNodes, unsigned int maxNodes);

    /**
     * Get the number of nodes in the graph.
     *
     * @return int The number of nodes in the graph.
     */
    int getNumNodes() const;

    /**
     * Get the nodes in the graph.
     *
     * @return std::vector<Node> The nodes in the graph.
     */
    std::vector<Node> getNodes() const;

    /**
     * Outputs a string version of the nodes in the graph in the format of the node index
     * followed by its row and column indices.
     *
     * @return std::string The nodes in the graph.
     */
    std::string printNodes() const;

    /**
     * Outputs a string version of adjacency list for the graph in the format of the node index
     * followed by its adjacent nodes.
     *
     * @return std::string The adjacency list of the graph.
     */
    std::string printAdjList() const;
};

#endif // GRAPH_H
