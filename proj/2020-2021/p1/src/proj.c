/* C Libraries. Only the bare minimum, no need for clutter */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef ASA_BENCHMARK
#include <time.h>
#endif

/*************************** Auxiliary functions ******************************/
#define get_number(a)     scanf("%d", a)
#define get_numbers(a, b) scanf("%d %d", a, b)
#define max(a, b) (a < b ? b : a)
#define min(a, b) (a > b ? b : a)

/*************************** Fake boolean structure ****************************/
typedef unsigned char bool;
#define false 0
#define true 1

/****************************** Vertex structure *******************************/
typedef int Vertex;
#define vertex_new(a)  a

int cmp_vertex(const void *a, const void *b) {
	return ( *(const Vertex*)b - *(const Vertex*)a );
}

/******************************* Edge structure ********************************/
typedef int Edge;

/****************************** Stack structure ********************************/
typedef struct stack {
	size_t idx;
	Vertex *data;   /* data[idx] = Vertex */
	bool *in_stack; /* in_stack[Vertex] = boolean */
} Stack;

void Stack_New(Stack *st, size_t size)
{
	if (st == NULL) { return; }
	st->idx = 0;
	st->data     = malloc(size* sizeof(*st->data));
	st->in_stack = calloc(size, sizeof(*st->in_stack));
}

void Stack_Push(Stack *st, Vertex u)
{
	if (st == NULL) { return; }
	if (st->in_stack[u]) { return; }
	st->data[++st->idx] = u;
	st->in_stack[u] = true;
}

int Stack_Pop(Stack *st)
{
	Vertex u;
	if (st == NULL) { return vertex_new(0); }
	u = st->data[st->idx--];
	st->in_stack[u] = 0;
	return u;
}

void Stack_Sort(Stack *st) {
	if (st == NULL) { return; }
	qsort(st->data+1, st->idx, sizeof(Vertex), cmp_vertex);
}
int Stack_Size(Stack *st) { return st->idx; }
bool Stack_Contains(Stack *st, Vertex u) { return st != NULL ? st->in_stack[u] : false; }
bool Stack_IsEmpty(Stack *st) { return st->idx == 0; }

void Stack_Reset(Stack *st) {
	memset(st->in_stack, 0, st->idx * sizeof(*st->in_stack));
	st->idx = 0;
}

void Stack_Destroy(Stack *st)
{
	if (st == NULL) { return; }
	free(st->data);     st->data = NULL;
	free(st->in_stack); st->in_stack = NULL;
}

/****************************** Graph structure *******************************/
typedef struct graph {

	/* Core graph data */
	int nr_vertices, nr_edges;
	bool is_bidir;

	/* Graph composition, represented as an Edge array */
	Edge   *first;    /* first[Vertex] = Edge   */
	Vertex *vertex;   /* vertex[Edge]  = Vertex */
	Edge   *next;     /* next[Edge]    = Edge   */

	/* Other data */
	int longest_path, nr_paths;

} Graph;

/* Connects two Vertices */
void Graph_Connect(Graph *g, Vertex u, Vertex v, bool undirected)
{
	g->nr_edges++;
	g->vertex[g->nr_edges] = v;

	if (g->first[u] == 0) {
		g->first[u] = g->nr_edges;
	} else {
		Edge adj;
		for (adj = g->first[u]; g->next[adj] != 0 && g->vertex[adj] != v; adj = g->next[adj]);
		/* if Vertex v is already in here, stop everything */
		if (g->vertex[adj] == v) {
			g->vertex[g->nr_edges--] = 0;
			return;
		}
		g->next[adj] = g->nr_edges;
	}

	if (undirected) { /* Add reverse Edge */
		Graph_Connect(g, v, u, false);
	}
}

/* Creates a new Graph */
void Graph_New(Graph *g, int num_v, int num_e, bool is_bidir)
{
	g->nr_vertices = num_v;
	g->nr_edges    = 0;
	g->is_bidir    = is_bidir;

	g->nr_paths = g->longest_path = 0;

	if (is_bidir) {
		num_e *= 2;
	}

	g->first  = calloc((num_v+1), sizeof(*g->first));
	g->vertex = calloc((num_e+1), sizeof(*g->vertex));
	g->next   = calloc((num_e+1), sizeof(*g->next));
}

