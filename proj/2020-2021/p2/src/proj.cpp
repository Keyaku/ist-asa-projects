/* Solution by Tomás Zaki; improvements by me */
#include <iostream>
#include <queue>

#include <stdio.h>
#include <limits.h>
#include <string.h>

using namespace std;

#define SOURCE 0
#define SINK (N+1)


struct Edge {
	int u, v;
	int residual;

	Edge() {
		u = v = residual = 0;
	}
	Edge(int i, int j, int cap) {
		u = i;
		v = j;
		residual = cap;
	}

	bool operator!=(const Edge &e) const {
		return !(*this == e);
	}
	bool operator==(const Edge &e) const {
		return u == e.u && v == e.v;
	}
};


class Graph {
	/* Graph composition, represented as an Edge array */
	int *first;    /* first[Vertex idx] = Edge idx  */
	int *next;     /* next[Edge idx]    = Edge idx   */
	int *prev;     /* prev[Edge idx]    = Edge idx   */
	Edge *edge;    /* edge[Edge idx] = Edge */

public:
	int nr_vertices, nr_edges;
	int N, K;

	Graph() {}
	~Graph() {
		delete[] first;
		delete[] next;
		delete[] prev;
		delete[] edge;
	}
	bool Connect(int u, int v, int weight, bool is_bidir=true);

	bool BFS(int parent[]);
	int GetMaxFlow();

	/* stdin overloader; builds the Graph */
	friend istream& operator>>(istream &is, Graph &g) {
		/* scanf() is faster than >> */
		scanf("%d %d", &g.N, &g.K);

		g.nr_vertices = g.N+2;
		g.nr_edges = 0;
		int num_e = 2*g.N + g.K;
		num_e = (num_e << 1) + 1; /* doubling due to reverse edges */

		/* counting source & sink vertices */
		g.first  = new int[g.nr_vertices];
		g.next   = new int[num_e];
		g.prev   = new int[num_e];
		g.edge   = new Edge[num_e];

		for (int idx = 0; idx < g.nr_vertices; idx++) {
			g.first[idx] = 0;
		}
		for (int idx = 0; idx < num_e; idx++) {
			g.next[idx] = 0;
			g.prev[idx] = 0;
			g.edge[idx] = Edge();
		}

		/* Connecting vertices to source & sink */
		for (int u = 1; u <= g.N; u++) {
			int w_xu, w_uy;
			scanf("%d %d", &w_xu, &w_uy);
			g.Connect(SOURCE, u, w_xu);
			g.Connect(u, g.N+1, w_uy);
		}

		/* Connecting vertices between each other */
		for (int idx = 0; idx < g.K; idx++) {
			int u, v, w;
			scanf("%d %d %d", &u, &v, &w);
			g.Connect(u, v, w);
		}

		return is;
	}

	/* stdout overloader; prints the Graph */
	friend ostream& operator<<(ostream &os, const Graph &g) {
		os << g.nr_vertices << " " << g.nr_edges << endl;
		for (int u = 0; u < g.nr_vertices; u++) {
			if (u == SOURCE) {
				os << "S";
			} else if (u == g.nr_vertices-1) {
				os << "T";
			} else {
				os << u;
			}
			for (int adj = g.first[u]; adj != 0; adj = g.next[adj]) {
				Edge e = g.edge[adj];
				if (e.v == SOURCE) {
					os << "S";
				} else if (e.v == g.nr_vertices-1) {
					os << "T";
				} else {
					os << e.v;
				}
			}
			os << endl;
		}
		return os;
	}
};

bool Graph::Connect(int u, int v, int weight, bool is_bidir)
{
	nr_edges++;

	if (first[u] == 0) {
		first[u] = nr_edges;
	} else {
		int adj;
		for (adj = first[u]; next[adj] != 0 && edge[adj].v != v; adj = next[adj]);
		/* if Vertex v is already in here, stop everything */
		if (edge[adj].v == v) {
			nr_edges--;
			return false;
		}
		next[adj] = nr_edges;
	}
	edge[nr_edges] = Edge(u, v, weight);

	if (is_bidir) {
		if (Connect(v, u, weight, false)) {
			prev[nr_edges] = nr_edges-1;
		}
	}

	return true;
}

/* Algorithm */

/* Returns true if there is a path from source to sink
  in residual graph. Also fills parent[] to store the path
*/
bool Graph::BFS(int parent[])
{
	/* Create a visited array and mark all vertices as not visited */
	bool visited[nr_vertices];
	memset(visited, false, sizeof(visited));

	/* Create a queue, enqueue source and mark it as visited */
	queue<int> q;
	q.push(SOURCE);
	visited[SOURCE] = true;
	parent[SOURCE] = -1;

	/* Standard BFS Loop */
	while (!q.empty()) {
		int u = q.front();
		q.pop();

		for (int adj = first[u]; adj != 0; adj = next[adj]) {
			Edge e = edge[adj];
			int v = e.v;

			if (!visited[v] && e.residual > 0) {
				/* If we find a connection to the sink node,
				then there is no point in BFS anymore We
				just have to set its parent and can return true
				*/
				parent[v] = adj;
				if (v == SINK) {
					return true;
				}
				q.push(v);
				visited[v] = true;
			}
		}
	}

	/* We didn't reach sink in BFS starting from source, so return false */
	return false;
}

/* Returns the maximum flow from s to t in the given graph */
int Graph::GetMaxFlow()
{
	int max_flow = 0; /* There is no flow initially */

	/* This array is filled by BFS and to store path */
	int parent[nr_vertices];

	/* Augment the flow while there is path from source to sink */
	while (BFS(parent)) {
		/* Find minimum residual capacity of the edges along
		the path filled by BFS. Or we can say find the
		maximum flow through the path found.
		*/
		int path_flow = INT_MAX;

		for (int v = SINK; v != SOURCE;) {
			int adj = parent[v];
			Edge& e = edge[adj];
			v = e.u;

			path_flow = min(path_flow, e.residual);
		}

		/* update residual capacities of the edges and
		reverse edges along the path
		*/
		for (int v = SINK; v != SOURCE;) {
			int adj = parent[v];
			Edge& e = edge[adj];
			Edge& e_rev = edge[prev[adj]];
			v = e.u;

			e.residual -= path_flow;
			e_rev.residual += path_flow;
		}

		/* Add path flow to overall flow */
		max_flow += path_flow;
	}

	/* Return the overall flow */
	return max_flow;
}

/* MAIN */
int main()
{
	/* Instancing main graph */
	Graph g;

	/* Grabbing input */
	cin >> g;

	/* Apply this project's magic */
	cout << g.GetMaxFlow() << endl;

	return 0;
}
