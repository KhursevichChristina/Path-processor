#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

// Вспомогательная функция для логирования
void log_result(const char* task, const char* message) {
    printf("[%s] %s\n", task, message);
}


int main() {
    printf("Starting testing\n");
    
    log_result("createGraph", "Starting tests");

    Graph *graph = createGraph();
    assert(graph != NULL);
    assert(graph->edges_count == 0);
    assert(graph->node_count == 0);
    assert(graph->indexes != NULL);
    assert(graph->nodes != NULL);
    freeGraph(graph);

    log_result("createGraph", "Tests passed");

    log_result("createNode", "Starting tests");

    GraphNode *node = createNode(49.895094, 30.331422, 111);
    assert(node != NULL);
    assert(node->node_id == 111);
    assert(node->lon == 30.331422);
    assert(node->lat == 49.895094);
    assert(node->edges != NULL);
    assert(node->edges->size == 0);
    freeNode(node);

    log_result("createNode", "Tests passed"); 

    log_result("addNode, getNode", "Starting tests");

    graph = createGraph();
    node = createNode(49.895094, 30.331422, 111);
    assert(addNode(graph, node) == 0);
    assert(graph->node_count == 1);

    GraphNode *node2 = createNode(47.895094, 31.331422, 112);
    assert(addNode(graph, node2) == 0);
    assert(graph->node_count == 2);

    GraphNode *nodeFound = getNode(graph, 111);
    assert(nodeFound != NULL);
    assert(nodeFound->node_id == 111);
    assert((nodeFound->lat - 49.895094) < 1e-6);
    assert((nodeFound->lon - 30.331422) < 1e-6);

    nodeFound = getNode(graph, 112);
    assert(nodeFound != NULL);
    assert(nodeFound->node_id == 112);
    assert((nodeFound->lat - 47.895094) < 1e-6);
    assert((nodeFound->lon - 31.331422) < 1e-6);

    nodeFound = getNode(graph, 113);
    assert(nodeFound == NULL);

    GraphNode *node2Dup = createNode(47.895094, 31.331422, 112);
    assert(addNode(graph, node2Dup) != 0);
    assert(graph->node_count == 2);
    freeNode(node2Dup);

    assert(addNode(NULL, node2) != 0);
    assert(addNode(graph, NULL) != 0);

    log_result("addNode, getNode", "Tests passed");

    log_result("findNearestNode", "Starting tests");

    long long foundID = findNearestNode(graph, 47.895094, 31.331422);
    assert(foundID == 112);

    foundID = findNearestNode(graph, 49.895094, 30.331422);
    assert(foundID == 111);

    foundID = findNearestNode(graph, 48.895094, 31.331422);
    assert(foundID == 112);

    foundID = findNearestNode(NULL, 48.895094, 31.331422);
    assert(foundID == -1);

    freeGraph(graph);

    log_result("findNearestNode", "Tests passed");

    log_result("addEdge", "Starting tests");

    graph = createGraph();
    addNode(graph, createNode(10, 10, 1));
    addNode(graph, createNode(10, 20, 2));
    addNode(graph, createNode(11, 12, 3));
    addNode(graph, createNode(100, 100, 4));
    addNode(graph, createNode(15, 15, 5));
    addNode(graph, createNode(20, 20, 6));
    assert(addEdge(graph, 1, 2, 10, 0, "one_two") == 0);
    assert(addEdge(graph, 1, 4, 150, 1, "one_four") == 0);
    assert(addEdge(graph, 2, 3, 5, 0, "two_three") == 0);
    assert(addEdge(graph, 4, 1, 140, 1, "four_one") == 0);
    assert(addEdge(graph, 4, 5, 80, 0, "four_five") == 0);
    assert(addEdge(graph, 1, 6, 30, 1, "one_six") == 0);

    assert(addEdge(NULL, 4, 5, 88, 0, "four_five") != 0);
    assert(addEdge(graph, 3, 4, 20, 0, NULL) == 0);
    assert(addEdge(graph, 4, 555, 88, 0, "four_five") != 0);
    assert(addEdge(graph, 44, 5, 88, 0, "four_five") != 0);
    assert(addEdge(graph, 4, 5, 88, 0, "four_five") == 0);

    log_result("addEdge", "Tests passed");

    log_result("dijkstra", "Starting tests");

    Vector *path = dijkstra(graph, 1, 5);
    assert(path != NULL);
    assert(path->size == 5);
    assert(*(long long *)getVectorItem(path, 0) == 1);
    assert(*(long long *)getVectorItem(path, 1) == 2);
    assert(*(long long *)getVectorItem(path, 2) == 3);
    assert(*(long long *)getVectorItem(path, 3) == 4);
    assert(*(long long *)getVectorItem(path, 4) == 5);

    path = dijkstra(graph, 2, 4);
    assert(path != NULL);
    assert(path->size == 3);
    assert(*(long long *)getVectorItem(path, 0) == 2);
    assert(*(long long *)getVectorItem(path, 1) == 3);
    assert(*(long long *)getVectorItem(path, 2) == 4);

    path = dijkstra(graph, 6, 1);
    assert(path != NULL);
    assert(path->size == 0);

    path = dijkstra(NULL, 6, 1);
    assert(path == NULL);

    path = dijkstra(graph, 7, 1);
    assert(path == NULL);

    path = dijkstra(graph, 6, 7);
    assert(path == NULL);

    path = dijkstra(graph, 2, 2);
    assert(path != NULL);
    assert(path->size == 1);
    assert(*(long long *)getVectorItem(path, 0) == 2);

    freeGraph(graph);
    
    log_result("dijkstra", "Tests passed");
    
    printf("All tests passed\n");
}
