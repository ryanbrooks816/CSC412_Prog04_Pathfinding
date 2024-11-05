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
 * Cache to store the results of previously computed subpaths.
 * The key represents a pair of start and end positions.
 * The value is a pair consisting of the cost of the subpath and the path itself.
 * The hash function is a custom hash function for pairs.
 */
std::unordered_map<std::pair<std::pair<int, int>, std::pair<int, int>>, std::pair<float, std::vector<std::pair<int, int>>>, PairHash> subpathCache;

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
LowestCostPath findCheapestPath(Graph &graph, std::vector<std::vector<float>> &grid, std::vector<std::vector<int>> validPaths, int startingNode, std::string scrapFolderPath)
{
    // Store the lowest cost path found
    LowestCostPath bestPath = {std::vector<int>(), std::vector<std::pair<int, int>>(), std::numeric_limits<float>::max()};

    // For each valid path, fork a child process to to explore each path and output the results to a scrap file
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

            // Now for each pair of nodes in the current path, fork a grandchild process to output the lowest cost subpath
            // between the pair of nodes to a scrap file
            for (size_t j = 0; j < validPaths[i].size() - 1; j++)
            {
                pid_t grandchildPid = fork();
                if (grandchildPid == 0)
                {
                    // Get the start and end node positions of the subpath
                    Node startNode = graph.getNodes()[validPaths[i][j]];
                    Node endNode = graph.getNodes()[validPaths[i][j + 1]];

                    // Compute the positions traveled and the total cost for each pair of nodes
                    findCheapestSubpath(startNode.pos, endNode.pos, grid, scrapFolderPath, i, j);
                    exit(0);
                }
                else if (grandchildPid < 0)
                {
                    std::cerr << "Error forking grandchild process." << std::endl;
                    exit(80);
                }
            }

            // Wait for all grandchild processes to finish
            int status = 0;
            while (wait(&status) > 0)
                ;

            exit(0);
        }
        else if (pid < 0)
        {
            std::cerr << "Error forking child process." << std::endl;
            exit(80);
        }

        DEBUG_CONSOLE("Child process " + std::to_string(i) + " forked.");

        // Wait for all child processes to finish
        int status = 0;
        while (wait(&status) > 0)
            ;

        // Child has now finished, so the parent process will compute the cost of this path
        LowestCostPath pathCost = computePathCost(scrapFolderPath, i, graph.getNodes()[startingNode].pos);

        // Update the lowest cost path if the current path has a lower cost
        if (pathCost.cost < bestPath.cost)
        {
            DEBUG_CONSOLE(std::to_string(pathCost.cost) + " is less than " + std::to_string(bestPath.cost) + ". Updating lowest cost.");
            bestPath = pathCost;
        }
    }

    // Remove all scrap files
    // removeScrapFiles(scrapFolderPath);

    return bestPath;
}

/**
 * Given a one of the valid paths on the graph, fork a grandchild process for each node pairing in the path
 * and compute the lowest cost subpath between each node pairing using the A* algorithm. Uses memozation: if
 * the result is cached, it writes the cached result to the output file. Otherwise, it computes the path,
 * caches the result, and writes it to the file.
 *
 * @param startPos The starting position of the subpath
 * @param endPos The ending position of the subpath
 * @param grid The cost grid to provide the bounds and weights for the graph
 * @param scrapFolderPath The path to the folder where scrap files will be stored.
 * @param pathIndex The index of the current path being processed.
 * @param subPathIndex The index of the current subpath (nodes in the path) being processed.
 */
