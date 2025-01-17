/* 
 * CS5007
 * A5: Header File
 * Author: Evan Douglass
 * Created: Feb. 17th 2019
 */

/*
 * Constants
 */
#define NUM_NODES 1000
#define MAX_TITLE_LEN 50

/*
 * Bit Manipulation Prototypes
 * ===========================
 */

// Prints the given value's binary representation.
// Given by instructors, but modified to account for packed ints.
void displayBits(unsigned int value, int printOptions);

// Packs two characters into a single 2 byte unsigned variable.
unsigned int packCharactersHelper(char c1, char c2);

// Wrapper for packCharactersHelper.
// Prints the output of the helper in the manner specified for the assignment.
void packCharacters(char c1, char c2);

// Unpacks two characters from a 2 byte unsigned integer that has been
// previously packed. Writes the results into a given char array.
void unpackCharactersHelper(unsigned int packed, int* resultArr);

// Wrapper for unpackCharactersHelper.
void unpackCharacters(unsigned int packed);

// Uses bit manipulation to calculate a number to a given power
// of two (i.e. x * 2^y).
unsigned int power2Helper(unsigned int number, int pow);

// Wrapper for power2Helper.
void power2(unsigned int number, int pow);


/*
 * Adjacency Matrix Prototypes
 * ===========================
 */

struct adjGraph {
    // An array of strings denoting the names of nodes
    char (*nodes)[MAX_TITLE_LEN];
    // A 2D array of floats containing the edges
    float (*adjMatrix)[NUM_NODES];

    // Dijkstra Related Fields
    // Updated when dijkstra's is called
    int mostRecentSource;
    // Used in dijkstra's, saved for continue reference
    float shortest[NUM_NODES];
    // Used in dijkstra's, saved for continue reference
    int pred[NUM_NODES];
    // 1 == visited, 2 == in queue, 0 == unvisited, not in queue
    int visited[NUM_NODES];
};
typedef struct adjGraph AdjGraph;

// Builds the graph from the given file.
AdjGraph* buildAdjGraphFromFile(char* file_path);

// Allocates memory for the graph
AdjGraph* buildAdjGraph();

// Frees memory associated with the graph
void freeAdjGraph(AdjGraph* graph);

// Reads a csv file containing an adjacency matrix, parsing the nodes into a
// char* array and the values into a 2D int array
void parseMatrixInto(AdjGraph* graph, FILE* fPtr);

// Splits a given string into an array of strings and
// attaches it to graph->nodes
void splitToStrings(AdjGraph* graph, char* string, char* delimiter);

// Splits a given string into an array of ints and
// attaches it to graph->adjMatrix[index]
void splitToFloats(AdjGraph* graph, char* string,
                    int rowIndex, char* delimiter);

// ===== Dijkstra Algorithm Functions =====

// Finds the integer index of a node
int findNodeIndex(AdjGraph* graph, char* nodeName);

// Performs Dijkstra's algorithm to find the shortest path
void dijkstra(AdjGraph* graph, char* source);

// Relaxes all neighbor nodes
void relaxNeighbors(AdjGraph* graph, int node);

// Updates the shortest path between to nodes
void relax(AdjGraph* graph, int nodeIndex1, int nodeIndex2);

// Gets the weight of an edge between two nodes
float getWeight(AdjGraph* graph, int nodeIndex1, int nodeIndex2);

// Finds the index of the node with the current shortest path
int getShortest(AdjGraph* graph);

// Tests for an empty set (visited)
// Needs work when I have more time
int isEmpty(int* array);

// Display the shortest distance from the most recent source to a given node
void displayShortestDistance(AdjGraph* graph, int nodeIndex);

// Recursively prints the shortest path from the latest source to the given node
void displayPath(AdjGraph* graph, int nodeIndex);
