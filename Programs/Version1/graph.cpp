#include "graph.h"
#include <iostream>
#include <unordered_set>

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
Graph::Graph(std::string nodesPath)
{
    // Read the nodes file
    std::ifstream nodesFile(nodesPath);

    // Read the first line to get the number of nodes
    std::string line;
    std::getline(nodesFile, line);
    std::istringstream iss(line);
    int numNodes;
    if (!(iss >> numNodes))
    {
        throw std::runtime_error("Failed to parse the number of nodes from file: " + nodesPath);
    }

    nodes = std::vector<Node>(numNodes);

    // Read the next line of the file to get a list of nodes' row and columns indecies separated by spaces
    std::getline(nodesFile, line);
    std::istringstream iss2(line);
    for (int i = 0; i < numNodes; i++)
    {
        int row, col;
        if (!(iss2 >> row >> col))
        {
            throw std::runtime_error("Failed to parse row and column indices for node " + std::to_string(i) + " from file: " + nodesPath);
        }

        if (row < 0 || col < 0)
        {
            throw std::invalid_argument("Row and column indices must be non-negative. Given: row=" + std::to_string(row) + ", col=" + std::to_string(col));
        }

        nodes[i] = Node{i, std::make_pair(row, col)};
    }
    nodesFile.close();

    if (this->getNumNodes() < 2)
    {
        throw std::invalid_argument("The graph must contain at least 2 nodes.");
    }
}

/**
 * Creates an adjacency list to represent the graph's edges by connecting the numClosestNodes closest
 * nodes based on Manhattan distance. The adjacency list is a vector of nodes by in order of node idx
 * and an inner set that contains the indices of the closest nodes.
 *
 * Invariants: The graph contains at least 2 nodes and the nodes list is valid.
 *
 * @return void, simply updates the adjList member variable
 */
void Graph::findClosestNodes()
{
    // Lambda function to calculate the Manhattan distance between two nodes
    auto manhattanDistance = [](Node a, Node b)
    {
        return std::abs(a.pos.first - b.pos.first) + std::abs(a.pos.second - b.pos.second);
    };

    // Create an adjacency list to represent the graph's edges
    this->adjList = std::vector<std::unordered_set<int>>(nodes.size());

    for (size_t i = 0; i < nodes.size(); ++i)
    {
        // Create a min priority queue to keep track of the 3 smallest distances
        std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>> minHeap;

        // Go through all the other nodes and calculate the distance with the current node
        // then add them to the queue
        for (size_t j = 0; j < nodes.size(); ++j)
        {
            if (i != j)
            {
                int distance = manhattanDistance(nodes[i], nodes[j]);
                minHeap.push({distance, j}); // Push the (distance, node idx) pair to minHeap
            }
        }

        // Add the smallest numClosestNodes distances to the adjacency list
        for (int k = 0; k < this->numClosestNodes && !minHeap.empty(); ++k)
        {
            int neighborIdx = minHeap.top().second;
            minHeap.pop();

            // Add the neighbor in both directions to make the graph undirected
            this->adjList[i].insert(neighborIdx);
            this->adjList[neighborIdx].insert(i);
        }
    }
}

/**
 * Get the adjacency list of the graph.
 *
 * @return std::vector<std::unordered_set<int>> The adjacency list
 */
const std::vector<std::unordered_set<int>> &Graph::getAdjList() const
{
    return this->adjList;
}

/**
 * Get the adjacent nodes given the index of a node from the adjacency list.
 *
 * @param i The index of a node in the adjacency list.
 * @return std::vector<int> The adjacency list of a path in the graph.
 */
const std::unordered_set<int> &Graph::getAdjNodes(int i) const
{
    return this->adjList[i];
}

/**
 * Get the nodes in the graph.
 *
 * @return std::vector<Node> The nodes in the graph.
 */
std::vector<Node> Graph::getNodes() const
{
    return this->nodes;
}

/**
 * Get the number of nodes in the graph.
 *
 * @return int The number of nodes in the graph.
 */
int Graph::getNumNodes() const
{
    return this->nodes.size();
}

/**
 * Outputs a string version of the nodes in the graph in the format of the node index
 * followed by its row and column indices.
 *
 * @return std::string The nodes in the graph.
 */
std::string Graph::printNodes() const
{
    std::ostringstream output;
    for (int i = 0; i < this->getNumNodes(); ++i)
    {
        output << i << " " << this->nodes[i].pos.first << " " << this->nodes[i].pos.second << "\n";
    }
    return output.str();
}

/**
 * Outputs a string version of adjacency list for the graph in the format of the node index
 * followed by its adjacent nodes.
 *
 * @return std::string The adjacency list of the graph.
 */
std::string Graph::printAdjList() const
{
    std::ostringstream output;
    for (int i = 0; i < this->getNumNodes(); ++i)
    {
        output << i << " ";
        for (int node : this->getAdjNodes(i))
        {
            output << node << " ";
        }
        output << "\n";
    }
    return output.str();
}