void findCheapestSubpath(std::pair<int, int> startPos, std::pair<int, int> endPos, const std::vector<std::vector<float>> &grid, const std::string &scrapFolderPath, size_t pathIndex, size_t subPathIndex)
{
    std::string grandchildFilePath = scrapFolderPath + "/grandchild_" + std::to_string(pathIndex) + "_" + std::to_string(subPathIndex) + ".txt";
    std::ofstream grandchildFile(grandchildFilePath);

    // Compute the subgrid between the start and end positions
    // The subgrid is formed by enclosing the start and end positions in a rectangle padded by 1
    int startRow = std::max(std::min(startPos.first, endPos.first) - 1, 0);
    int endRow = std::min(std::max(startPos.first, endPos.first) + 1, (int)grid.size() - 1);
    int startCol = std::max(std::min(startPos.second, endPos.second) - 1, 0);
    int endCol = std::min(std::max(startPos.second, endPos.second) + 1, (int)grid[0].size() - 1);

    // The key is a pair of starting and ending positions
    // The value is a pair consisting of the total cost of the path and a vector of pairs representing the path coordinates
    std::pair<std::pair<int, int>, std::pair<int, int>> cacheKey = {startPos, endPos}; // Construct the cache key with startPos and endPos

    // Using mutex, lock the cache to avoid race conditions
    std::mutex cacheMutex;
    std::unique_lock<std::mutex> lock(cacheMutex);

    // Check if the result is already in the cache
    auto iter = subpathCache.find(cacheKey);
    if (iter != subpathCache.end())
    {
        // Write the cached results to the output file
        // Total cost
        grandchildFile << iter->second.first << std::endl;

        // Path coordinates
        for (size_t i = 1; i < iter->second.second.size(); ++i)
        {
            grandchildFile << iter->second.second[i].first << " " << iter->second.second[i].second << std::endl;
        }
    }
    else
    {
        // Unlock the mutex while performing the A* algorithm to avoid holding the lock for too long
        lock.unlock();

        // Use the A* algorithm to find the lowest cost subpath between the start and end position
        std::vector<std::pair<int, int>> path;

        // The cost calculation includes the final node but not the starting node
        float totalCost = aStar(grid, path, startPos, endPos, startRow, endRow, startCol, endCol, scrapFolderPath, pathIndex, subPathIndex);

        // Lock the mutex again before updating the cache
        lock.lock();

        // Store the result in the cache
        subpathCache[cacheKey] = {totalCost, path};

        grandchildFile << totalCost << std::endl;

        // Don't include the first position in the path (won't be written to the grandchild file) since it's the starting position
        // If it were to be included, the starting and ending nodes would be duplicated.
        // This has no effect on the cost calculation.
        for (size_t i = 1; i < path.size(); ++i)
        {
            grandchildFile << path[i].first << " " << path[i].second << std::endl;
        }

        grandchildFile.close();
    }
}

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
float aStar(const std::vector<std::vector<float>> &grid, std::vector<std::pair<int, int>> &path, std::pair<int, int> startPos, std::pair<int, int> endPos, int startRow, int endRow, int startCol, int endCol, std::string scrapFolderPath, size_t pathIndex, size_t subPathIndex)
{
    // Debugging
    std::string debugFilePath = scrapFolderPath + "/debug_grandchild_" + std::to_string(pathIndex) + "_" + std::to_string(subPathIndex) + ".txt";

    DEBUG_FILE("Start Position: (" + std::to_string(startPos.first) + ", " + std::to_string(startPos.second) + ")", debugFilePath);
    DEBUG_FILE("End Position: (" + std::to_string(endPos.first) + ", " + std::to_string(endPos.second) + ")", debugFilePath);
    DEBUG_FILE("Subgrid bounds: (" + std::to_string(startRow) + ", " + std::to_string(startCol) + ") to (" + std::to_string(endRow) + ", " + std::to_string(endCol) + ")", debugFilePath);

    // Implement Dijkstra's A* algorithm to find the lowest cost subpath between the start and end positions
    using Cell = std::pair<float, std::pair<int, int>>; // <cost, <row, col>>

    // Use a priority queue to store the cells to visit in order of lowest cost
    std::priority_queue<Cell, std::vector<Cell>, std::greater<Cell>> pq;
    pq.push({0, startPos});

    DEBUG_FILE("Initialized priority queue with start position.", debugFilePath);

    // Initialize the cost matrix with maximum float values to represent infinity
    // This matrix will track the cost of the lowest cost path to each cell
    std::vector<std::vector<float>> cost(grid.size(), std::vector<float>(grid[0].size(), std::numeric_limits<float>::max()));

    // Initialize the predecessors matrix with pairs of (-1, -1)
    // This matrix will track the predecessor of each cell in the path
    std::vector<std::vector<std::pair<int, int>>> predecessors(grid.size(), std::vector<std::pair<int, int>>(grid[0].size(), {-1, -1}));

    // Set the cost of the starting position to 0
    cost[startPos.first][startPos.second] = 0;

    DEBUG_FILE("Initialized cost and predecessor matrices.", debugFilePath);

    float totalCost = 0;

    // Pathfinding loop
    while (!pq.empty())
    {
        // Get the cell with the lowest cost from the priority queue
        auto [currentCost, current] = pq.top();
        pq.pop();

        // Extract the row and column indices of the current cell
        int row = current.first, col = current.second;

        DEBUG_FILE("Visiting cell: (" + std::to_string(row) + ", " + std::to_string(col) + ") with current cost: " + std::to_string(currentCost), debugFilePath);

        // Check if we've reached the destination
        if (current == endPos)
        {
            totalCost = currentCost;
            DEBUG_FILE("Reached end position with total cost: " + std::to_string(totalCost), debugFilePath);
            break;
        }

        // Check the 8 adjacent cells
        for (const std::pair<int, int> &dir : directions)
        {
            int newRow = row + dir.first;
            int newCol = col + dir.second;

            // Check if the new cell is within bounds
            if (newRow >= startRow && newRow <= endRow && newCol >= startCol && newCol <= endCol)
            {
                // Compute the cost to move to the new cell
                float newCost = currentCost + grid[newRow][newCol];
                DEBUG_FILE("Checking cell: (" + std::to_string(newRow) + ", " + std::to_string(newCol) + ")", debugFilePath);
                DEBUG_FILE("New cost = current cost + grid cost = " + std::to_string(currentCost) + " + " + std::to_string(grid[newRow][newCol]) + " = " + std::to_string(newCost), debugFilePath);

                // Update the cost and predecessor if the new cost is lower
                if (newCost < cost[newRow][newCol])
                {
                    cost[newRow][newCol] = newCost;
                    predecessors[newRow][newCol] = {row, col};
                    pq.push({newCost, {newRow, newCol}});

                    DEBUG_FILE("New cost is less than current cost. Updating cost and predecessor.", debugFilePath);
                    DEBUG_FILE("Set predecessor of cell: (" + std::to_string(newRow) + ", " + std::to_string(newCol) + ") to: (" + std::to_string(row) + ", " + std::to_string(col) + ")", debugFilePath);
                }
            }
        }
    }

    // Reconstruct the path from the end position to the start position
    for (std::pair<int, int> current = endPos; current != startPos; current = predecessors[current.first][current.second])
    {
        path.push_back(current);
    }
    path.push_back(startPos);
    std::reverse(path.begin(), path.end());

    return totalCost;
}

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
LowestCostPath computePathCost(const std::string &scrapFolderPath, size_t pathIndex, std::pair<int, int> startPos)
{
    // First open up the child file's that stores the nodes of the path to find out how many nodes are in the path
    std::string scrapFilePath = scrapFolderPath + "/child_" + std::to_string(pathIndex) + ".txt";
    std::vector<int> nodes = readChildPath(scrapFilePath);

    // Find the lowest cost path by summing the costs of the subpaths
    float totalCost = 0;
    std::vector<std::pair<int, int>> path = {startPos}; // Include the starting position in the path

    // Then go through each child's grandchildren files that store the subpaths
    for (size_t subPathIndex = 0; subPathIndex < nodes.size() - 1; subPathIndex++)
    {
        std::string grandchildFilePath = scrapFolderPath + "/grandchild_" + std::to_string(pathIndex) + "_" + std::to_string(subPathIndex) + ".txt";
        totalCost += readGrandchildSubpath(grandchildFilePath, path);
    }

    DEBUG_CONSOLE("Total cost for path " + std::to_string(pathIndex) + ": " + std::to_string(totalCost));

    return {nodes, path, totalCost};
}

