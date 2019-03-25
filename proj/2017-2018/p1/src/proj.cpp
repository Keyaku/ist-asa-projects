/******************************* ASA - Grupo 3 *********************************
** António Sarmento - 77906
** Marta Simões     - 81947
*******************************************************************************/

/* C++ Libraries. Only the bare minimum, no need for clutter */
#include <iostream>
#include <list>
#include <stack>
#include <algorithm>

/*************************** Auxiliary functions ******************************/
#define get_number(a)     std::cin >> a
#define get_numbers(a, b) std::cin >> a >> b
#define max(a, b) (a < b ? b : a)
#define min(a, b) (a > b ? b : a)

/******************** Data structures and their "methods" *********************/

/* Vertex Structure */
typedef size_t Vertex;
#define vertex_new(a)  a
#define vertex_next(a) a + 1
#define vertex_root(a) 1
#define vertex_end(GRAPH, a)  a <= GRAPH->nr_vertices

class Stack {
	private:
		std::stack<Vertex> _stack;
		bool *_in_stack;

	public:
		Stack(size_t size) { _in_stack = new bool[size]; };
		~Stack() { delete [] _in_stack; };
		bool contains(Vertex u) { return _in_stack[u]; }
		void push(const Vertex& u) {
			_stack.push(u);
			_in_stack[u] = true;
		};
		Vertex pop() {
			Vertex u = _stack.top();
			_stack.pop();
			_in_stack[u] = false;
			return u;
		};
};

class Graph {
	private:
		size_t nr_vertices, nr_edges;
		bool *in_graph;

		void SCC_find_aux(Vertex *head, Stack &s, Vertex u, int *disc, int *low, int &disc_time);

	public:
		/* Members */
		std::list<Vertex> *adj; /* Adjacency list of Vertices */

		/* Important */
		Graph(size_t num_v); /* Constructor */
		void init(size_t num_e);
		~Graph(); /* Destructor */

		/* Overloaders */
		friend std::ostream &operator<<(std::ostream &output, const Graph &g);

		/* Methods */
		void sort();
		bool has_connection(Vertex u, Vertex v);
		void connect(Vertex u, Vertex v);
		void SCC_find(Graph *scc);
};

Graph::Graph(size_t num_v)
{
	this->nr_vertices = this->nr_edges = 0;
	this->in_graph = new bool[num_v+1];
	this->adj = new std::list<Vertex>[num_v+1];
}
void Graph::init(size_t num_e)
{
	while (num_e-- > 0) {
		int num1, num2;
		Vertex u, v;

		/* Grab two numbers from input and convert them to vertices */
		get_numbers(num1, num2);
		u = vertex_new(num1); v = vertex_new(num2);

		/* Connect them and add them to the graph */
		this->connect(u, v);
	}
}
Graph::~Graph()
{
	delete [] this->in_graph;
	delete [] this->adj;
}

std::ostream &operator<<(std::ostream &output, const Graph &g)
{
	/* Printing number of vertices and edges */
	output << g.nr_vertices << std::endl << g.nr_edges << std::endl;

	/* Printing connections */
	size_t idx = 0;
	for (Vertex u = vertex_root(); idx < g.nr_edges; u = vertex_next(u)) {
		std::list<Vertex>::iterator it;
		for (it = g.adj[u].begin(); it != g.adj[u].end(); ++it) {
			Vertex v = *it;
			output << u << " " << v << std::endl;
			idx++;
		}
	}
	return output;
}

bool Graph::has_connection(Vertex u, Vertex v)
{
	/* Search for an already established u-v connection */
	std::list<Vertex>::iterator it = std::find(this->adj[u].begin(), this->adj[u].end(), v);
	return it != this->adj[u].end();
}

void Graph::connect(Vertex u, Vertex v)
{
	/* See if our Vertex is in our Graph */
	if (!this->in_graph[u]) {
		this->nr_vertices++;
		this->in_graph[u] = true;
	}
	if (!this->in_graph[v]) {
		this->nr_vertices++;
		this->in_graph[v] = true;
	}
	/* Search for an already established u-v connection */
	if (!this->has_connection(u, v)) {
		this->adj[u].push_back(v);
		this->nr_edges++;
	}
}

void Graph::sort()
{
	size_t idx = 0;
	for (Vertex u = vertex_root(); idx < this->nr_edges; u = vertex_next(u)) {
		this->adj[u].sort();
		idx+=this->adj[u].size();
	}
}

/****************** Finding Strongly Connected Components *********************/
/* Apply Tarjan's algorithm to find SCCs */
void Graph::SCC_find_aux(Vertex *head, Stack &st, Vertex u, int *disc, int *low, int &disc_time)
{
	disc[u] = low[u] = ++disc_time;
	st.push(u);

	std::list<Vertex>::iterator it;
	for (it = adj[u].begin(); it != adj[u].end(); ++it) {
		Vertex v = *it;

		/* If v is not visited yet, recur for it */
		if (disc[v] == 0) {
			this->SCC_find_aux(head, st, v, disc, low, disc_time);
			low[u] = min(low[u], low[v]);
		}
		/* Update low value of 'u' only if 'v' is still in stack */
		else if (st.contains(v)) {
			low[u] = min(low[u], disc[v]);
		}
	}

	/* head node found, so it's an SCC. Popping stack until we reach head node */
	if (low[u] == disc[u]) {
		Stack st_temp(this->nr_vertices+1);
		Vertex v, root = u;

		st_temp.push(u);
		while ((v = st.pop()) != u) {
			root = min(root, v);
			st_temp.push(v);
		}
		head[u] = root;
		while ((v = st_temp.pop()) != u) {
			head[v] = root;
		}
	}
}

void Graph::SCC_find(Graph *scc)
{
	int *disc = new int[this->nr_vertices+1];
	int *low  = new int[this->nr_vertices+1];
	Vertex *head = new Vertex[this->nr_vertices+1];
	int disc_time = 0;
	Stack tarjan(this->nr_vertices+1);

	/* Initializing data */
    for (size_t idx = 0; idx <= this->nr_vertices; idx++) {
		disc[idx] = 0;
		low[idx]  = 0;
		head[idx] = 0;
    }

	/* Performing first DFS with Tarjan */
	for (Vertex u = vertex_root(); vertex_end(this, u); u = vertex_next(u)) {
		if (disc[u] == 0) {
			this->SCC_find_aux(head, tarjan, u, disc, low, disc_time);
		}
	}

	/* Scouring through Graph for SCC connections */
	for (Vertex u = vertex_root(); vertex_end(this, u); u = vertex_next(u)) {
		std::list<Vertex>::iterator it;
		for (it = adj[u].begin(); it != adj[u].end(); ++it) {
			Vertex v = *it;

			/* Found connection between SCCs */
			if (head[u] != head[v]) {
				scc->connect(head[u], head[v]);
			}
		}
	}

	/* Correcting number of vertices */
	scc->nr_vertices = max(scc->nr_vertices, 1);

	/* Freeing data */
	delete [] disc;
	delete [] low;
	delete [] head;
}

/***************************** MAIN function **********************************/
int main(void) {
	size_t num_v, num_e;

	/* Grabbing input */
	get_number(num_v); /* Grabbing number of vertices */
	get_number(num_e); /* Grabbing number of edges */

	/* Initializing graphs */
	Graph g(num_v), scc(num_v);
	g.init(num_e);

	/* Apply this project's magic */
	g.SCC_find(&scc);
	scc.sort();
	std::cout << scc;

	return 0;
}
