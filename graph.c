#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "graph.h"
#include "lab3/comparators.h"
#include "lab3/list/generic.h"
#include "lab4/hash_table/generic.h"

Graph *createGraph() {
    Graph *graph = (Graph *)malloc(sizeof(Graph));
    if (!graph) {
        printf("Failed to allocate memory for Graph\n");
        return NULL;
    }

    graph->nodes = createVector(sizeof(GraphNode*));
    if (!graph->nodes) {
        printf("Failed to create vector for nodes\n");
        free(graph);
        return NULL;
    }

    graph->indexes = createHashTable(sizeof(long long), sizeof(int));
    if (!graph->indexes) {
        printf("Failed to create hash table for indexes\n");
        vectorFree(graph->nodes);
        free(graph);
        return NULL;
    }

    graph->edges_count = 0;
    graph->node_count = 0;

    return graph;
}

GraphNode *createNode(double lat, double lon, long long node_id) {
    GraphNode *node = (GraphNode *)malloc(sizeof(GraphNode));
    if (!node) {
        printf("Failed to allocate memory for node\n");
        return NULL;
    }

    node->edges = createVector(sizeof(Edge));
    if (!node->edges) {
        printf("Failed to create vector for edges\n");
        free(node);
        return NULL;
    }

    node->node_id = node_id;
    node->lat = lat;
    node->lon = lon;

    return node;
}

int addNode(Graph *graph, GraphNode *node) {
    if (!graph || !node) {
        printf("Invalid data for adding node in graph\n");
        return -1;
    }

    if (getNode(graph, node->node_id) != NULL) {
        printf("This node is in Graph\n");
        return -1;
    }

    if (appendVectorItem(graph->nodes, &node) != 0) {
        printf("Failed to append node in vector of nodes\n");
        return -1;
    }

    int index = graph->nodes->size - 1;
    setItemHashTable(graph->indexes, &node->node_id, &index, HashLongLong, longLongEquals);
    graph->node_count++;

    return 0;
}

GraphNode *getNode(Graph *graph, long long node_id) {
    if (!graph) {
        printf("invalid data for getting node\n");
        return NULL;
    }

    int *index = (int *)getItemHashTable(graph->indexes, &node_id, HashLongLong, longLongEquals);
    if (!index) {
        return NULL;
    }

    GraphNode **ptr = (GraphNode **)getVectorItem(graph->nodes, *index);
    if (!ptr) {
        return NULL;
    }
    return *ptr;
}

int addEdge(Graph *graph, long long from_id, long long id_to, double length, int oneway, char *name) {
    if (!graph) {
        printf("Invalid data for adding Edge\n");
        return -1;
    }

    GraphNode *node_from = getNode(graph, from_id);
    if (!node_from) {
        printf("Node from not found\n");
        return -1;
    }

    if (!oneway) {
        GraphNode *node_to = getNode(graph, id_to);
        if (!node_to) {
            printf("Node to not found\n");
            return -1;
        }
    }

    Edge edge;
    edge.id_to = id_to;
    edge.length = length;
    edge.name = name ? strdup(name) : NULL;
    edge.oneway = oneway;

    if (appendVectorItem(node_from->edges, &edge) != 0) {
        printf("Failed to add edge to node\n");
        if (edge.name) {
            free(edge.name);
        }
        return -1;
    }

    graph->edges_count++;

    if (!oneway) {
        GraphNode *node_to = getNode(graph, id_to);

        Edge reverse_edge;
        reverse_edge.id_to = from_id;
        reverse_edge.length = length;
        reverse_edge.name = name ? strdup(name) : NULL;
        reverse_edge.oneway = oneway;

        if (appendVectorItem(node_to->edges, &reverse_edge) != 0) {
            printf("Failed to add reverse edge to node\n");
            if (reverse_edge.name) {
                free(reverse_edge.name);
            }
            popVectorItem(node_from->edges, node_from->edges->size-1);
            graph->edges_count--;
            return -1;
        }

        graph->edges_count++;
    }

    return 0;
}

