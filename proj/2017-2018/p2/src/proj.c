/******************************* ASA - Grupo 3 *********************************
** António Sarmento - 77906
** Marta Simões     - 81947
*******************************************************************************/

/* C Libraries. Only the bare minimum, no need for clutter */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#ifdef ASA_BENCHMARK
#include <time.h>
#endif

/*************************** Auxiliary functions ******************************/
#define get_number(a)     scanf("%d", a)
#define get_numbers(a, b) scanf("%d %d", a, b)
#define max(a, b) (a < b ? b : a)
#define min(a, b) (a > b ? b : a)
#define print_spaces(x) printf("%"#x"c", ' ')

/******************** Data structures and their "methods" *********************/

#define INF INT_MAX
#define NIL -1

/* Fake boolean */
typedef unsigned char bool;
#define false 0
#define true 1

/* Pixel Structure */
typedef int Pixel;

/* Edge Structure */
typedef int Edge;

/* "Queue" structure */
typedef struct queue {
	int front, rear;
	Pixel *data;    /* data[idx] = Pixel */
	bool *in_queue; /* in_queue[Pixel] = boolean */
} Queue;

void queue_new(Queue *q, size_t size, bool refcount) {
	q->data     = malloc(size* sizeof(q->data));
	q->in_queue = refcount ? calloc(size, sizeof(q->in_queue)) : NULL;
	q->front = q->rear = 0;
}
void queue_destroy(Queue *q) {
	free(q->data); q->data = NULL;
	free(q->in_queue); q->in_queue = NULL;
}

void queue_push(Queue *q, Pixel u) {
	q->data[q->rear++] = u;
	if (q->in_queue) { q->in_queue[u] = true; }
}
Pixel queue_pop(Queue *q) {
	Pixel u = q->data[q->front++];
	if (q->in_queue) { q->in_queue[u] = false; }
	return u;
}

int queue_size(Queue *q) { return q->rear - q->front; }
bool queue_in_queue(Queue *q, Pixel u) { return q->in_queue ? q->in_queue[u] : false; }
bool queue_is_empty(Queue *q) { return q->front == q->rear; }
void queue_reset(Queue *q) {
	if (q->in_queue) { memset(q->in_queue, false, (q->rear)* sizeof(bool)); }
	q->front = q->rear = 0;
}

/* Graph Structure */
typedef struct graph {

	/* Core graph data */
	int m, n;
	int nr_vertices;
	int nr_edges;

	/* Graph composition, represented as an Edge array */
	Edge  *first;   /* first[Pixel] = Edge  */
	Edge  *last;    /* last[Pixel]  = Edge  */
	Pixel *pixel;   /* pixel[Edge]  = Pixel */
	Edge  *next;    /* next[Edge]   = Edge  */

	/* Network flow data */
	Pixel s, t; /* s == source, t == sink */

	Pixel *parent;  /* parent[Pixel] = Pixel */
	int *residual;  /* residual[Edge]  = int */
	int *capacity;  /* capacity[Edge]  = int */

	/* Other data */
	int weight;
	char *segments; /* segments[Pixel] = 'P' or 'C' */

} Graph;

/* Creates a new Graph */
void graph_new(Graph *g, int m, int n)
{
	int num_v = (m * n) + 1; /* Graph size + s + t */
	int num_e = (m * (n-1) + (m-1) * n) + 2 * num_v; /* Graph edges + 2 * num_v */

	g->s = 0;
	g->t = num_v;

	g->m = m; g->n = n;
	g->nr_vertices = num_v; /* Total size of the Graph */
	g->nr_edges = 0; /* Number of edges */
	g->weight = 0;

	g->first  = calloc((num_v+1), sizeof(g->first));
	g->last   = calloc((num_v+1), sizeof(g->last));
	g->pixel  = calloc((num_e+1), sizeof(g->pixel));
	g->next   = calloc((num_e+1), sizeof(g->next));

	g->parent   = malloc((num_v+1)* sizeof(g->parent));
	g->capacity = calloc((num_e+1), sizeof(g->capacity));
	g->residual = g->capacity;

	g->segments = malloc((num_v)* sizeof(g->segments));
	memset(g->segments, 'P', (num_v)* sizeof(g->segments));
}

void graph_destroy(Graph *g)
{
	free(g->first);    g->first    = NULL;
	free(g->last);     g->last     = NULL;
	free(g->pixel);    g->pixel    = NULL;
	free(g->next);     g->next     = NULL;

	free(g->parent);   g->parent   = NULL;
	free(g->capacity); g->capacity = NULL;

	free(g->segments); g->segments = NULL;
}