/* Initializes Graph with input data */
void Graph_Init(Graph *g, int num_e)
{
	while (num_e-- > 0) {
		int num1, num2;
		Vertex u, v;

		/* Grab two numbers from input & convert them to Vertex */
		get_numbers(&num1, &num2);
		u = vertex_new(num1);
		v = vertex_new(num2);
		/* Connect them to the graph */
		Graph_Connect(g, u, v, g->is_bidir);
	}
}

void Graph_Reset(Graph *g)
{
	memset(g->first,  0, (g->nr_vertices+1) * sizeof(*g->first));
	memset(g->vertex, 0, (g->nr_edges+1)    * sizeof(*g->vertex));
	memset(g->next,   0, (g->nr_edges+1)    * sizeof(*g->next));
	g->nr_edges = 0;
}

void Graph_Print(Graph *g)
{
	int u;
	for (u = 1; u <= g->nr_vertices; u++) {
		int adj;
		for (adj = g->first[u]; adj != 0; adj = g->next[adj]) {
			int v = g->vertex[adj];
			printf("%d %d\n", u, v);
		}
	}
}

void Graph_Destroy(Graph *g)
{
	free(g->first);   g->first      = NULL;
	free(g->vertex);  g->vertex     = NULL;
	free(g->next);    g->next       = NULL;
}

/****************************** Topologic *******************************/
void DP(Graph *g, Stack *order, int *dist)
{
	while (!Stack_IsEmpty(order)) {
		int adj;
		int u = Stack_Pop(order);

		for (adj = g->first[u]; adj != 0; adj = g->next[adj]) {
			int v = g->vertex[adj];
			dist[v] = dist[u] + 1;
		}
	}
}

void DFS(Graph *g, Stack *order, bool *visited)
{
	Stack dfs;
	bool *is_last = calloc(g->nr_vertices+1, sizeof(*is_last));
	int src;

	Stack_New(&dfs, g->nr_vertices+1);
	Stack_New(order, g->nr_vertices+1); /* This needs to be always a new stack */

	for (src = 1; src <= g->nr_vertices; src++) {
		if (!visited[src]) {
			Stack_Push(&dfs, src);
			is_last[src] = false;
		}

		while (!Stack_IsEmpty(&dfs)) {
			int adj;
			int u = Stack_Pop(&dfs);

			if (is_last[u]) {
				Stack_Push(order, u);
				continue;
			}
			visited[u] = true;
			Stack_Push(&dfs, u);
			is_last[u] = true;

			for (adj = g->first[u]; adj != 0; adj = g->next[adj]) {
				int v = g->vertex[adj];
				if (!visited[v]) {
					Stack_Push(&dfs, v);
					is_last[v] = false;
				}
			}
		}
	}

	Stack_Destroy(&dfs);
	free(is_last);
}

void TopologicalSort(Graph *g)
{
	bool *visited = calloc((g->nr_vertices+1), sizeof(*visited));
	int *dist = calloc((g->nr_vertices+1), sizeof(*dist));
	int u;

	Stack order;

	/* Do Topologic sorting and gather our data from it */
	DFS(g, &order, visited);
	DP(g, &order, dist);

	for (u = 1; u <= g->nr_vertices; u++) {
		if (dist[u] == 0) { g->nr_paths++; }
		g->longest_path = max(g->longest_path, dist[u]);
	}
	g->longest_path++;

	/* Freeing data */
	Stack_Destroy(&order);
	free(visited);
	free(dist);
}

/***************************** MAIN function **********************************/
int main(void) {
	int num_v, num_e;
	Graph g;

	/* Grabbing input */
	get_number(&num_v); /* Grabbing number of vertices */
	get_number(&num_e); /* Grabbing number of edges */

	/* Instancing main graph from input */
	Graph_New(&g, num_v, num_e, false);
	Graph_Init(&g, num_e);

	/* Apply this project's magic */
	TopologicalSort(&g);

	/* Outputing solution */
	printf("%d %d\n", g.nr_paths, g.longest_path);

	/* Freeing data */
	Graph_Destroy(&g);

	return 0;
}
