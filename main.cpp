#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <ctime>
#include <map>
#include <set>

using namespace std;

vector<vector<int>> generateGraph(long long numOfVertices, long long numOfEdges);
vector<vector<int>> generateGraph(long long numOfVertices);
void generateDot(const vector<vector<int>> &graph, const string& filename);
vector<vector<int>> getSubgraph(const vector<vector<int>> &graph, long long first, long long numOfEdges);
vector<vector<int>> gs(const vector<vector<int>> &graph, long long first, long long numOfEdges);
vector<vector<int>> validSubgraph(const vector<vector<int>> &graphs);

int main() {
    const long long NUM_OF_VERTICES = 10;
    vector<vector<int>> graph = generateGraph(NUM_OF_VERTICES);
    generateDot(graph, "graph.dot");
    vector<vector<int>> subgraph = getSubgraph(graph, 1, 10);
    generateDot(subgraph, "subgraph.dot");
    return 0;
}

vector<vector<int>> generateGraph(long long numOfVertices, long long numOfEdges) {
    random_device rd;
    mt19937 gen(rd() + (unsigned) time(nullptr));
    vector<vector<int>> graph;

    vector<int> g;

    for (long long i = 0; i < numOfVertices; ++i) {
        g.push_back(0);
    }

    for (long long i = 0; i < numOfVertices; ++i) {
        graph.push_back(g);
    }

    for (long long i = 0; i < numOfEdges; ++i) {
        tryAgain:
        long long currVert = gen() % numOfVertices;
        long long currEdge = gen() % numOfVertices;

        if (graph[currVert][currEdge] == 0) {
            graph[currVert][currEdge] = 1;
            graph[currEdge][currVert] = 1;
        } else {
            goto tryAgain;
        }
    }

    return graph;
}

vector<vector<int>> generateGraph(long long numOfVertices) {
    long long numOfEdges = (numOfVertices * (numOfVertices - 1)) / 6;
    return generateGraph(numOfVertices, numOfEdges);
}

void generateDot(const vector<vector<int>> &graph, const string& filename) {
    ofstream fileOut(filename);
    fileOut << "graph {\n";
    long long len = graph.size();

    for (long long i = 0; i < len; ++i) {
        string str = "    " + to_string(i) + " -- ";

        for (long long ii = i; ii < len; ++ii) {
            if (graph[i][ii]) {
                str += std::to_string(ii) + ", ";
            }
        }

        if (str[str.size() - 2] == '-') {
            for (int j = 0; j < 4; ++j)
                str.pop_back();
            fileOut << str << ";\n";
        } else {
            str[str.size() - 2] = ';';
            fileOut << str << endl;
        }
    }

    fileOut << '}';
}

vector<vector<int>> getSubgraph(const vector<vector<int>> &graph, long long int first, long long int numOfEdges) {
    random_device rd;
    mt19937 gen(rd() + (unsigned) time(nullptr));
    numOfEdges++;
    vector<vector<int>> subgraph, subgraphs = gs(graph, first, numOfEdges);
    subgraphs = validSubgraph(subgraphs);
    if (!subgraphs.empty()) {
        vector<int> g = subgraphs[gen() % subgraphs.size()];
        map<int, int> m;

        for (int &i : g) {
            if (m.count(i) == 0) {
                int sz = m.size();
                m.insert(pair<int, int>(i, sz));
            }
        }

        vector<int> ng(m.size());

        for (int i = 0; i < m.size(); ++i) {
            subgraph.push_back(ng);
        }

        for (int &i : g) {
            i = m[i];
        }

        for (int i = 0; i < g.size() - 1; i++) {
            subgraph[g[i]][g[i + 1]] = 1;
            subgraph[g[i + 1]][g[i]] = 1;
        }

        return subgraph;
    } else {
        return static_cast<vector<vector<int>>>(0);
    }
}

vector<vector<int>> gs(const vector<vector<int>> &graph, long long int first, long long int numOfEdges) {
    vector<vector<int>> subgraphs, bufSubgraphs;

    if (numOfEdges == 1) {
        vector<int> g;
        g.push_back(first);
        subgraphs.push_back(g);
    } else {
        for (int i = 0; i < graph.size(); ++i) {
            if (graph[first][i]) {
                bufSubgraphs = gs(graph, i, numOfEdges - 1);
                subgraphs.insert(subgraphs.end(), bufSubgraphs.begin(), bufSubgraphs.end());
            }
        }

        bufSubgraphs = subgraphs;
        subgraphs.clear();

        for (auto & bufSubgraph : bufSubgraphs) {
            vector<int> buf;
            buf.push_back(first);
            buf.insert(buf.end(), bufSubgraph.begin(), bufSubgraph.end());
            subgraphs.push_back(buf);
        }
    }
    return subgraphs;
}

vector<vector<int>> validSubgraph(const vector<vector<int>> &graphs) {
    vector<vector<int>> validatedGraphs;

    for (auto curr : graphs) {
        set<string> s;

        for (int i = 0; i < curr.size() - 1; ++i) {
            s.insert(to_string(curr[i]) + '-' + to_string(curr[i + 1]));
            s.insert(to_string(curr[i + 1]) + '-' + to_string(curr[i]));
        }

        if (s.size() > (curr.size() - 2) * 2) {
            validatedGraphs.push_back(curr);
        }
    }

    return validatedGraphs;
}
