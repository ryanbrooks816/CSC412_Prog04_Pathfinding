#include "pathfinder.h"

/**
 * Reads a grid from a file and constructs a matrix of floats representing the cost grid
 *
 * @param gridPath The path to the file containing the grid
 * @return std::vector<std::vector<float>> A matrix of floats representing the cost grid
 */
std::vector<std::vector<float>> createCostGrid(std::string gridPath)
{
    // Open the file
    std::ifstream gridFile(gridPath);

    // Read the first line to get the dimensions of the grid
    std::string line;
    std::getline(gridFile, line);
    std::istringstream iss(line);
    int width, height;
    if (!(iss >> width >> height))
    {
        throw std::runtime_error("Error reading grid dimensions from file: " + gridPath);
    }

    if (width <= 0 || height <= 0)
    {
        throw std::invalid_argument("Grid dimensions must be positive. Given: width=" + std::to_string(width) + ", height=" + std::to_string(height));
    }

    std::vector<std::vector<float>> grid = std::vector<std::vector<float>>(height, std::vector<float>(width));

    // Read the rest of the file to get the grid
    for (int i = 0; i < height; i++)
    {
        if (!std::getline(gridFile, line))
        {
            throw std::runtime_error("Error reading line " + std::to_string(i + 1) + " from file: " + gridPath);
        }
        std::istringstream iss(line);
        for (int j = 0; j < width; j++)
        {
            if (!(iss >> grid[i][j]))
            {
                throw std::runtime_error("Error reading grid value at row " + std::to_string(i) + ", column " + std::to_string(j) + " from file: " + gridPath);
            }
        }
    }

    return grid;
}

/**
 * Throws an error if the graph is out of bounds based on the cost grid.
 *
 * Invariants: graph contains a list of nodes at non-negative positions.
 *
 * @param graph The graph to overlay on the cost grid
 * @param grid The cost grid to provide the bounds and weights for the graph
 * @return bool True if the graph is within the bounds of the cost grid, false otherwise
 */
bool overlayGraph(Graph &graph, std::vector<std::vector<float>> &grid)
{
    DEBUG_CONSOLE("Grid size " + std::to_string(grid.size()) + " " + std::to_string(grid[0].size()));

    for (Node node : graph.getNodes())
    {
        DEBUG_CONSOLE("Checking node: " + std::to_string(node.idx) + " at position: " + std::to_string(node.pos.first) + ", " + std::to_string(node.pos.second));

        if (static_cast<size_t>(node.pos.first) >= grid.size() || static_cast<size_t>(node.pos.second) >= grid[0].size())
        {
            throw std::invalid_argument("Node " + std::to_string(node.idx) + " is out of bounds.");
            return false;
        }
    }
    return true;
}

/**
 * Find the cheapest path between the starting and destination node by trying all the possible
 * paths in the adjacency list and outputs each chain of nodes to a file in the scrap folder. It then, for each valid path,
 * forks a grandchild process to output each node pairing to a scrap file in order to determine the cheapest subpath from node to node.
 *
 * Invariants: graph and nodes are valid and within bounds, and the index of node1 and node2 correspond to the same node
 * index order as in the adjacency list.
 *
 * @param graph The graph to search for the path
 * @param start The index of the starting node
 * @param dest The index of the ending node
 * @param scrapFolderPath The path to the folder where scrap files will be stored
 */
void findCheapestPath(Graph &graph, int start, int dest, std::string scrapFolderPath)
{
    if (graph.getAdjList().size() == 0)
    {
        std::cout << "No valid paths found in the graph." << std::endl;
        return;
    }

    const unsigned int minNodes = 3;
    const unsigned int maxNodes = 5;

    std::vector<int> path;

    // Find all valid paths from the starting node to the destination node
    std::vector<std::vector<int>> validPaths;

    findValidPaths(graph, path, validPaths, start, dest, minNodes, maxNodes);

    if (validPaths.size() == 0)
    {
        std::cout << "No valid paths found in the graph." << std::endl;
        return;
    }

// Test the graph's paths by writing them to a file and then generate all the possible paths and write them to a file
#ifdef DEBUG
    writePathsToFile(validPaths, "tree_valid.txt");
    outputAllGraphPaths(graph, start, dest);
#endif

    // For each valid path, fork a child process to output the path to a scrap file
    for (size_t i = 0; i < validPaths.size(); i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            std::string scrapFilePath = scrapFolderPath + "/child_" + std::to_string(i) + ".txt";
            std::ofstream scrapFile(scrapFilePath);

            for (int node : validPaths[i])
            {
                scrapFile << node << " ";
            }

            scrapFile.close();

            // Now for each valid path, fork a grandchild process to output each node pairing to a scrap file
            // in order to determine the cheapest subpath from node to node
            findCheapestSubpath(validPaths[i], scrapFolderPath, i);

            exit(0);
        }
    }

    // Wait for all child processes to finish
    int status = 0;
    while (wait(&status) > 0)
        ;
}

