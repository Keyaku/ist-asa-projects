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

/****************************** Stack structure ********************************/
typedef struct stack {
	size_t idx;
	Vertex *data;   /* data[idx] = Vertex */
	bool *in_stack; /* in_stack[Vertex] = boolean */
} Stack;

void stack_new(Stack *st, size_t size)
{
	if (st == NULL) { return; }
	st->idx = 0;
	st->data     = malloc(size* sizeof(*st->data));
	st->in_stack = calloc(size, sizeof(*st->in_stack));
}

void stack_push(Stack *st, Vertex u)
{
	if (st == NULL) { return; }
	if (st->in_stack[u]) { return; }
	st->data[++st->idx] = u;
	st->in_stack[u] = true;
}

int stack_pop(Stack *st)
{
	Vertex u;
	if (st == NULL) { return vertex_new(0); }
	u = st->data[st->idx--];
	st->in_stack[u] = 0;
	return u;
}

void stack_sort(Stack *st) {
	if (st == NULL) { return; }
	qsort(st->data+1, st->idx, sizeof(Vertex), cmp_vertex);
}
int stack_size(Stack *st) { return st->idx; }
bool stack_contains(Stack *st, Vertex u) { return st != NULL ? st->in_stack[u] : false; }
bool stack_is_empty(Stack *st) { return st->idx == 0; }

void stack_reset(Stack *st) {
	memset(st->in_stack, 0, st->idx * sizeof(*st->in_stack));
	st->idx = 0;
}

void stack_destroy(Stack *st)
{
	if (st == NULL) { return; }
	free(st->data);     st->data = NULL;
	free(st->in_stack); st->in_stack = NULL;
}

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
	Vertex *parent;

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

	g->parent = calloc((num_v+1), sizeof(*g->parent));
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
	memset(g->parent, 0, (g->nr_vertices+1) * sizeof(*g->parent));
	g->nr_edges = 0;
}

void graph_destroy(Graph *g)
{
	free(g->first);   g->first      = NULL;
	free(g->vertex);  g->vertex     = NULL;
	free(g->next);    g->next       = NULL;
	free(g->parent);  g->parent     = NULL;
}

/****************************** SCC structure *******************************/
typedef struct scc_data {
	int biggest_scc;  /* Biggest size of one the SCCs after audit */

	Stack ids, ap;
} SCC_data;

void SCC_new(SCC_data *scc, size_t size)
{
	scc->biggest_scc = 0;
	stack_new(&scc->ids, size);
	stack_new(&scc->ap, size);
}

void SCC_destroy(SCC_data *scc)
{
	stack_destroy(&scc->ids);
	stack_destroy(&scc->ap);
}

void SCC_print(SCC_data *scc)
{
	/* Print number of SCCs */
	printf("%d\n", stack_size(&scc->ids));

	/* Print all last nodes found in SCC */
	while (!stack_is_empty(&scc->ids)) {
		Vertex u = stack_pop(&scc->ids);
		printf("%d", u);
		if (!stack_is_empty(&scc->ids)) { printf(" "); }
	}
	printf("\n");

	/* Print number of nodes that, when removed, create more SCCs */
	printf("%d\n", stack_size(&scc->ap));

	/* Print the biggest SCC's node count */
	printf("%d\n", scc->biggest_scc);
}

/* Runs DFS through graph whilst ignoring Articulation Points */
void count_scc(
	Graph *g, SCC_data *scc,
	Stack *st,
	Vertex u,
	int *disc, int *low, int *disc_time
) {
	Edge adj;

	if (stack_contains(&scc->ap, u)) { return; }

	disc[u] = low[u] = ++(*disc_time);
	stack_push(st, u);

	for (adj = g->first[u]; adj != 0; adj = g->next[adj]) {
		Vertex v = g->vertex[adj];

		if (stack_contains(&scc->ap, v)) { continue; }

		/* If v is not visited yet, recur for it */
		if (disc[v] == 0) {
			count_scc(g, scc, st, v, disc, low, disc_time);
			low[u] = min(low[u], low[v]);
		}
		/* Update low value of 'u' only if 'v' is still in stack */
		else if (stack_contains(st, v)) {
			low[u] = min(low[u], disc[v]);
		}
	}

	/* head node found; so it's an SCC. Popping stack until we reach head node. */
	if (low[u] == disc[u]) {
		Vertex v;
		scc->biggest_scc = max(scc->biggest_scc, stack_size(st));
		while ((v = stack_pop(st)) != u);
	}
}

