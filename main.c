#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Must be: ./path_processor <path_to_data_dir> <path_to_input> <path_to_output>\n");
        return 1;
    }

    const char *data_dir = argv[1];
    const char *input_file = argv[2];
    const char *output_file = argv[3];

    char nodes_path[512];
    char edges_path[512];
    snprintf(nodes_path, sizeof(nodes_path), "%s/nodes.csv", data_dir);
    snprintf(edges_path, sizeof(edges_path), "%s/edges.csv", data_dir);

    Graph *graph = createGraph();
    if (!graph) {
        return 1;
    }

    if (loadGraph(graph, nodes_path, edges_path) != 0) {
        freeGraph(graph);
        return 1;
    }

    FILE *input = fopen(input_file, "r");
    if (!input) {
        printf("Failed to open input file\n");
        freeGraph(graph);
        return 1;
    }

    double start_lat;
    double start_lon;
    double finish_lat;
    double finish_lon;

    if (fscanf(input, "%lf %lf", &start_lat, &start_lon) != 2) {
        printf("Invalid input\n");
        fclose(input);
        freeGraph(graph);
        return 1;
    }
    
    if (fscanf(input, "%lf %lf", &finish_lat, &finish_lon) != 2) {
        printf("Invalid input\n");
        fclose(input);
        freeGraph(graph);
        return 1;
    }

    fclose(input);

    long long start_id = findNearestNode(graph, start_lat, start_lon);
    long long finish_id = findNearestNode(graph, finish_lat, finish_lon);

    if (start_id == -1 || finish_id == -1) {
        printf("failed to find hearest node\n");
        freeGraph(graph);
        return 1;
    }

    Vector *path = dijkstra(graph, start_id, finish_id);
    
    FILE *output = fopen(output_file, "wb");
    if (!output) {
        printf("Failed to open output file\n");
        freeGraph(graph);
        if (path) {
            vectorFree(path);
        }
        return 1;
    }

    if (path && path->size > 0) {
        for (size_t i = 0; i < path->size; i++) {
            long long *node_id = (long long *)getVectorItem(path, i);
            if (node_id) {
                GraphNode *node = getNode(graph, *node_id);
                if (node) {
                    fprintf(output, "%.6lf %.6lf\n", node->lat, node->lon);
                }
            }
        }
    }

    fclose(output);

    if (path) {
        vectorFree(path);
    }
    freeGraph(graph);

    return 0;
}
