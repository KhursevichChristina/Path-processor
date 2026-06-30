#ifndef GRAPH_H
#define GRAPH_H

#include "lab3/list/generic.h"
#include "lab4/hash_table/generic.h"
#include "lab3/vector/generic.h"

typedef struct GraphNode {
    long long node_id;
    double lat;
    double lon;
    Vector *edges;
} GraphNode;

typedef struct Edge {
    long long id_to;
    double length;
    int oneway;
    char *name;
} Edge;

typedef struct Graph {
    Vector *nodes;
    HashTable *indexes;
    int node_count;
    int edges_count;
} Graph;

typedef struct {
    long long node_id;
    double distance;
} HeapNode;

typedef struct {
    HeapNode **nodes;
    int size;
    int capacity;
} MinHeap;

Graph *createGraph();
void freeGraph(Graph *graph);
GraphNode *createNode(double lat, double lon, long long node_id);
int addNode(Graph *graph, GraphNode *node);
GraphNode *getNode(Graph *graph, long long node_id);
void freeNode(GraphNode *node);
GraphNode *getNode(Graph *graph, long long node_id);
int addEdge(Graph *graph, long long from_id, long long id_to, double length, int oneway, char *name);
int loadGraph(Graph *graph, const char *nodes_file, const char *edges_file);
long long findNearestNode(Graph *graph, double lat, double lon);
Vector *dijkstra(Graph *graph, long long startID, long long finishID);

#endif // GRAPH_H