/* Returns the corresponding Pixel of the position given (i, j) */
Pixel graph_get_pixel(Graph *g, int i, int j) { return (i-1)*g->n + j; }
Pixel graph_h_neighbor(Graph *g, Pixel u) { return u+1; }
Pixel graph_v_neighbor(Graph *g, Pixel u) { return u+g->n; }

/* Returns the index of the connection between two Pixels. 0 if not found. */
Edge graph_get_edge(Graph *g, Pixel u, Pixel v)
{
	Edge adj; /* O(1) : Takes (4 pixel neighbors) amount of time */
	for (adj = g->first[u]; adj != 0 && g->pixel[adj] != v; adj = g->next[adj]);
	return adj;
}

/* Adds/Removes weight between two Pixels */
Edge graph_connect(Graph *g, Pixel u, Pixel v, int weight)
{
	Edge e = ++g->nr_edges;

	g->pixel[e] = v;
	g->capacity[e] = weight;

	if (g->first[u] == 0) {
		g->first[u] = e;
	} else {
		Edge adj = g->last[u];
		g->next[adj] = e;
	}
	g->last[u] = e;

	return e;
}

/* Initializes Graph with input data */
void graph_init(Graph *g)
{
	Pixel u, v;
	int i, j, w;
	int *weight_l = calloc((g->nr_vertices+1), sizeof(weight_l));

	/* Grabbing each Pixel L weight */
	for (u = 1; u < g->nr_vertices; u++) {
		get_number(&weight_l[u]);
	}

	/* Grabbing each Pixel C weight */
	for (u = 1; u < g->nr_vertices; u++) {
		int l = weight_l[u], c;
		get_number(&c);

		/* To avoid additional BFS cycles, we're creating only the necessary edges
		* by connecting the edges with the minimum weight.
		*/
		w = l - c;
		if (w > 0) {
			graph_connect(g, g->s, u,  w);
		} else if (w < 0) {
			graph_connect(g, u, g->t, -w);
		}
		g->weight += min(l, c);
	}
	free(weight_l);

	/* Grabbing horizontal neighboring weights */
	for (i = 1; i <= g->m; i++) {
		for (j = 1; j <= g->n-1; j++) {
			u = graph_get_pixel(g, i, j);
			v = graph_h_neighbor(g, u);

			get_number(&w);
			if (w > 0) {
				graph_connect(g, u, v, w);
				graph_connect(g, v, u, w);
			}
		}
	}

	/* Grabbing vertical neighboring weights */
	for (i = 1; i <= g->m-1; i++) {
		for (j = 1; j <= g->n; j++) {
			u = graph_get_pixel(g, i, j);
			v = graph_v_neighbor(g, u);

			get_number(&w);
			if (w > 0) {
				graph_connect(g, u, v, w);
				graph_connect(g, v, u, w);
			}
		}
	}
}

/* Graph weight gathering  */
int graph_get_f_weight(Graph *g, Pixel u, Pixel v) {
	return g->capacity[graph_get_edge(g, u, v)];
}
int graph_get_l_weight(Graph *g, Pixel u) { return graph_get_f_weight(g, g->s, u); }
int graph_get_c_weight(Graph *g, Pixel u) { return graph_get_f_weight(g, u, g->t); }
int graph_get_weight(Graph *g) { return g->weight; }

/* Prints weights of the Graph's Pixels and their connections */
void graph_print(Graph *g)
{
	Pixel u, v;
	int i, j = 0;

	for (i = u = 1; u < g->nr_vertices; i++, u = graph_v_neighbor(g, u)) {
		/* Printing Pixel's l and c weights */
		v = u;
		for (j = 1; j <= g->n; j++) {
			printf("( %d | %d )", graph_get_l_weight(g, v), graph_get_c_weight(g, v));
			if (j < g->n) {
				printf(" - %d - ", graph_get_f_weight(g, v, v+1));
			}
			v = graph_h_neighbor(g, v);
		}
		printf("\n");

		if (i >= g->m) { break; }

		/* Drawing 1st separator */
		for (j = 1; j <= g->n; j++) {
			print_spaces(4); printf("|");
			if (j < g->n) { print_spaces(11); }
		}
		printf("\n");

		/* Printing vertical Edge weights */
		v = u;
		for (j = 1; j <= g->n; j++) {
			print_spaces(4);
			printf("%d", graph_get_f_weight(g, v, graph_v_neighbor(g, v)));
			if (j < g->n) {
				print_spaces(11);
			}
			v = graph_h_neighbor(g, v);
		}
		printf("\n");

		/* Drawing 2nd separator */
		for (j = 1; j <= g->n; j++) {
			print_spaces(4); printf("|");
			if (j < g->n) { print_spaces(11); }
		}
		printf("\n");
	}
}

