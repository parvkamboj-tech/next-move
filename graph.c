/*
 * graph.c  —  Siddhant (C)
 * Adjacency matrix graph + BFS + DFS + Dijkstra + dynamic rerouting
 *
 * Run alone:   gcc -DSTANDALONE graph.c -o siddhant_graph
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

#define INF 1000000000

static int graph[MAX_LOCATIONS][MAX_LOCATIONS];    /* 0 = no path, else distance */
static int blocked[MAX_LOCATIONS][MAX_LOCATIONS];  /* 1 = path temporarily closed */
static char names[MAX_LOCATIONS][LOC_NAME_LEN];
static int vertices = 0;

void graph_init(void)
{
    memset(graph, 0, sizeof(graph));
    memset(blocked, 0, sizeof(blocked));
    memset(names, 0, sizeof(names));
    vertices = 0;
}

int graph_location_count(void) { return vertices; }

int graph_is_valid(int location)
{
    return location >= 1 && location <= vertices;
}

const char *graph_location_name(int location)
{
    return graph_is_valid(location) ? names[location - 1] : "Unknown";
}

/* path is usable only if it exists AND is not blocked */
static int isOpen(int u, int v)
{
    return graph[u][v] != 0 && blocked[u][v] == 0;
}

/* ===================== BUILD GRAPH ===================== */
int graph_add_location(const char *name)
{
    if (vertices == MAX_LOCATIONS)
        return 0;

    if (name != NULL && name[0] != '\0')
    {
        strncpy(names[vertices], name, LOC_NAME_LEN - 1);
        names[vertices][LOC_NAME_LEN - 1] = '\0';
    }
    else
    {
        sprintf(names[vertices], "Location %d", vertices + 1);
    }

    vertices++;
    return vertices;
}

int graph_add_path(int start, int end, int distance)
{
    if (!graph_is_valid(start) || !graph_is_valid(end) || start == end || distance <= 0)
        return 0;

    graph[start - 1][end - 1] = distance;
    graph[end - 1][start - 1] = distance;
    blocked[start - 1][end - 1] = 0;
    blocked[end - 1][start - 1] = 0;
    return 1;
}

int graph_remove_path(int start, int end)
{
    if (!graph_is_valid(start) || !graph_is_valid(end) || graph[start - 1][end - 1] == 0)
        return 0;

    graph[start - 1][end - 1] = 0;
    graph[end - 1][start - 1] = 0;
    blocked[start - 1][end - 1] = 0;
    blocked[end - 1][start - 1] = 0;
    return 1;
}

int graph_toggle_block(int start, int end)
{
    int s = start - 1, e = end - 1;

    if (!graph_is_valid(start) || !graph_is_valid(end) || graph[s][e] == 0)
        return -1;

    blocked[s][e] = !blocked[s][e];
    blocked[e][s] = blocked[s][e];
    return blocked[s][e];
}

/* ===================== DISPLAY ===================== */
void graph_display(void)
{
    int i, j, any;

    if (vertices == 0)
    {
        printf("\nNo Warehouse Locations Available!\n");
        return;
    }

    printf("\n===== WAREHOUSE MAP =====\n");
    for (i = 0; i < vertices; i++)
    {
        printf("%2d. %-15s -> ", i + 1, names[i]);
        any = 0;
        for (j = 0; j < vertices; j++)
        {
            if (graph[i][j] != 0)
            {
                printf("%s(%dm%s)  ", names[j], graph[i][j], blocked[i][j] ? ", BLOCKED" : "");
                any = 1;
            }
        }
        if (!any)
            printf("no paths");
        printf("\n");
    }
}

/* ===================== BFS ===================== */
void graph_bfs(int start)
{
    int visited[MAX_LOCATIONS] = {0};
    int queue[MAX_LOCATIONS];
    int front = 0, rear = 0;
    int current, i, count = 0;

    if (!graph_is_valid(start))
    {
        printf("\nInvalid Starting Location!\n");
        return;
    }

    start--;
    visited[start] = 1;
    queue[rear++] = start;

    printf("\nBFS Traversal: ");
    while (front < rear)
    {
        current = queue[front++];
        printf("%s%s", count ? " -> " : "", names[current]);
        count++;

        for (i = 0; i < vertices; i++)
        {
            if (isOpen(current, i) && !visited[i])
            {
                visited[i] = 1;
                queue[rear++] = i;
            }
        }
    }
    printf("\nReached %d of %d locations.\n", count, vertices);
}

/* ===================== DFS ===================== */
static void dfsVisit(int current, int visited[], int *count)
{
    int i;

    visited[current] = 1;
    printf("%s%s", *count ? " -> " : "", names[current]);
    (*count)++;

    for (i = 0; i < vertices; i++)
    {
        if (isOpen(current, i) && !visited[i])
            dfsVisit(i, visited, count);
    }
}

void graph_dfs(int start)
{
    int visited[MAX_LOCATIONS] = {0};
    int count = 0;

    if (!graph_is_valid(start))
    {
        printf("\nInvalid Starting Location!\n");
        return;
    }

    printf("\nDFS Traversal: ");
    dfsVisit(start - 1, visited, &count);
    printf("\nReached %d of %d locations.\n", count, vertices);
}