void freeNode(GraphNode *node) {
    if (node) {
        if (node->edges) {
            for (size_t i = 0; i < node->edges->size; i++) {
                Edge *edge = (Edge *)getVectorItem(node->edges, i);
                if (edge && edge->name) {
                    free(edge->name);
                }
            }
            vectorFree(node->edges);
        }
        free(node);
    }
}

void freeGraph(Graph *graph) {
    if (graph) {
        if (graph->nodes) {
            for (size_t i = 0; i < graph->nodes->size; i++) {
                GraphNode **nodePtr = (GraphNode **)getVectorItem(graph->nodes, i);
                if (nodePtr && *nodePtr){
                    freeNode(*nodePtr);
                }
            }
            vectorFree(graph->nodes);
        }

        if (graph->indexes) {
            freeHashTable(graph->indexes);
        }
        free(graph);
    }
}

int loadGraph(Graph *graph, const char *nodes_file, const char *edges_file) {
    if (!graph || !nodes_file || !edges_file) {
        printf("Invalid data for loading graph\n");
        return -1;
    }

    FILE *file = fopen(nodes_file, "r");
    if (!file) {
        printf("Failed to open file with nodes\n");
        return -1;
    }

    char line[1024];
    fgets(line, sizeof(line), file);
    
    while (fgets(line, sizeof(line), file)) {
        long long id;
        double lat;
        double lon;
        
        if (sscanf(line, "%lld,%lf,%lf", &id, &lat, &lon) == 3) {
            GraphNode *node = createNode(lat, lon, id);
            if (node) {
                if (addNode(graph, node) != 0) {
                    freeNode(node);
                }
            }
        }
    }
    fclose(file);
   
    file = fopen(edges_file, "r");
    if (!file) {
        printf("Failed to open file with edges\n");
        return -1;
    }

    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file)) {
        long long from;
        long long to;
        double length;
        int oneway;
        char name[256] = "";
 
        char *token = strtok(line, ",");
        if (!token) {
            continue;
        }
        from = atoll(token);
       
        token = strtok(NULL, ",");
        if (!token) {
            continue;
        }
        to = atoll(token);
        
        token = strtok(NULL, ",");
        if (!token) {
            continue;
        }
        length = atof(token);
       
        token = strtok(NULL, ",");
        if (!token) {
            continue;
        }
        
        if (strcmp(token, "True") == 0) {
            oneway = 1;
        } else if (strcmp(token, "False") == 0) {
            oneway = 0;
        }
        
        token = strtok(NULL, ",\n");
        if (token) {
            strncpy(name, token, sizeof(name) - 1);
            name[sizeof(name) - 1] = '\0';
            size_t len = strlen(name);
            if (len >= 2 && name[0] == '"' && name[len - 1] == '"') {
                memmove(name, name + 1, len - 2);
                name[len - 2] = '\0';
            }
        }
     
        if (addEdge(graph, from, to, length, oneway, name[0] ? name : NULL) != 0) {
            printf("Failed to add edge\n");
        }
    }
    fclose(file);

    return 0;
}

MinHeap *createMinHeap(int capacity)
{
    if (capacity <= 0) {
        printf("Invalid data for creating MinHeap\n");
        return NULL;
    }

    MinHeap *heap = (MinHeap *)malloc(sizeof(MinHeap));
    if (!heap) {
        printf("Failed to allocate memory for MinHeap\n");
        return NULL;
    }

    heap->capacity = capacity;
    heap->size = 0;
    heap->nodes = (HeapNode **)malloc(sizeof(HeapNode *) * capacity);
    if (!heap->nodes) {
        printf("Failed to allocate memory for heap nodes\n");
        free(heap);
        return NULL;
    }

    return heap;
}

static void swapNodes(HeapNode **a, HeapNode **b)
{
    if (*a != *b) {
        *a = (HeapNode *)((uintptr_t)*a ^ (uintptr_t)*b);
        *b = (HeapNode *)((uintptr_t)*b ^ (uintptr_t)*a);
        *a = (HeapNode *)((uintptr_t)*a ^ (uintptr_t)*b);
    }   
}

