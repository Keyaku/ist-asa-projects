#include <iostream>
#include <vector>
#include <list>
#include <stack>

#include <stdio.h>

using namespace std;

class Graph {
	bool is_bidir;
	list<int> *adj;

	vector<int> dist;
	vector<bool> visited;
	vector<int> order;

public:
	int nr_vertices, nr_edges;
	int longest_path, nr_paths;

	Graph(bool is_bidir);
	~Graph() {
		delete[] adj;
	}
	void Connect(int u, int v);
	void DFS();
	void DP();
	void TopologicalSort();

	/* stdin overloader; builds the Graph */
	friend istream& operator>>(istream &is, Graph &g) {
		/* scanf() is faster than >> */
		scanf("%d %d", &g.nr_vertices, &g.nr_edges);
		g.adj = new list<int>[g.nr_vertices+1];

		/* Create a vector to store indegrees of all vertices */
		g.visited = vector<bool>(g.nr_vertices+1, false);
		g.dist = vector<int>(g.nr_vertices+1, 0);

		for (int i = 0; i < g.nr_edges; i++) {
			int u, v;
			scanf("%d %d", &u, &v);
			g.Connect(u, v);
		}

		return is;
	}

	/* stdout overloader; prints the Graph */
	friend ostream& operator<<(ostream &os, const Graph &g) {
		os << g.nr_vertices << " " << g.nr_edges << endl;
		for (int u = 1; u <= g.nr_vertices; u++) {
			os << u;
			for (list<int>::const_iterator it = g.adj[u].begin(); it != g.adj[u].end(); ++it) {
				int v = *it;
				os << " " << v;
			}
			os << endl;
		}
		return os;
	}
};

Graph::Graph(bool is_bidirectional)
{
	this->is_bidir = is_bidirectional;
	this->longest_path = this->nr_paths = 0;
}

void Graph::Connect(int u, int v)
{
	adj[u].push_back(v);
	if (this->is_bidir) {
		adj[v].push_back(u);
	}
}

void Graph::DP()
{
	/* order is supposed to be a stack, iterate it from last to first items */
	for (size_t idx = order.size()-1; idx > 0; idx--) {
	// for (size_t idx = 0; idx < order.size(); idx++) {
		int u = order[idx];

		for (list<int>::iterator it = adj[u].begin(); it != adj[u].end(); it++) {
			int v = *it;
			dist[v] = dist[u] + 1;
		}
	}
}

void Graph::DFS()
{
	/* Applying Topological sort while obtaining number of paths (indegree == 0) */
	vector<bool> is_last(nr_vertices+1, false);
	stack<int> dfs;

	for (int src = 1; src <= nr_vertices; src++) {
		if (!visited[src]) {
			dfs.push(src);
			is_last[src] = false;
		}

		while (!dfs.empty()) {
			int u = dfs.top();
			dfs.pop();

			if (is_last[u]) {
				order.push_back(u);
				continue;
			}
			visited[u] = true;
			dfs.push(u);
			is_last[u] = true;

			for (list<int>::iterator it = adj[u].begin(); it != adj[u].end(); it++) {
				int v = *it;
				if (!visited[v]) {
					dfs.push(v);
					is_last[v] = false;
				}
			}
		}
	}
}

void Graph::TopologicalSort()
{
	DFS();
	DP();

	for (int u = 1; u <= nr_vertices; u++) {
		if (dist[u] == 0) { nr_paths++; }
		longest_path = max(longest_path, dist[u]+1);
	}
}

int main()
{
	/* Instancing main graph */
	Graph g(false);

	/* Grabbing input */
	cin >> g;

	/* Apply this project's magic */
	g.TopologicalSort();

	/* Outputing solution */
	cout << g.nr_paths << " " << g.longest_path << endl;

	return 0;
}
