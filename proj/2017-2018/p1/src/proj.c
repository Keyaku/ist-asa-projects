/******************************* ASA - Grupo 3 *********************************
** António Sarmento - 77906
** Marta Simões     - 81947
*******************************************************************************/

/* C Libraries. Only the bare minimum, no need for clutter */
#include <stdio.h>
#include <stdlib.h>

/*************************** Auxiliary functions ******************************/
#define get_number(a)     scanf("%d", a)
#define get_numbers(a, b) scanf("%d %d", a, b)
#define max(a, b) (a < b ? b : a)
#define min(a, b) (a > b ? b : a)

/******************** Data structures and their "methods" *********************/

/* Fake boolean */
typedef unsigned char bool;
#define false 0
#define true 1

/* Vertex Structure */
typedef int Vertex;
#define vertex_new(a)  a
#define vertex_next(a) a + 1
#define vertex_root(a) 1
#define vertex_end(GRAPH, a)  a <= GRAPH->nr_vertices

int cmp_vertex(const void *a, const void *b)
{
	return ( *(const Vertex*)a - *(const Vertex*)b );
}

/* Edge Structure */
typedef int Edge;

/* Stack structure */
typedef struct stack {
	size_t idx;
	Vertex *data;            /* data[idx] = Vertex */
	bool *in_stack; /* in_stack[Vertex] = boolean */
} Stack;

void stack_new(Stack *st, size_t size)
{
	st->idx = 0;
	st->data     = malloc(size* sizeof(st->data));
	st->in_stack = calloc(size, sizeof(st->in_stack));
}

void stack_push(Stack *st, Vertex u)
{
	st->data[++st->idx] = u;
	st->in_stack[u] = 1;
}

int stack_pop(Stack *st)
{
	Vertex u = st->data[st->idx--];
	st->in_stack[u] = 0;
	return u;
}

int stack_contains(Stack *st, Vertex u) { return st->in_stack[u]; }
int stack_is_empty(Stack *st) { return st->idx == 0; }

void stack_destroy(Stack *st)
{
	free(st->data);     st->data = NULL;
	free(st->in_stack); st->in_stack = NULL;
}

/* Graph Structure */
typedef struct graph {

	/* Core graph data */
	int nr_vertices;
	int nr_edges;

	/* Graph composition, represented as an Edge array */
	Edge   *first;    /* first[Vertex] = Edge   */
	Vertex *vertex;   /* vertex[Edge]  = Vertex */
	Edge   *next;     /* next[Edge]    = Edge   */

} Graph;

/* Connects two Vertices */
void graph_connect(Graph *g, Vertex u, Vertex v)
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
}

/* Creates a new Graph */
void graph_new(Graph *g, int num_v, int num_e)
{
	g->nr_vertices = num_v;
	g->nr_edges    = 0;

	g->first  = calloc((num_v+1), sizeof(g->first));
	g->vertex = calloc((num_e+1), sizeof(g->vertex));
	g->next   = calloc((num_e+1), sizeof(g->next));
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
		graph_connect(g, u, v);
	}
}

void graph_destroy(Graph *g)
{
	free(g->first);    g->first    = NULL;
	free(g->vertex);   g->vertex   = NULL;
	free(g->next);     g->next     = NULL;
}

void graph_sort(Graph *g)
{
	int idx = 0;
	Vertex u;
	Vertex *temp = calloc(g->nr_vertices, sizeof(*temp));

	for (u = vertex_root(); idx < g->nr_edges; u = vertex_next(u)) {
		size_t count = 0;
		Edge adj;

		/* Retrieving adjacency list */
		for (adj = g->first[u]; adj != 0; adj = g->next[adj]) {
			temp[count++] = g->vertex[adj];
		}

		/* Sorting */
		qsort(temp, count, sizeof(Vertex), cmp_vertex);

		/* Putting adjacency list back */
		count = 0;
		for (adj = g->first[u]; adj != 0; adj = g->next[adj]) {
			g->vertex[adj] = temp[count++];
		}
		idx += count;
	}

	free(temp);
}