/* Apply Tarjan's algorithm to find SCCs */
void tarjan_aux(
	Graph *g, SCC_data *scc,
	Stack *st,
	Vertex u,
	int *disc, int *low, int *disc_time
) {
	Edge adj;
	int children = 0;

	disc[u] = low[u] = ++(*disc_time);
	stack_push(st, u);

	for (adj = g->first[u]; adj != 0; adj = g->next[adj]) {
		Vertex v = g->vertex[adj];

		/* If v is not visited yet, recur for it */
		if (disc[v] == 0) {
			g->parent[v] = u;
			children++;
			tarjan_aux(g, scc, st, v, disc, low, disc_time);
			low[u] = min(low[u], low[v]);

			if (g->parent[u] == 0 && children > 1) {
				stack_push(&scc->ap, u);
			}
			if (g->parent[u] != 0 && low[v] >= disc[u]) {
				stack_push(&scc->ap, u);
			}
		}
		/* Update low value of 'u' only if 'v' is still in stack */
		else if (stack_contains(st, v)) {
			low[u] = min(low[u], disc[v]);
		}
	}

	/* head node found; so it's an SCC. Popping stack until we reach head node. */
	if (low[u] == disc[u]) {
		Vertex v, tail = u;
		while ((v = stack_pop(st)) != u) {
			tail = max(tail, v); /* Getting the tail of the SCC */
		}
		stack_push(&scc->ids, tail);
	}
}

void tarjan(Graph *g, SCC_data *scc)
{
	Vertex u;
	int *disc, *low;
	int disc_time = 0;
	Stack st;

	/* Initializing data */
	disc = calloc(g->nr_vertices+1, sizeof(*disc));
	low  = calloc(g->nr_vertices+1, sizeof(*low));
	stack_new(&st, g->nr_vertices+1);

	/* Performing 1st DFS */
	for (u = vertex_root(); vertex_iter(g, u); u = vertex_next(u)) {
        if (disc[u] == 0) {
            tarjan_aux(g, scc, &st, u, disc, low, &disc_time);
		}
	}

	/* Performing 2nd DFS for SCC counting */
	memset(disc, 0, (g->nr_vertices+1) * sizeof(*disc));
	memset(low,  0, (g->nr_vertices+1) * sizeof(*low));
	stack_reset(&st);
	for (u = vertex_root(); vertex_iter(g, u); u = vertex_next(u)) {
        if (disc[u] == 0) {
            count_scc(g, scc, &st, u, disc, low, &disc_time);
		}
	}

	/* Freeing data */
	stack_destroy(&st);
	free(disc);
	free(low);
}

void graph_find_SCCs(Graph *g)
{
	SCC_data scc;
		#ifdef ASA_BENCHMARK
		clock_t start, end;
		float secs;

		start = clock();
		#endif

	SCC_new(&scc, g->nr_vertices+1);

		#ifdef ASA_BENCHMARK
		end = clock();
		secs = (float)(end - start) / CLOCKS_PER_SEC;
		fprintf(stderr, "Creation: %.6lf seconds.\n", secs);
		#endif

		#ifdef ASA_BENCHMARK
		start = clock();
		#endif
	tarjan(g, &scc);
		#ifdef ASA_BENCHMARK
		end = clock();
		secs = (float)(end - start) / CLOCKS_PER_SEC;
		fprintf(stderr, "Algorithm: %.6lf seconds.\n", secs);
		#endif

		#ifdef ASA_BENCHMARK
		start = clock();
		#endif
	stack_sort(&scc.ids); /* This adds one more to the complexity */
		#ifdef ASA_BENCHMARK
		end = clock();
		secs = (float)(end - start) / CLOCKS_PER_SEC;
		fprintf(stderr, "Sorting: %.6lf seconds.\n", secs);
		fprintf(stderr, "\n");
		#endif

	SCC_print(&scc);

	SCC_destroy(&scc);
}

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
	graph_find_SCCs(&g);

	/* Freeing data */
	graph_destroy(&g);

	return 0;
}