/**
 * Reads a child file to get the nodes along the path
 *
 * @param filePath The path to the child file in the scrap folder
 * @return std::vector<int> The nodes along the path
 */
std::vector<int> readChildPath(const std::string &filePath)
{
    std::ifstream childFile(filePath);

    if (!childFile.is_open())
    {
        std::cerr << "Error opening child scrap file: " << filePath << std::endl;
        exit(42);
    }

    std::vector<int> nodes;
    std::ifstream file(filePath);
    int node;
    while (file >> node)
    {
        nodes.push_back(node);
    }

    childFile.close();

    return nodes;
}

/**
 * Reads a grandchild file to get the positions of the cells traversed in the subpath
 *
 * @param filePath The path to the grandchild file in the scrap folder
 * @return std::vector<std::pair<int, int>> The positions of the cells traversed in the subpath
 */
float readGrandchildSubpath(const std::string &filePath, std::vector<std::pair<int, int>> &path)
{
    std::ifstream grandchildFile(filePath);

    if (!grandchildFile.is_open())
    {
        std::cerr << "Error opening grandchild file: " << filePath << std::endl;
        exit(43);
    }

    // Read in the total cost of the subpath from the first line
    float subPathCost;
    grandchildFile >> subPathCost;

    DEBUG_CONSOLE("Value: " + std::to_string(subPathCost));

    // For every line after the first, read in the x and y positions of the cells traversed
    int x, y;
    while (grandchildFile >> x >> y)
    {
        path.emplace_back(x, y);
    }

    grandchildFile.close();

    return subPathCost;
}