/* ===================== DIJKSTRA ===================== */
int graph_shortest_path(int start, int end, int path[], int *pathLength)
{
    int dist[MAX_LOCATIONS], prev[MAX_LOCATIONS], done[MAX_LOCATIONS];
    int reversed[MAX_LOCATIONS];
    int i, k, u, v, n = 0;

    *pathLength = 0;
    if (!graph_is_valid(start) || !graph_is_valid(end))
        return -1;

    for (i = 0; i < vertices; i++)
    {
        dist[i] = INF;
        prev[i] = -1;
        done[i] = 0;
    }
    dist[start - 1] = 0;

    for (k = 0; k < vertices; k++)
    {
        /* pick unvisited location with smallest distance */
        u = -1;
        for (i = 0; i < vertices; i++)
            if (!done[i] && (u == -1 || dist[i] < dist[u]))
                u = i;

        if (u == -1 || dist[u] == INF)
            break;
        done[u] = 1;

        /* relax open neighbours */
        for (v = 0; v < vertices; v++)
        {
            if (isOpen(u, v) && dist[u] + graph[u][v] < dist[v])
            {
                dist[v] = dist[u] + graph[u][v];
                prev[v] = u;
            }
        }
    }

    if (dist[end - 1] == INF)
        return -1;

    /* rebuild path backwards, then reverse it */
    for (v = end - 1; v != -1; v = prev[v])
        reversed[n++] = v;
    for (i = 0; i < n; i++)
        path[i] = reversed[n - 1 - i] + 1;

    *pathLength = n;
    return dist[end - 1];
}

void graph_print_shortest_path(int start, int end)
{
    int path[MAX_LOCATIONS], length, distance, i;

    distance = graph_shortest_path(start, end, path, &length);
    if (distance < 0)
    {
        printf("\nNo open route from %s to %s!\n",
               graph_location_name(start), graph_location_name(end));
        return;
    }

    printf("\nShortest Route: ");
    for (i = 0; i < length; i++)
        printf("%s%s", i ? " -> " : "", graph_location_name(path[i]));
    printf("\nTotal Distance: %d m\n", distance);
}

/* ===================== STANDALONE MENU (Siddhant's demo) ===================== */
#ifdef STANDALONE
static int readInt(const char *prompt)
{
    int x, c;
    printf("%s", prompt);
    while (scanf("%d", &x) != 1)
    {
        while ((c = getchar()) != '\n' && c != EOF);
        if (c == EOF) exit(0);
        printf("Please enter a number: ");
    }
    return x;
}

int main(void)
{
    int choice, a, b, d, r;
    char name[LOC_NAME_LEN];

    graph_init();

    do
    {
        printf("\n\n===== WAREHOUSE GRAPH SYSTEM (Siddhant) =====");
        printf("\n1. Add Warehouse Location");
        printf("\n2. Add Path");
        printf("\n3. Remove Path");
        printf("\n4. Display Graph");
        printf("\n5. BFS Traversal");
        printf("\n6. DFS Traversal");
        printf("\n7. Shortest Path (Dijkstra)");
        printf("\n8. Block / Reopen Path (Rerouting)");
        printf("\n9. Exit");
        choice = readInt("\n\nEnter Choice: ");

        switch (choice)
        {
        case 1:
            printf("\nEnter Location Name: ");
            if (scanf(" %29[^\n]", name) != 1) return 0;
            r = graph_add_location(name);
            if (r) printf("\nLocation %d (%s) Added!\n", r, name);
            else printf("\nMaximum Locations Reached!\n");
            break;
        case 2:
            a = readInt("\nEnter Starting Location: ");
            b = readInt("Enter Ending Location: ");
            d = readInt("Enter Distance: ");
            printf(graph_add_path(a, b, d) ? "\nPath Added!\n" : "\nInvalid Path!\n");
            break;
        case 3:
            a = readInt("\nEnter Starting Location: ");
            b = readInt("Enter Ending Location: ");
            printf(graph_remove_path(a, b) ? "\nPath Removed!\n" : "\nNo Such Path!\n");
            break;
        case 4: graph_display(); break;
        case 5: graph_bfs(readInt("\nEnter Starting Location: ")); break;
        case 6: graph_dfs(readInt("\nEnter Starting Location: ")); break;
        case 7:
            a = readInt("\nEnter Starting Location: ");
            b = readInt("Enter Ending Location: ");
            graph_print_shortest_path(a, b);
            break;
        case 8:
            a = readInt("\nEnter Starting Location: ");
            b = readInt("Enter Ending Location: ");
            r = graph_toggle_block(a, b);
            if (r == 1) printf("\nPath BLOCKED. Routes will avoid it.\n");
            else if (r == 0) printf("\nPath Reopened.\n");
            else printf("\nNo Such Path!\n");
            break;
        case 9: printf("\nExiting...\n"); break;
        default: printf("\nInvalid Choice!");
        }
    } while (choice != 9);

    return 0;
}
#endif
