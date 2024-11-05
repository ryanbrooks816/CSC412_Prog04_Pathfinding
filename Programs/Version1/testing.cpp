#include "testing.h"

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
#endif
