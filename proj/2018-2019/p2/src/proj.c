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

#define vertex_nil  -1
#define source 0
#define sink   1

void vertex_print(Vertex u) {
	u == source ? printf("s") : u == sink ? printf("t") : printf("%d", u);
}

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
	q->data     = malloc((size+1) * sizeof(*q->data));
	q->in_queue = refcount ? calloc((size+1), sizeof(*q->in_queue)) : NULL;
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
	Edge   *prev;     /* prev[Edge]    = Edge   */

	/* Other data */
	int *flow;       /* flow[Edge] */
	int *capacity;   /* capacity[Edge] */
	int *v_minimum;  /* v_minimum[Vertex] */

} Graph;


/* Finds a specified Edge. */
Edge graph_find_next_edge(Graph *g, Vertex u, Vertex v)
{
	Edge e = g->first[u];
	while (g->next[e] != 0 && g->vertex[e] != v) e = g->next[e];
	return e;
}

Edge graph_find_edge(Graph *g, Vertex u, Vertex v)
{
	Edge e = g->first[u];
	while (e != 0 && g->vertex[e] != v) e = g->next[e];
	return e;
}

/* Connects two Vertices */
Edge graph_connect(Graph *g, Vertex u, Vertex v)
{
	Edge edge = ++g->nr_edges;
	g->vertex[edge] = v;

	if (g->first[u] == 0) {
		g->first[u] = edge;
	} else {
		Edge adj = graph_find_next_edge(g, u, v);
		/* if Vertex v is already in here, stop everything */
		if (g->vertex[adj] == v) {
			g->vertex[g->nr_edges--] = vertex_nil;
			return adj;
		}
		g->next[adj] = edge;
	}

	return edge;
}

/* Adds weighted Edge to Graph */
void graph_add_edge(Graph *g, Vertex u, Vertex v)
{
	int num;
	Edge e = graph_connect(g, u, v);
	get_number(&num);
	g->capacity[e] = num;
	g->prev[e] = graph_connect(g, v, u);
}

/* Creates a new Graph */
void graph_new(Graph *g, int num_v, int num_e)
{
	g->nr_vertices = num_v;
	g->nr_edges    = 0;

	num_e *= 2;

	g->first  = calloc((num_v+1), sizeof(*g->first));
	g->vertex = malloc((num_e+1)* sizeof(*g->vertex));
	memset(g->vertex, vertex_nil, (num_e+1)*sizeof(*g->vertex));
	g->next   = calloc((num_e+1), sizeof(*g->next));
	g->prev   = calloc((num_e+1), sizeof(*g->prev));

	g->flow      = calloc((num_e+1), sizeof(*g->flow));
	g->capacity  = calloc((num_e+1), sizeof(*g->capacity));
	g->v_minimum = calloc((num_v+1), sizeof(*g->v_minimum));
}

/* Initializes Graph with input data */
void graph_init(Graph *g, int num_e)
{
	while (num_e-- > 0) {
		int num1, num2;
		Vertex u, v;

		/* Grab two numbers from input & convert them to Vertex */
		get_2_numbers(&num1, &num2);
		u = vertex_new(num1);
		v = vertex_new(num2);

		graph_add_edge(g, u, v);
	}
}

void graph_add_sources(Graph *g, int vertices, Vertex v)
{
	Vertex u = source;
	int i;

	/* Adding Vertex capacity */
	for (i = 0, v = vertex_next(v); i < vertices; v = vertex_next(v), i++) {
		graph_add_edge(g, u, v);
	}
}

void graph_add_stops(Graph *g, int vertices, Vertex v)
{
	int i;

	for (i = 0, v = vertex_next(v); i < vertices; v = vertex_next(v), i++) {
		int num;
		get_number(&num);
		g->v_minimum[v] = num;
	}
}

void graph_print(Graph *g)
{
	Vertex u; Edge adj;

	for (u = source; vertex_iter(g, u); u = vertex_next(u))
	for (adj = g->first[u]; adj != 0; adj = g->next[adj]) {
		Vertex v = g->vertex[adj];
		vertex_print(u);
		printf(" -- %3d --> ", g->capacity[adj]);
		vertex_print(v);
		printf("\n");
	}

}

void graph_destroy(Graph *g)
{
	free(g->first);   g->first   = NULL;
	free(g->vertex);  g->vertex  = NULL;
	free(g->next);    g->next    = NULL;
	free(g->prev);    g->prev    = NULL;

	free(g->flow);      g->flow      = NULL;
	free(g->capacity);  g->capacity  = NULL;
	free(g->v_minimum); g->v_minimum = NULL;
}

/*************************** Special structure ********************************/
typedef struct {
	int value;
	int *level;
	Queue q_data, *q;
	Queue q_stations, *stations;
	Queue q_edges, *edges;
} MaxFlow;

void maxflow_new(MaxFlow *mf, int size)
{
	mf->value = 0;
	mf->level = malloc(size * sizeof(*mf->level));

	mf->q = &mf->q_data;
	mf->stations = &mf->q_stations;
	mf->edges = &mf->q_edges;
	queue_new(mf->q, size, true);
	queue_new(mf->stations, size, true);
	queue_new(mf->edges, size*2, false);
}