/**
 * Given a one of the valid paths on the graph, fork a grandchild process for each node pairing in the path
 * and writes the current node and the next node to a scrap file.
 *
 * @param validPath The vector of nodes indices in the valid path.
 * @param scrapFolderPath The path to the folder where scrap files will be stored.
 * @param pathIndex The index of the current path being processed.
 */
void findCheapestSubpath(const std::vector<int> &validPath, const std::string &scrapFolderPath, size_t pathIndex)
{
    for (size_t j = 0; j < validPath.size() - 1; j++)
    {
        pid_t grandchildPid = fork();
        if (grandchildPid == 0)
        {
            std::string grandchildFilePath = scrapFolderPath + "/grandchild_" + std::to_string(pathIndex) + "_" + std::to_string(j) + ".txt";
            std::ofstream grandchildFile(grandchildFilePath);

            grandchildFile << validPath[j] << " " << validPath[j + 1];

            grandchildFile.close();
            exit(0);
        }
    }

    // Wait for all grandchild processes to finish
    int status = 0;
    while (wait(&status) > 0)
        ;
}

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
void findValidPaths(Graph &graph, std::vector<int> &path, std::vector<std::vector<int>> &validPaths, int current, int dest, unsigned int minNodes, unsigned int maxNodes)
{
    path.push_back(current);

#ifdef DEBUG
    DEBUG_FILE("Current path: ", "debug_valid_paths.txt", false);
    for (int node : path)
    {
        DEBUG_FILE(std::to_string(node) + " ", "debug_valid_paths.txt", false);
    }
    DEBUG_FILE("", "debug_valid_paths.txt");
#endif

    // Base case: if we've reached the destination, check if the path length is valid
    if (current == dest)
    {
        if (path.size() >= minNodes && path.size() <= maxNodes)
        {
            validPaths.push_back(path);

#ifdef DEBUG
            DEBUG_FILE("Found valid path: ", "debug_valid_paths.txt", false);
            for (int node : path)
            {
                DEBUG_FILE(std::to_string(node) + " ", "debug_valid_paths.txt", false);
            }
            DEBUG_FILE("", "debug_valid_paths.txt");
#endif
        }
        else
        {
            DEBUG_FILE("Path does not meet node constraints. Length: " + std::to_string(path.size()), "debug_valid_paths.txt");
        }
    }
    else
    {

#ifdef DEBUG
        DEBUG_FILE("Possible neighbors: ", "debug_valid_paths.txt", false);
        for (int neighbor : graph.getAdjNodes(current))
        {
            DEBUG_FILE(std::to_string(neighbor) + " ", "debug_valid_paths.txt", false);
        }
        DEBUG_FILE("", "debug_valid_paths.txt");
#endif

        // Path is not valid yet, so continue exploring the neighboring nodes
        for (int neighbor : graph.getAdjNodes(current))
        {
            // Avoid revisiting nodes within the same path
            if (std::find(path.begin(), path.end(), neighbor) == path.end())
            {
                DEBUG_FILE("Exploring neighbor: " + std::to_string(neighbor) + " from node: " + std::to_string(current), "debug_valid_paths.txt");
                findValidPaths(graph, path, validPaths, neighbor, dest, minNodes, maxNodes);
            }
        }
    }

    // Remove the current node from the path to backtrack
    DEBUG_FILE("Backtracking from node: " + std::to_string(current), "debug_valid_paths.txt");
    path.pop_back();
}

/**
 * Output the final results of the best path found. For this version it's blank.
 *
 * @param outputFilePath The path to the file where the results will be written
 */
void outputLowestCostPath(const std::string &outputFilePath)
{
    std::ofstream outputFile(outputFilePath);
    outputFile << std::endl;
    outputFile.close();
}

/**
 * Test the graph's paths by writing them to a file and then generate all the possible paths and write
 * them to a file
 *
 * @param graph The graph to test
 * @param start The index of the starting node
 * @param dest The index of the ending node
 */
void outputAllGraphPaths(Graph &graph, int start, int dest)
{
    std::vector<int> path;
    std::vector<std::vector<int>> allPaths;
    findValidPaths(graph, path, allPaths, start, dest, 0, graph.getNumNodes());

    if (allPaths.size() == 0)
    {
        std::cout << "No valid paths found in the graph." << std::endl;
        return;
    }

    writePathsToFile(allPaths, "tree.txt");
}

/**
 * Outputs the graph's paths to a file in the format of a list of node indices separated by spaces.
 * Each path is written to a new line.
 *
 * @param paths A vector of vectors containing the possible paths in the graph.
 * @param filename The name of the file to write the paths to.
 */
void writePathsToFile(std::vector<std::vector<int>> &paths, std::string filename)
{
    // Write the valid paths to an output file
    std::ofstream outFile(filename);
    for (size_t i = 0; i < paths.size(); i++)
    {
        for (int node : paths[i])
        {
            outFile << node << " ";
        }
        outFile << std::endl;
    }
    outFile.close();
}
