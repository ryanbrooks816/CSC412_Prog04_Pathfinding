#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <filesystem>
#include "pathfinder.h"

int main(int argc, char **argv)
{
    // Validate CLAs
    const std::string usage = "Usage: " + std::string(argv[0]) + " <gridPath> <nodesPath> <node1> <node2> <scrapFolderPath> <outputFilePath>";
    if (argc > 7)
    {
        std::cout << "Too many arguments. " << usage << std::endl;
        return 51;
    }
    else if (argc < 7)
    {
        std::cout << "Too few arguments. " << usage << std::endl;
        return 52;
    }

    std::string gridPath = argv[1];
    std::string nodesPath = argv[2];

    // See if the grid path is a file
    std::ifstream gridFile(gridPath);
    if (!gridFile.is_open())
    {
        std::cout << "Grid file at specified path does not exist" << std::endl;
        return 40;
    }

    // See if the nodes path is a file
    std::ifstream nodesFile(nodesPath);
    if (!nodesFile.is_open())
    {
        std::cout << "Nodes file at specified path does not exist" << std::endl;
        return 41;
    }

    int node1 = std::stoi(argv[3]);
    int node2 = std::stoi(argv[4]);

    std::string scrapFolderPath = argv[5];
    std::string outputFilePath = argv[6];

    // Create the scrap folder if it does not exist
    if (!std::filesystem::exists(scrapFolderPath))
    {
        std::filesystem::create_directories(scrapFolderPath);
    }
    
    // Construct the cost grid
    std::vector<std::vector<float>> grid = createCostGrid(gridPath);

    // Construct the graph
    Graph graph = Graph(nodesPath);

    if (!overlayGraph(graph, grid))
    {
        return 90;
    }

    // Construct an adjacency list to represent the graph's edges
    graph.findClosestNodes();

#ifdef DEBUG
    testGraph(graph);
#endif

    // Find the cheapest path between given all the possible paths in the Graph's adjacency list
    // outputs results to scrap folder
    findCheapestPath(graph, node1, node2, scrapFolderPath);
    outputLowestCostPath(outputFilePath);
}
