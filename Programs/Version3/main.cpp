#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <filesystem>
#include "pathfinder.h"
#include "testing.h"

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

    int startingNode = std::stoi(argv[3]);
    int endingNode = std::stoi(argv[4]);

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

    // Find all the possible paths given the graph's adjacency list
    std::vector<std::vector<int>> validPaths = graph.findValidPaths(startingNode, endingNode);

    // Test the graph's paths by writing them to a file and then generate all the possible paths
    // (without the min and max nodes constraint) and write them to a file
#ifdef DEBUG
    writePathsToFile(validPaths, "tree_valid.txt");
    outputAllGraphPaths(graph, startingNode, endingNode);
#endif

    // Find the cheapest path between given all the possible paths and output results to scrap folder
    LowestCostPath bestPath = findCheapestPath(graph, grid, validPaths, startingNode, scrapFolderPath);
    outputLowestCostPath(bestPath, outputFilePath);
}

#ifdef DEBUG

#endif