static void heapifyUp(MinHeap *heap, int idx)
{
    if (!heap || idx < 0) {
        printf("Invalid data for heapifyUp\n");
        return;
    }

    while (idx > 0) {
        int parent = (idx - 1) / 2;

        if (heap->nodes[parent]->distance > heap->nodes[idx]->distance) {
            swapNodes(&heap->nodes[parent], &heap->nodes[idx]);
        }
        else {
            break;
        }

        idx = parent;
    }
}

static void heapifyDown(MinHeap *heap, int idx)
{
    if (!heap || idx < 0) {
        printf("Invalid data for heapifyDown\n");
        return;
    }

    int current = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < heap->size && heap->nodes[left]->distance < heap->nodes[current]->distance) {
        current = left;
    }

    if (right < heap->size && heap->nodes[right]->distance < heap->nodes[current]->distance) {
        current = right;
    }

    if (current != idx) {
        swapNodes(&heap->nodes[current], &heap->nodes[idx]);
        heapifyDown(heap, current);
    }
}

void insertMinHeap(MinHeap *heap, HeapNode *node)
{
    if (!heap || !node || heap->capacity <= heap->size) {
        printf("Invalid data for inserting\n");
        return;
    }

    heap->nodes[heap->size] = node;
    heapifyUp(heap, heap->size);
    heap->size++;
}

HeapNode *extractMin(MinHeap *heap)
{
    if (!heap || heap->size <= 0) {
        printf("Invalid data for extracting Min\n");
        return NULL;
    }

    HeapNode *node = heap->nodes[0];
    heap->size--;

    if (heap->size > 0) {
        heap->nodes[0] = heap->nodes[heap->size];
        heapifyDown(heap, 0);
    }

    return node;
}

void freeMinHeap(MinHeap *heap)
{
    if (heap) {
        if (heap->nodes) {
            for (int i = 0; i < heap->size; i++) {
                if (heap->nodes[i]) {
                    free(heap->nodes[i]);
                }
            }
            free(heap->nodes);
        }
        free(heap);
    }
}

long long findNearestNode(Graph *graph, double lat, double lon) {
    if (!graph || !graph->nodes || graph->nodes->size == 0) {
        printf("Invalid data for finding nearest node\n");
        return -1;
    }

    double minDist = DBL_MAX;
    long long nearestID = -1;
    
    for (size_t i = 0; i < graph->nodes->size; i++) {
        GraphNode **nodePtr = (GraphNode **)getVectorItem(graph->nodes, i);
        if (*nodePtr) {
            GraphNode *node = *nodePtr;
            double xDist = node->lat - lat;
            double yDist = node->lon - lon;
            double dist = xDist * xDist + yDist * yDist;

            if (dist < minDist) {
                minDist = dist;
                nearestID = node->node_id;
            }
        }
    }

    return nearestID;
}