void maxflow_output(MaxFlow *mf)
{
	int i, size;

	/* Printing Max Flow */
	printf("%d\n", mf->value);

	/* Printing Stations in need of augmenting */
	size = queue_size(mf->stations);
	for (i = 0; i < size; i++) {
		Vertex u = queue_pop(mf->stations);
		printf("%d", u);
		if (i+1 < size) printf(" ");
	} printf("\n");

	/* Printing Edges in need of augmenting (closest to sink) */
	size = queue_size(mf->edges);
	for (i = 0; i < size; i++) {
		Vertex u = queue_pop(mf->edges);
		Vertex v = queue_pop(mf->edges);

		printf("%d %d\n", u, v);
	}
}

void maxflow_destroy(MaxFlow *mf)
{
	free(mf->level); mf->level = NULL;

	queue_destroy(mf->q); mf->q = NULL;
	queue_destroy(mf->stations); mf->stations = NULL;
	queue_destroy(mf->edges); mf->edges = NULL;
}


/* BFS material */
bool bfs_minimum_cut(Graph *g, MaxFlow *mf)
{
	/* Resetting data */
	queue_reset(mf->q);

	/* Adding source to Queue */
	queue_push(mf->q, source);

	while (!queue_is_empty(mf->q)) {
		Edge adj;
		Vertex u = queue_pop(mf->q);

		for (adj = g->first[u]; adj > 0; adj = g->next[adj]) {
			Vertex v = g->vertex[adj];
			int cap = g->capacity[adj];
			int flow = g->flow[adj];

			if (flow) {
				if (!queue_in_queue(mf->q, v)) queue_push(mf->q, v);

				if (g->v_minimum[u] != 0 && g->v_minimum[u] < cap) {
					if (!queue_in_queue(mf->stations, u)) queue_push(mf->stations, u);
				}
			}
		}
	}

	return false;
}

bool bfs_update_level(Graph *g, MaxFlow *mf)
{
	/* Resetting data */
	queue_reset(mf->q);
	memset(mf->level, -1, (g->nr_vertices+1) * sizeof(*mf->level));

	/* Adding source to Queue */
	queue_push(mf->q, source);
	mf->level[source] = 0;

	while (!queue_is_empty(mf->q)) {
		Edge adj;
		Vertex u = queue_pop(mf->q);

		for (adj = g->first[u]; adj > 0; adj = g->next[adj]) {
			Vertex v = g->vertex[adj];

			int cap = (g->v_minimum[u] != 0) ? min(g->v_minimum[u], g->capacity[adj]) : g->capacity[adj];
			cap -= g->flow[adj];

			if (mf->level[v] < 0 && 0 < cap) {
				mf->level[v] = mf->level[u] + 1;
				queue_push(mf->q, v);
			}
		}
	}

	return mf->level[sink] >= 0;
}

/* Algorithm code */
int send_flow(Graph *g, Vertex u, int flow, MaxFlow *mf)
{
	Edge adj;

	if (u == sink) return flow;

	for (adj = g->first[u]; adj != 0; adj = g->next[adj]) {
		Vertex v = g->vertex[adj];

		int cap = (g->v_minimum[u] != 0) ? min(g->v_minimum[u], g->capacity[adj]) : g->capacity[adj];
		cap -= g->flow[adj];

		if (mf->level[v] == mf->level[u]+1 && 0 < cap) {
			int curr_flow = min(flow, cap);
			int temp_flow = send_flow(g, v, curr_flow, mf);

			if (temp_flow > 0) {
				g->flow[adj] += temp_flow; /* add flow to current edge */
				return temp_flow;
			}
		}
	}

	return 0;
}

int dinic(Graph *g, MaxFlow *mf)
{
	/* Running algorithms */
	while (bfs_update_level(g, mf)) {
		int flow;
		while ((flow = send_flow(g, source, __INT_MAX__, mf))) {
			mf->value += flow;
		}
	}

	bfs_minimum_cut(g, mf);

	return mf->value;
}

void apply(Graph *g)
{
	MaxFlow mf;

	maxflow_new(&mf, g->nr_vertices+1);
	dinic(g, &mf); /* Summoning algorithm Dinic, O(E V^2) */
	maxflow_output(&mf);
	maxflow_destroy(&mf);
}

/***************************** MAIN function **********************************/
int main(void) {
	int f, e, t;
	Graph g;

	/* Grabbing Graph's main data */
	get_3_numbers(&f, &e, &t);

	/* Instancing Graph from input */
	graph_new(&g, f+e+1, f+t);
	graph_add_sources(&g, f, sink); /* Adding capacity to each vertex */
	graph_add_stops(&g, e, vertex_new(f+1));
	graph_init(&g, t);

	/* Apply this project's magic */
	apply(&g);

	/* Freeing data */
	graph_destroy(&g);

	return 0;
}