/* Prints the Graph represented through P or C */
void graph_print_segments(Graph *g)
{
	int i, j;

	for (i = 1; i <= g->m; i++) {
		/* Printing Pixel's segment type */
		for (j = 1; j <= g->n; j++) {
			Pixel u = graph_get_pixel(g, i, j);
			printf("%c ", g->segments[u]);
		}
		printf("\n");
	}
}


/***************************+* Algorithm code **************+******************/

/* Edmonds-Karp algorithm */
bool bfs(Graph *g, Queue *q, int *df)
{
	/* Initializing data */
	queue_reset(q);
	memset(g->parent+1, NIL, (g->nr_vertices)* sizeof(g->parent));

	/* Synchronizing queues */
	queue_push(q, g->s);

	/* O(V+E) : Scouring Graph. */
	while (!queue_is_empty(q)) {
		Edge adj;
        Pixel u = queue_pop(q);

        for (adj = g->first[u]; adj != 0; adj = g->next[adj]) {
            Pixel v = g->pixel[adj];
			int cf = g->residual[adj];

            if (g->parent[v] == NIL && cf > 0) {
				g->parent[v] = u;
				*df = min(*df, cf);
				if (v == g->t) { break; }
				queue_push(q, v);
            }
        }
    }

	return g->parent[g->t] != NIL;
}

int edmonds_karp(Graph *g)
{
	int flow = 0, df = INF;
	Pixel u, v;
	Queue q;

	#ifdef ASA_BENCHMARK
	int count = 0;
	clock_t start = 0, end = 0;
	float secs;
	#endif

	/* Initializing data */
	queue_new(&q, g->nr_vertices+1, false);

	/* Applying algorithm */
	#ifdef ASA_BENCHMARK
	start = clock();
	#endif
	while (bfs(g, &q, &df)) {
		/* We found an augmenting path. See how much flow we can send */
		Edge adj;
		if (df == 0 || df == INF) { break; }

		/* O(E) : Backtrack again */
		for (v = g->t, u = g->parent[v]; u!= g->s && v != g->parent[v]; v = u, u = g->parent[v]) {
			adj = graph_get_edge(g, u, v);
			g->residual[adj] -= df;

			adj = graph_get_edge(g, v, u);
			g->residual[adj] += df;
		}

		flow += df;
		df = INF;
		#ifdef ASA_BENCHMARK
		count++;
		#endif
	}
	#ifdef ASA_BENCHMARK
	end = clock();
	secs = (float)(end - start) / CLOCKS_PER_SEC;
	fprintf(stderr, "BFS: %.2lf seconds.\n", secs);
	#endif

	/* Checking for network cuts */
	for (u = 1; u < g->nr_vertices; u++) {
		if (g->parent[u] != NIL) {
			g->segments[u] = 'C';
		}
	}

	/* Destroying data */
	queue_destroy(&q);

	return flow;
}

void update_weights(Graph *g)
{
	#ifdef ASA_BENCHMARK
	clock_t start, end;
	float secs;

	start = clock();
	#endif
	/* Using Edmonds-Karp algorithm */
	g->weight += edmonds_karp(g);

	#ifdef ASA_BENCHMARK
	end = clock();

	secs = (float)(end - start) / CLOCKS_PER_SEC;
	// fprintf(stderr, "Algorithm: %.2lf seconds.\n", secs);
	#endif
}

#undef NIL


/***************************** MAIN function **********************************/
int main(void) {
	int m, n;
	Graph g;

	/* Grabbing dimensions of our Graph from input */
	get_numbers(&m, &n);

	/* Instancing graphs */
	graph_new(&g, m, n);
	graph_init(&g); /* Initializing Graph from input */

	/* Apply this project's magic */
	update_weights(&g);

	/* Printing required output */
	printf("%d\n\n", graph_get_weight(&g));
	graph_print_segments(&g);

	/* Freeing data */
	graph_destroy(&g);

	return 0;
}