/**
 * Output the final results of the best path found, which includes
 * - the length of the best node path found and a list of nodes that make up the path
 * - the length of the best grid path found and a list of coordinates that this path traverses
 * - the total cost of the best path found
 *
 * @param bestPath The information found for the lowest cost path, including the nodes in the path, the cost of the path, and the positions of the cells traveled in the path
 * @param outputFilePath The path to the file where the results will be written
 */
void outputLowestCostPath(LowestCostPath bestPath, const std::string &outputFilePath)
{
    std::ofstream outputFile(outputFilePath);

    outputFile << "Lowest cost path found:" << std::endl;
    outputFile << "\t" << bestPath.nodes.size() << " nodes:";
    for (int node : bestPath.nodes)
    {
        outputFile << " " << node;
    }
    outputFile << std::endl;
    outputFile << "\t" << bestPath.path.size() << " grid points {row, col}:" << std::endl;
    outputFile << "\t\t";
    for (size_t i = 0; i < bestPath.path.size(); i++)
    {
        outputFile << "{" << bestPath.path[i].first << ", " << bestPath.path[i].second << "}";
        if (i != bestPath.path.size() - 1)
        {
            outputFile << ", ";
        }
    }
    outputFile << std::endl;
    outputFile << "\tTotal cost: " << bestPath.cost << std::endl;
    outputFile.close();
}

/**
 * Removes all files in the scrap folder
 *
 * @param scrapFolderPath The path to the folder where scrap files are stored
 */
void removeScrapFiles(const std::string &scrapFolderPath)
{
    try
    {
        for (const auto &entry : std::filesystem::directory_iterator(scrapFolderPath))
        {
            std::filesystem::remove_all(entry.path());
        }
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        std::cerr << "Error removing scrap files: " << e.what() << std::endl;
    }
}

#ifdef DEBUG
/**
 * Test the graph by writing the nodes and adjacency list to output files.
 *
 * @param graph The graph to test
 */
void testGraph(Graph &graph)
{
    // Write the nodes and adjacency list to output files
    std::ofstream outFileNodes("nodes.txt");

    if (outFileNodes.is_open())
    {
        outFileNodes << graph.printNodes();
        outFileNodes.close();
    }

    std::ofstream outFileAdjList("edges.txt");

    if (outFileAdjList.is_open())
    {
        outFileAdjList << graph.printAdjList();
        outFileAdjList.close();
    }
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
    graph.findValidPath(path, allPaths, start, dest, 0, graph.getNumNodes());

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

#endif
