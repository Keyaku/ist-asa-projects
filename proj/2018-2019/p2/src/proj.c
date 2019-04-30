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

/****************************** Queue structure ********************************/
typedef struct queue {
	int front, rear;
	Vertex *data;    /* data[idx] = Vertex */
	bool *in_queue; /* in_queue[Vertex] = boolean */
} Queue;

void queue_new(Queue *q, size_t size, bool refcount) {
	q->data     = malloc(size * sizeof(*q->data));
	q->in_queue = refcount ? calloc(size, sizeof(*q->in_queue)) : NULL;
	q->front = q->rear = 0;
}
void queue_destroy(Queue *q) {
	free(q->data);     q->data = NULL;
	free(q->in_queue); q->in_queue = NULL;
}

void queue_push(Queue *q, Vertex u) {
	q->data[q->rear++] = u;
	if (q->in_queue) { q->in_queue[u] = true; }
}
Vertex queue_pop(Queue *q) {
	Vertex u = q->data[q->front++];
	if (q->in_queue) { q->in_queue[u] = false; }
	return u;
}

int queue_size(Queue *q) { return q->rear - q->front; }
bool queue_in_queue(Queue *q, Vertex u) { return q->in_queue ? q->in_queue[u] : false; }
bool queue_is_empty(Queue *q) { return q->front == q->rear; }
void queue_reset(Queue *q) {
	if (q->in_queue) { memset(q->in_queue, false, (q->rear) * sizeof(*q->in_queue)); }
	q->front = q->rear = 0;
}

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
	int *capacity;   /* capacity[Edge] */

} Graph;

#define source 0
#define sink   1

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
		Edge adj = graph_find_edge(g, u, v);
		/* if Vertex v is already in here, stop everything */
		if (g->vertex[adj] == v) {
			g->vertex[g->nr_edges--] = 0;
			return adj;
		}
		g->next[adj] = g->nr_edges;
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

	g->capacity = calloc((num_e+1), sizeof(*g->capacity));
}

/* Initializes Graph with input data */
void graph_init(Graph *g, int num_e)
{
	while (num_e-- > 0) {
		int num1, num2;
		Vertex u, v;
		Edge e;

		/* Grab two numbers from input & convert them to Vertex */
		get_2_numbers(&num1, &num2);
		u = vertex_new(num1);
		v = vertex_new(num2);

		/* Adding Edge to Graph */
		e = graph_connect(g, u, v);
		get_number(&num1);
		g->capacity[e] = num1;
	}
}

void graph_add_weights(Graph *g)
{
	Vertex u = vertex_root();
	/* Adding Vertex weights */
	for (u = vertex_next(u); vertex_iter(g, vertex_prev(u)); u = vertex_next(u)) {
		int num;
		Edge e;

		/* Adding source Edge to Graph */
		e = graph_connect(g, source, u);
		get_number(&num);
		g->capacity[e] = num;
	}
}

void graph_print(Graph *g)
{
	Vertex u;
	for (u = source; vertex_iter(g, u); u = vertex_next(u)) {
		Edge adj;

		for (adj = g->first[u]; adj != 0; adj = g->next[adj]) {
			Vertex v = g->vertex[adj];

			if (g->capacity[v] > 0) {
				u == source ? printf("s") : printf("%d", u);
				printf(" -- %3d --> ", g->capacity[v]);
				v == sink ? printf("t\n") : printf("%d\n", v);
			}
		}
	}

}

void graph_destroy(Graph *g)
{
	free(g->first);   g->first      = NULL;
	free(g->vertex);  g->vertex     = NULL;
	free(g->next);    g->next       = NULL;

	free(g->capacity); g->capacity  = NULL;
}

/*************************** Special structure ********************************/
bool bfs(Graph *g, Queue *q)
{
	/* Resetting data */
	queue_reset(q);

	/* Adding source to Queue */
	queue_push(q, source);

	while (!queue_is_empty(q)) {
		Edge adj;
		Vertex u = queue_pop(q);

		for (adj = g->first[u]; adj != 0; adj = g->next[adj]) {
			Vertex v = g->vertex[adj];

			// TODO: Add vertex to Queue if condition is verified
		}
	}

	return false; // FIXME: Add condition
}


void apply(Graph *g)
{
	int i, size;
	int max_flow = -1;
	Queue stations;

	/* Initializing data */
	queue_new(&stations, g->nr_vertices+1, true);

	/* Summoning algorithm */
	// TODO

	/* Outputting */
	printf("%d\n", max_flow);

	size = queue_size(&stations);
	for (i = 0; i < size; i++) {
		Vertex u = queue_pop(&stations);
		printf("%d", u);
		if (i+1 < size) printf(" ");
	} printf("\n");

	for (i = 0; i < 0; i++) {
		// TODO: print sequence of src-dst vertices that need augmenting
	}

	/* Destroying data */
	queue_destroy(&stations);
}

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
	apply(&g);

	/* Freeing data */
	graph_destroy(&g);

	return 0;
}