Vector *dijkstra(Graph *graph, long long startID, long long finishID) {
    if (!graph) {
        printf("Invalid data for dijkstra\n");
        return NULL;
    }

    if (startID == finishID) {
        Vector *path = createVector(sizeof(long long));
        if (path) {
            appendVectorItem(path, &startID);
        }
        return path;
    }

    int *startIndex = (int *)getItemHashTable(graph->indexes, &startID, HashLongLong, longLongEquals);
    int *finishIndex = (int *)getItemHashTable(graph->indexes, &finishID, HashLongLong, longLongEquals);

    if (!startIndex || !finishIndex) {
        printf("Failed to find start or finish index\n");
        return NULL;
    }

    int startIdx = *startIndex;
    int finishIdx = *finishIndex;

    double *dist = (double *)malloc(sizeof(double) * graph->node_count);
    if (!dist) {
        printf("Failed to allocate memory for dist\n");
        return NULL;
    }

    int *prev = (int *)malloc(sizeof(int) * graph->node_count);
    if (!prev) {
        printf("Failed to allocate memory for prev\n");
        free(dist);
        return NULL;
    }

    int *visited = (int *)calloc(graph->node_count, sizeof(int));
    if (!visited) {
        printf("Failed to allocate memory for visited\n");
        free(dist);
        free(prev);
        return NULL;
    }

    for (int i = 0; i < graph->node_count; i++) {
        dist[i] = DBL_MAX;
        prev[i] = -1;
    }
    dist[startIdx] = 0;

    MinHeap *heap = createMinHeap(graph->node_count);
    if (!heap) {
        free(dist);
        free(prev);
        free(visited);
        return NULL;
    }

    HeapNode *startNode = (HeapNode *)malloc(sizeof(HeapNode));
    if (!startNode) {
        printf("failed to allocate memory for start node\n");
        free(dist);
        free(prev);
        free(visited);
        freeMinHeap(heap);
        return NULL;
    }
    startNode->distance = 0.0;
    startNode->node_id = startID;
    insertMinHeap(heap, startNode);

    while (heap->size > 0) {
        HeapNode *current = extractMin(heap);
        
        int *currentIndex = (int *)getItemHashTable(graph->indexes, &current->node_id, HashLongLong, longLongEquals);
        if (!currentIndex) {
            free(current);
            continue;
        }
        int currentIdx = *currentIndex;

        if (visited[currentIdx]) {
            free(current);
            continue;
        }

        visited[currentIdx] = 1;

        if (current->node_id == finishID) {
            free(current);
            break;
        }

        GraphNode *currentNode = getNode(graph, current->node_id);
        if (!currentNode) {
            free(current);
            continue;
        }

        for (size_t i = 0; i < currentNode->edges->size; i++) {
            Edge *edge = (Edge *)getVectorItem(currentNode->edges, i);
            if (!edge) {
                continue;
            }

            int *nextIndex = (int *)getItemHashTable(graph->indexes, &edge->id_to, HashLongLong, longLongEquals);
            if (!nextIndex) {
                continue;
            }
            int nextIdx = *nextIndex;

            if (visited[nextIdx]) {
                continue;
            }

            double nowDist = dist[currentIdx] + edge->length;
            if (nowDist < dist[nextIdx]) {
                dist[nextIdx] = nowDist;
                prev[nextIdx] = currentIdx;

                HeapNode *nextNode = (HeapNode *)malloc(sizeof(HeapNode));
                if (!nextNode) {
                    printf("Failed to allocate memory for next node\n");
                    continue;
                }

                nextNode->distance = nowDist;
                nextNode->node_id = edge->id_to;
                insertMinHeap(heap, nextNode);
            }
        }
        
        free(current);
    }

    Vector *path = createVector(sizeof(long long));
    if (!path) {
        printf("Failed to allocate memory for path");
        freeMinHeap(heap);
        free(dist);
        free(prev);
        free(visited);
        return NULL;
    }

    if (dist[finishIdx] == DBL_MAX) {
        freeMinHeap(heap);
        free(dist);
        free(prev);
        free(visited);
        return path;
    }

    long long *reversedPath = (long long *)malloc(sizeof(long long) * graph->node_count);
    if (!reversedPath) {
        printf("Failed to allocate memory for path");
        freeMinHeap(heap);
        free(dist);
        free(prev);
        free(visited);
        vectorFree(path);
        return NULL;
    }

    int pathLen = 0;
    int currentIndex = finishIdx;

    while (currentIndex != -1) {
        GraphNode **nodePtr = (GraphNode **)getVectorItem(graph->nodes, currentIndex);
        if (!nodePtr || !*nodePtr) {
            break;
        }

        GraphNode *node = *nodePtr;
        reversedPath[pathLen] = node->node_id;
        pathLen++;
        currentIndex = prev[currentIndex];
    }

    for (int i = pathLen - 1; i >= 0; i--) {
        appendVectorItem(path, &reversedPath[i]);
    }

    freeMinHeap(heap);
    free(dist);
    free(prev);
    free(visited);
    free(reversedPath);

    return path;
}
