/*
 * CS5007
 * A5: Adjacency Matrix
 * Author: Evan Douglass
 * Created: Feb. 20 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "a5.h"

// Builds a graph from the given file
AdjGraph* buildAdjGraphFromFile(char* file_path) {
    AdjGraph* graph;
    FILE* fPtr = fopen(file_path, "r");

    // Test if file exists
    if (fPtr == NULL) {
        printf("File not found\n");
    } else {
        graph = buildAdjGraph();
        parseMatrixInto(graph, fPtr);
        fclose(fPtr);
    }
    return graph;
}

// Allocates memory for a graph
AdjGraph* buildAdjGraph() {
    AdjGraph* graph;
    char (*nodes)[MAX_TITLE_LEN];
    float (*adjMatrix)[NUM_NODES];

    // Allocate struct graph memory
    graph = (AdjGraph*)malloc(sizeof(AdjGraph));
    assert(graph);

    // Allocate graph representation memory
    nodes = malloc(sizeof(*nodes) * NUM_NODES);
    adjMatrix = malloc(sizeof(*adjMatrix) * NUM_NODES);
    assert(nodes);
    assert(adjMatrix);

    // Attach arrays to graph
    graph->nodes = nodes;
    graph->adjMatrix = adjMatrix;

    return graph;
}

// Deallocates the graph
void freeAdjGraph(AdjGraph* graph) {
    free(graph->nodes);
    free(graph->adjMatrix);
    free(graph);
}

// Reads a file contianing an adjacency matrix and parses it into an AdjGraph
// Assumes the file exists and is open, therefore does not close it either.
void parseMatrixInto(AdjGraph* graph, FILE* fPtr) {
    int len = MAX_TITLE_LEN * NUM_NODES;

    // Linter said not to use len here
    char line[MAX_TITLE_LEN * NUM_NODES];

    // Get first line for node names.
    if (!feof(fPtr)) {
        fgets(line, len, fPtr);
        splitToStrings(graph, line, ",");
    }

    // Get rest of matrix for adjMatrix
    int i = 0;
    while (!feof(fPtr)) {
        strcpy(line, "");
        fgets(line, len, fPtr);
        splitToFloats(graph, line, i, ",");
        i++;
    }
}

// Splits a line of text in an array of strings
void splitToStrings(AdjGraph* graph, char* string, char* delimiter) {
    char* title;
    int lastChar = strlen(string) - 1;

    if (string[lastChar] == '\n') {
        string[lastChar] = '\0';
    }

    // Discard first column, nothing there.
    strsep(&string, delimiter);

    // Parse remaining title strings
    int i = 0;
    while ((title = strsep(&string, delimiter)) != NULL) {
        strcpy(graph->nodes[i], title);
        i++;
    }
}

// Splits a line of text into an integer array
void splitToFloats(AdjGraph* graph, char* string,
                    int rowIndex, char* delimiter) {
    char* temp;
    int lastChar = strlen(string) - 1;

    if (string[lastChar] == '\n') {
        string[lastChar] = '\0';
    }

    // Discard first column, node title
    strsep(&string, delimiter);

    // Parse remaining integers, making empty strings -1.
    int i = 0;
    while ((temp = strsep(&string, delimiter)) != NULL) {
        if (strcmp(temp, "") == 0) {
            graph->adjMatrix[rowIndex][i++] = -1.0;
        } else {
            graph->adjMatrix[rowIndex][i++] = atof(temp);
        }
    }
}

// ===== Dijkstra Algorithm Functions =====

// Finds the index of a given node.
// A better implementation would be binary search, but in the
// interest of time, this is linear search.
// Returns -1 on failure.
int findNodeIndex(AdjGraph* graph, char* nodeName) {
    int index = -1;
    for (int i = 0; i < NUM_NODES; i++) {
        if (strcmp(nodeName, graph->nodes[i]) == 0) {
            index = i;
        }
    }
    return index;
}

// Dijkstra's Algorithm for shortest paths.
// Prints the value of the shortest path as well as the nodes it includes.
void dijkstra(AdjGraph* graph, char* source) {
    int sourceI = findNodeIndex(graph, source);

    // Initialize Dijkstra related fields in graph
    graph->mostRecentSource = sourceI;
    for (int i = 0; i < NUM_NODES; i++) {
        graph->shortest[i] = INFINITY;
        graph->pred[i] = -1;
        graph->visited[i] = 0;
    }
    graph->shortest[sourceI] = 0.0;
    graph->visited[sourceI] = 2;

    // The actual algorithm
    int* set = graph->visited;
    while (!isEmpty(set)) {
        // puts("start of while");
        int node = getShortest(graph);
        // printf("node = %d\n", node);
        // printf("shortest[node] = %f\n", graph->shortest[node]);

        // Test dead end
        if (node > -1) {
            set[node] = 1;  // was visited
            relaxNeighbors(graph, node);
        //   puts("End of while\n");
        } else {
            break;
        }
    }
}

int getShortest(AdjGraph* graph) {
    float temp = INFINITY;
    int index = -1;
    for (int i = 0; i < NUM_NODES; i++) {
        if (graph->visited[i] == 2 && graph->shortest[i] < temp) {
            temp = graph->shortest[i];
            index = i;
        }
    }
    return index;
}

// Tests for an empty set (visited)
// Needs work when I have more time
int isEmpty(int* array) {
    for (int i = 0; i < NUM_NODES; i++) {
        if (array[i] == 0) {
            return 0;
        }
    }
    return 1;
}

// Relax all neighbor nodes
void relaxNeighbors(AdjGraph* graph, int index) {
    // Search row=index to find neighbors and call relax
    for (int j = 0; j < NUM_NODES; j++) {
        if (graph->adjMatrix[index][j] > 0 && graph->visited[j] != 1) {
            relax(graph, index, j);
            graph->visited[j] = 2;
        }
    }
}

// Adjusts the shortest path between the given nodes, if needed.
void relax(AdjGraph* graph, int nodeIndex1, int nodeIndex2) {
    float* s = graph->shortest;
    int* p = graph->pred;

    float testWeight = s[nodeIndex1] + getWeight(graph, nodeIndex1, nodeIndex2);
    if (testWeight < s[nodeIndex2]) {
        s[nodeIndex2] = testWeight;
        p[nodeIndex2] = nodeIndex1;
    }
}

// Determines the weight between two given nodes
float getWeight(AdjGraph* graph, int nodeIndex1, int nodeIndex2) {
    return graph->adjMatrix[nodeIndex1][nodeIndex2];
}

// Display the shortest distance from the most recent source to a given node
void displayShortestDistance(AdjGraph* graph, int nodeIndex) {
    printf("Shortest distance from %s to %s: %.2f\n",
            graph->nodes[graph->mostRecentSource],
            graph->nodes[nodeIndex],
            graph->shortest[nodeIndex]);
}

// Display the path to get from the most recent source to a given node
void displayPath(AdjGraph* graph, int nodeIndex) {
    // Only print one node if the source is the sink
    if (graph->pred[nodeIndex] == -1) {
        printf("%s", graph->nodes[nodeIndex]);

    // Recursively print the path nodes
    } else {
        displayPath(graph, graph->pred[nodeIndex]);
        printf("-> %s", graph->nodes[nodeIndex]);
    }
    printf("\n");
}


int main() {
    AdjGraph* g = buildAdjGraphFromFile(
                   "./a5_data_files/miles_graph_FINAL.csv");

    // Seattle to Boston
    dijkstra(g, "Seattle_WA");
    int bosIndex = findNodeIndex(g, "Boston_MA");
    displayShortestDistance(g, bosIndex);
    printf("The path:\n");
    displayPath(g, bosIndex);
    printf("\n");

    // Minneapolis to Ann Arbor
    dijkstra(g, "Minneapolis_MN");
    int annArborIndex = findNodeIndex(g, "Ann Arbor_MI");
    displayShortestDistance(g, annArborIndex);
    printf("The path:\n");
    displayPath(g, annArborIndex);
    printf("\n");

    freeAdjGraph(g);

    return 0;
}
