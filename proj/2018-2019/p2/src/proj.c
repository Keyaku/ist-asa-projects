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
#define max(a, b) (a < b ? b : a)
#define min(a, b) (a > b ? b : a)
#define get_number(a) scanf("%d", a)
#define get_2_numbers(a, b) scanf("%d %d", a, b)
#define get_3_numbers(a, b, c) scanf("%d %d %d", a, b, c)

/*************************** Fake boolean structure ****************************/
typedef unsigned char bool;
#define false 0
#define true 1

/****************************** Vertex structure *******************************/
typedef int Vertex;
#define vertex_new(a)  a
#define vertex_next(a) a + 1
#define vertex_prev(a) a - 1
#define vertex_root()  1
#define vertex_end(GRAPH) GRAPH->nr_vertices
#define vertex_iter(GRAPH, a) a <= vertex_end(GRAPH)

int cmp_vertex(const void *a, const void *b) {
	return ( *(const Vertex*)b - *(const Vertex*)a );
}

/******************************* Edge structure ********************************/
typedef int Edge;

/*************************** Weighted Graph structure **************************/
typedef struct graph {

	/* Core graph data */
	int nr_vertices;
	int nr_edges;

	/* Graph composition, represented as an Edge array */
	Edge   *first;    /* first[Vertex] = Edge   */
	Vertex *vertex;   /* vertex[Edge]  = Vertex */
	Edge   *next;     /* next[Edge]    = Edge   */

	/* Other data */
	int *v_weight;  /* v_weight[Vertex] = weight */
	int *e_weight;  /* e_weight[Edge]   = weight */

} Graph;

#define max_capacity(GRAPH) GRAPH->v_weight[0]

/* Finds a specified Edge. Returns 0 if not found */
Edge graph_find_edge(Graph *g, Vertex u, Vertex v)
{
	Edge edge;
	for (edge = g->first[u]; g->next[edge] != 0 && g->vertex[edge] != v; edge = g->next[edge]);
	return edge;
}

/* Connects two Vertices */
Edge graph_connect(Graph *g, Vertex u, Vertex v)
{
	Edge edge = ++g->nr_edges;
	g->vertex[edge] = v;

	if (g->first[u] == 0) {
		g->first[u] = edge;
	} else {
		edge = graph_find_edge(g, u, v);
		/* if Vertex v is already in here, stop everything */
		if (g->vertex[edge] == v) {
			g->vertex[g->nr_edges--] = 0;
			return 0;
		}
		g->next[edge] = g->nr_edges;
	}

	return edge;
}

/* Creates a new Graph */
void graph_new(Graph *g, int num_v, int num_e)
{
	g->nr_vertices = num_v;
	g->nr_edges    = 0;

	g->first  = calloc((num_v+1), sizeof(*g->first));
	g->vertex = calloc((num_e+1), sizeof(*g->vertex));
	g->next   = calloc((num_e+1), sizeof(*g->next));

	g->v_weight = calloc((num_v+1), sizeof(*g->v_weight));
	g->e_weight = calloc((num_e+1), sizeof(*g->e_weight));
}

/* Initializes Graph with input data */
void graph_init(Graph *g, int num_e)
{
	while (num_e-- > 0) {
		int num1, num2;
		Vertex u, v;
		Edge edge;

		/* Grab two numbers from input & convert them to Vertex */
		get_2_numbers(&num1, &num2);
		u = vertex_new(num1);
		v = vertex_new(num2);
		/* Connect them to the graph */
		edge = graph_connect(g, u, v);

		/* Adding Edge weight */
		get_number(&num1);
		g->e_weight[edge] = num1;

	}
}

void graph_add_weights(Graph *g)
{
	Vertex u = vertex_root();
	/* Adding Vertex weights */
	for (u = vertex_next(u); vertex_iter(g, vertex_prev(u)); u = vertex_next(u)) {
		int num; get_number(&num);
		g->v_weight[u] = num;
	}
}

void graph_destroy(Graph *g)
{
	free(g->first);   g->first      = NULL;
	free(g->vertex);  g->vertex     = NULL;
	free(g->next);    g->next       = NULL;

	free(g->v_weight); g->v_weight  = NULL;
	free(g->e_weight); g->e_weight  = NULL;
}

/*************************** Special structure ********************************/


/***************************** MAIN function **********************************/
int main(void) {
	int f, e, t;
	Graph g;

	/* Grabbing Graph's main data */
	get_3_numbers(&f, &e, &t);

	/* Instancing Graph from input */
	graph_new(&g, f+e, t);
	graph_add_weights(&g); /* Adding weights to each vertex */
	graph_init(&g, t);

	/* Apply this project's magic */
	// TODO

	/* Freeing data */
	graph_destroy(&g);

	return 0;
}
