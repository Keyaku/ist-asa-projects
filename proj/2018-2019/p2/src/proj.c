/******************************* ASA - Grupo 25 *********************************
** António Sarmento - 77906
** André Dias Nobre - 79763
*******************************************************************************/

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
#define vertex_next(a) a + 1
#define vertex_prev(a) a - 1
#define vertex_root(a) 1
#define vertex_end(GRAPH) GRAPH->nr_vertices
#define vertex_iter(GRAPH, a) a <= vertex_end(GRAPH)

int cmp_vertex(const void *a, const void *b) {
	return ( *(const Vertex*)b - *(const Vertex*)a );
}

/******************************* Edge structure ********************************/
typedef int Edge;

/****************************** Graph structure *******************************/
typedef struct graph {

	/* Core graph data */
	int nr_vertices;
	int nr_edges;
	bool is_bidir;

	/* Graph composition, represented as an Edge array */
	Edge   *first;    /* first[Vertex] = Edge   */
	Vertex *vertex;   /* vertex[Edge]  = Vertex */
	Edge   *next;     /* next[Edge]    = Edge   */

	/* Other data */

} Graph;

/* Connects two Vertices */
void graph_connect(Graph *g, Vertex u, Vertex v, bool undirected)
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
		graph_connect(g, v, u, false);
	}
}

/* Creates a new Graph */
void graph_new(Graph *g, int num_v, int num_e, bool is_bidir)
{
	g->nr_vertices = num_v;
	g->nr_edges    = 0;
	g->is_bidir    = is_bidir;

	if (is_bidir) {
		num_e *= 2;
	}

	g->first  = calloc((num_v+1), sizeof(*g->first));
	g->vertex = calloc((num_e+1), sizeof(*g->vertex));
	g->next   = calloc((num_e+1), sizeof(*g->next));
}

/* Initializes Graph with input data */
void graph_init(Graph *g, int num_e)
{
	while (num_e-- > 0) {
		int num1, num2;
		Vertex u, v;

		/* Grab two numbers from input & convert them to Vertex */
		get_numbers(&num1, &num2);
		u = vertex_new(num1);
		v = vertex_new(num2);
		/* Connect them to the graph */
		graph_connect(g, u, v, g->is_bidir);
	}
}

void graph_reset(Graph *g)
{
	memset(g->first,  0, (g->nr_vertices+1) * sizeof(*g->first));
	memset(g->vertex, 0, (g->nr_edges+1)    * sizeof(*g->vertex));
	memset(g->next,   0, (g->nr_edges+1)    * sizeof(*g->next));
	g->nr_edges = 0;
}

void graph_destroy(Graph *g)
{
	free(g->first);   g->first      = NULL;
	free(g->vertex);  g->vertex     = NULL;
	free(g->next);    g->next       = NULL;
}

/*************************** Special structure ********************************/


/***************************** MAIN function **********************************/
int main(void) {
	int num_v, num_e;
	Graph g;

	/* Grabbing input */
	get_number(&num_v); /* Grabbing number of vertices */
	get_number(&num_e); /* Grabbing number of edges */

	/* Instancing main graph from input */
	graph_new(&g, num_v, num_e, true);
	graph_init(&g, num_e);

	/* Apply this project's magic */
	// TODO

	/* Freeing data */
	graph_destroy(&g);

	return 0;
}
