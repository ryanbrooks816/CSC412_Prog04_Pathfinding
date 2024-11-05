#ifndef TESTING_H
#define TESTING_H

/**
 * Define the DEBUG macro to enable debug output. Utilize the DEBUG_CONSOLE and DEBUG_FILE macros
 * to output debug messages to the console and a file, respectively.
 *
 * DEBUG_CONSOLE(Out): Prints the debug message `Out` to the console if debugging is enabled.
 * DEBUG_FILE(Out, filePath): Writes the debug message `Out` to the specified file if debugging is enabled.
 */
#ifdef DEBUG
#include <iostream>
#include <fstream>
#include "graph.h"

// DEBUG_FILE with optional newline parameter (default is true)
#define DEBUG_FILE_DEF(message, filepath, newline)                         \
    {                                                                      \
        std::ofstream file(filepath, std::ios_base::app);                  \
        if (file.is_open())                                                \
        {                                                                  \
            file << message;                                               \
            if (newline)                                                   \
                file << std::endl;                                         \
            file.close();                                                  \
        }                                                                  \
        else                                                               \
        {                                                                  \
            std::cerr << "Unable to open file: " << filepath << std::endl; \
        }                                                                  \
    }
#define DEBUG_FILE_ENDL(message, filepath) DEBUG_FILE_DEF(message, filepath, true)
#define GET_DEBUG_FILE_MACRO(_1, _2, _3, NAME, ...) NAME
#define DEBUG_FILE(...) GET_DEBUG_FILE_MACRO(__VA_ARGS__, DEBUG_FILE_DEF, DEBUG_FILE_ENDL)(__VA_ARGS__)

// DEBUG_CONSOLE with optional newline parameter (default is true)
#define DEBUG_CONSOLE_DEF(message, newline) \
    {                                       \
        std::cout << message;               \
        if (newline)                        \
            std::cout << std::endl;         \
    }
#define DEBUG_CONSOLE_ENDL(message) DEBUG_CONSOLE_DEF(message, true)
#define GET_DEBUG_CONSOLE_MACRO(_1, _2, NAME, ...) NAME
#define DEBUG_CONSOLE(...) GET_DEBUG_CONSOLE_MACRO(__VA_ARGS__, DEBUG_CONSOLE_DEF, DEBUG_CONSOLE_ENDL)(__VA_ARGS__)

/**
 * Test the graph by writing the nodes and adjacency list to output files.
 *
 * @param graph The graph to test
 */
void testGraph(Graph &graph);

#else

#define DEBUG_CONSOLE(...)
#define DEBUG_FILE(...)
#define testGraph(graph)

#endif // DEBUG

#endif // TESTING_H
