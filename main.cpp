#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <ctime>
#include <map>
#include <set>
#include <omp.h>
#include <sstream>

using namespace std;

vector<vector<int>> generateGraph(long long numOfVertices, long long numOfEdges);
// Генерирует граф по его параметрам (Количество вершин и рёбер)

vector<vector<int>> generateGraph(long long numOfVertices);
// Генерирует граф только по количеству вершин, беря за количество рёбер треть от количества рёбер в полном графе

vector<vector<vector<int>>> getSubgraphs(const vector<vector<int>> &graph, long long int numOfEdges);
// Возвращает все подграфы из переданного графа

vector<vector<int>> getSubgraph(const vector<vector<int>> &graph, long long first, long long numOfEdges);
// Возвращает случайный подграф с заданным числом вершин

bool searchSubgraph(const vector<vector<vector<int>>>& allSubgraphs, const vector<vector<int>>& neededSubgraphs);
// Возвращает true если искомый подграф существует

bool compareSubgraph(vector<vector<int>> subgraph1, vector<vector<int>> subgraph2);
// Сравнивает 2 графа

vector<vector<int>> gs(const vector<vector<int>> &graph, long long first, long long numOfEdges);
// Рекурсивно проходит по вершинам графа, возвращает свой путь (Используется для getSubgraph)

vector<vector<int>> validSubgraph(const vector<vector<int>> &graphs);
// Проверяет соответствие графа нужному количеству рёбер

vector<vector<int>> normalize(vector<int> &g);
// Приводит путь возвращенный рекурсивной функцией в классический вид подграфа

void generateDot(const vector<vector<int>> &graph, const string &filename);
// Генерирует .dot файл для визуализации графа

int main() {
    long long NUM_OF_VERTICES = 12;
    long long NUM_OF_EDGES = 5;

    vector<vector<int>> graph = generateGraph(NUM_OF_VERTICES);
    generateDot(graph, "graph.dot");
    vector<vector<int>> subgraph = getSubgraph(graph, 1, NUM_OF_EDGES);
    generateDot(subgraph, "subgraph.dot");

    for (long long k = 8; k <= 28; ++k) {
        graph = generateGraph(k);
        subgraph = getSubgraph(graph, 1, NUM_OF_EDGES);

        double tm = omp_get_wtime();
        vector<vector<vector<int>>> subgraphs = getSubgraphs(graph, NUM_OF_EDGES);

        cout << k << ": "
             << (searchSubgraph(subgraphs, subgraph) ? "Subgraph was found" : "Subgraph not found")
             << " in " << ((double) ((long long) ((omp_get_wtime() - tm) * 1000))) / 1000 << endl;
    }

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

vector<vector<vector<int>>> getSubgraphs(const vector<vector<int>> &graph, long long int numOfEdges) {
    set<vector<vector<int>>> out;
    for (int i = 0; i < graph.size(); ++i) {
        vector<vector<int>> subgraphs = gs(graph, i, numOfEdges + 1);
        subgraphs = validSubgraph(subgraphs);
        if (!subgraphs.empty()) {
            for (auto g : subgraphs) {
                out.insert(normalize(g));
            }
        }
    }
    if (out.empty()) {
        return static_cast<vector<vector<vector<int>>>>(0);
    } else {
        vector<vector<vector<int>>> v(out.begin(), out.end());
        return v;
    }
}

vector<vector<int>> getSubgraph(const vector<vector<int>> &graph, long long int first, long long int numOfEdges) {
    random_device rd;
    mt19937 gen(rd() + (unsigned) time(nullptr));
    vector<vector<int>> subgraphs = gs(graph, first, numOfEdges + 1);
    subgraphs = validSubgraph(subgraphs);
    if (!subgraphs.empty()) {
        vector<int> g = subgraphs[gen() % subgraphs.size()];
        vector<vector<int>> subgraph = normalize(g);

        return subgraph;
    } else {
        return static_cast<vector<vector<int>>>(0);
    }
}

vector<vector<int>> normalize(vector<int> &g) {
    vector<vector<int>> subgraph;
    map<int, int> m;

    for (int &i : g) {
        if (m.count(i) == 0) {
            int sz = m.size();
            m.insert(pair<int, int>(i, sz));
        }
    }

    vector<int> ng(m.size());

    subgraph.reserve(m.size());
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

        for (auto &bufSubgraph : bufSubgraphs) {
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

bool searchSubgraph(const vector<vector<vector<int>>>& allSubgraphs, const vector<vector<int>>& neededSubgraph) {
    bool check = false;
    for (auto & allSubgraph : allSubgraphs)
        if (compareSubgraph(allSubgraph, neededSubgraph)) {
            check = true;
        }
    return check;
}

bool compareSubgraph(vector<vector<int>> subgraph1, vector<vector<int>> subgraph2) {
    bool check = true;
    vector<int> buf(max(subgraph1.size(), subgraph2.size()), 0);
    subgraph1.resize(max(subgraph1.size(), subgraph2.size()), buf);
    subgraph2.resize(max(subgraph1.size(), subgraph2.size()), buf);

    for (int i = 0; i < max(subgraph1.size(), subgraph2.size()); i++) {
        subgraph1[i].resize(max(subgraph1[i].size(), subgraph2[i].size()), 0);
        subgraph2[i].resize(max(subgraph1[i].size(), subgraph2[i].size()), 0);

        for (int ii = 0; ii < max(subgraph1[i].size(), subgraph2[i].size()); ii++) {
            if (subgraph1[i][ii] == 1) {
                if (subgraph2[i][ii] != 1) {
                    check = false;
                    return check;
                }
            }
        }
    }

    return check;
}

void generateDot(const vector<vector<int>> &graph, const string &filename) {
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
        } else {
            str[str.size() - 2] = ';';
            fileOut << str << endl;
        }
    }

    fileOut << '}';
}