void graph_print(Graph *g)
{
	int idx = 0;
	Vertex u;
	printf("%d\n%d\n", g->nr_vertices, g->nr_edges);

	for (u = vertex_root(); idx < g->nr_edges; u = vertex_next(u)) {
		Edge adj;
		for (adj = g->first[u]; adj != 0; adj = g->next[adj]) {
			Vertex v = g->vertex[adj];
			printf("%d %d\n", u, v);
			idx++;
		}
	}
}

/****************** Finding Strongly Connected Components *********************/
/* Apply Tarjan's algorithm to find SCCs */
void graph_SCC_find_aux(
	Graph *g, Graph *scc,
	Vertex *head,
	Stack *st, Stack *st_temp,
	Vertex u,
	int *disc, int *low, int *disc_time
) {
	Edge adj;
	disc[u] = low[u] = ++(*disc_time);
	stack_push(st, u);

	for (adj = g->first[u]; adj != 0; adj = g->next[adj]) {
		Vertex v = g->vertex[adj];

		/* If v is not visited yet, recur for it */
		if (disc[v] == 0) {
			graph_SCC_find_aux(g, scc, head, st, st_temp, v, disc, low, disc_time);
			low[u] = min(low[u], low[v]);
		}
		/* Update low value of 'u' only if 'v' is still in stack */
		else if (stack_contains(st, v)) {
			low[u] = min(low[u], disc[v]);
		}
	}

	/* head node found; so it's an SCC. Popping stack until we reach head node. */
	if (low[u] == disc[u]) {
		Vertex v, root = u;
		scc->nr_vertices++;

		stack_push(st_temp, u);
		while ((v = stack_pop(st)) != u) {
			root = min(root, v);
			stack_push(st_temp, v);
		}
		head[u] = root;
		while ((v = stack_pop(st_temp)) != u) {
			head[v] = root;
		}
	}
}

void graph_SCC_find(Graph *g, Graph *scc)
{
	int *disc = calloc(g->nr_vertices+1, sizeof(*disc));
	int *low  = calloc(g->nr_vertices+1, sizeof(*low));
	Vertex *head = calloc(g->nr_vertices+1, sizeof(*head));
	int disc_time = 0;
	Vertex u;
	Stack st, st_temp;

	/* Initializing data */
	stack_new(&st, g->nr_vertices);
	stack_new(&st_temp, g->nr_vertices);
	scc->nr_vertices = 0;

	/* Performing first DFS with Tarjan */
	for (u = vertex_root(); vertex_end(g, u); u = vertex_next(u)) {
        if (disc[u] == 0) {
            graph_SCC_find_aux(g, scc, head, &st, &st_temp, u, disc, low, &disc_time);
		}
	}

	/* Scouring through Graph for SCC connections */
	for (u = vertex_root(); vertex_end(g, u); u = vertex_next(u)) {
		Edge adj;
		for (adj = g->first[u]; adj != 0; adj = g->next[adj]) {
			Vertex v = g->vertex[adj];

			/* Found connection between SCCs */
			if (head[u] != head[v]) {
				graph_connect(scc, head[u], head[v]);
			}
		}
	}

	/* Freeing data */
	stack_destroy(&st);
	stack_destroy(&st_temp);
	free(disc);
	free(low);
	free(head);
}

/***************************** MAIN function **********************************/
int main(void) {
	int num_v, num_e;
	Graph g, scc;

	/* Grabbing input */
	get_number(&num_v); /* Grabbing number of vertices */
	get_number(&num_e); /* Grabbing number of edges */

	/* Instancing graphs */
	graph_new(&g, num_v, num_e);
	graph_new(&scc, num_v, num_e);

	graph_init(&g, num_e); /* Initializing Graph from input */

	/* Apply this project's magic */
	graph_SCC_find(&g, &scc);
	graph_sort(&scc);
	graph_print(&scc);

	/* Freeing data */
	graph_destroy(&scc);
	graph_destroy(&g);

	return 0;
}
