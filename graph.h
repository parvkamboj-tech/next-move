/*
 * graph.h  —  Siddhant (C)
 * Warehouse map: weighted undirected graph (adjacency matrix)
 * BFS, DFS, Dijkstra shortest path, blocked paths (dynamic rerouting)
 *
 * Locations are numbered from 1 (like the original code).
 * Location 1 is treated as the Dispatch Dock by the integrated system.
 */
#ifndef GRAPH_H
#define GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LOCATIONS 20
#define LOC_NAME_LEN 30

void graph_init(void);

/* returns new location number, or 0 if MAX_LOCATIONS reached */
int graph_add_location(const char *name);

/* returns 1 = ok, 0 = invalid */
int graph_add_path(int start, int end, int distance);
int graph_remove_path(int start, int end);

/* returns 1 = now blocked, 0 = now open, -1 = no such path */
int graph_toggle_block(int start, int end);

void graph_display(void);
void graph_bfs(int start);
void graph_dfs(int start);

/* Dijkstra. Fills path[] (location numbers) and *pathLength.
   Returns total distance, or -1 if no open route exists. */
int  graph_shortest_path(int start, int end, int path[], int *pathLength);
void graph_print_shortest_path(int start, int end);

int graph_location_count(void);
int graph_is_valid(int location);
const char *graph_location_name(int location);

#ifdef __cplusplus
}
#endif

#endif
