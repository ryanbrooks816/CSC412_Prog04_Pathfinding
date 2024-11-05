#ifndef GRAPH_H
#define GRAPH_H

#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <queue>
#include <unordered_set>

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
     * Get the adjacency list of the graph.
     *
     * @return std::vector<std::unordered_set<int>> The adjacency list
     */
    const std::vector<std::unordered_set<int>> &getAdjList() const;

    /**
     * Get the adjacent nodes given the index of a node from the adjacency list.
     *
     * @param i The index of a node in the adjacency list.
     * @return std::unordered_set<int> The adjacency list of a path in the graph.
     */
    const std::unordered_set<int> &getAdjNodes(int i) const;

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
