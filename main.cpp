#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <ctime>
#include <map>
#include <set>
#include <omp.h>
#include <sstream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

vector<vector<int>> generateGraph(long long numOfVertices, long long numOfEdges);
// Генерирует граф по его параметрам (Количество вершин и рёбер)

vector<vector<int>> generateGraph(long long numOfVertices);
// Генерирует граф только по количеству вершин, беря за количество рёбер треть от количества рёбер в полном графе

vector<vector<vector<int>>> getSubgraphs(const vector<vector<int>> &graph, long long int numOfEdges);
void *getSubgraphsMulti(void *args);
// Возвращает все подграфы из переданного графа

vector<vector<int>> getSubgraph(const vector<vector<int>> &graph, long long first, long long numOfEdges);
// Возвращает случайный подграф с заданным числом вершин

bool searchSubgraph(vector<vector<vector<int>>> allSubgraphs, vector<vector<int>> neededSubgraphs);
void *searchSubgraphMulti(void *args);
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

vector<vector<vector<int>>> allSubgraph;
set<vector<vector<int>>> out;
bool multiSearchCheck = false;

const int NUM_OF_TREADS = 4;
sem_t q;

typedef struct someArgs_tag // создание структуры для передачи интервала для сортировки в массив
{
    int left, right, numOfEdges;
    vector<vector<int>> graph;
} someArgs_t;

typedef struct someArgsSearch_tag // создание структуры для передачи интервала для сортировки в массив
{
    int left, right;
    vector<vector<vector<int>>> allSubgraphs;
    vector<vector<int>> neededSubgraphs;
} someArgsSearch_t;

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
    someArgs_t args[NUM_OF_TREADS];
    pthread_t thread[NUM_OF_TREADS];// "создание" потоков
    sem_init(&q, 0, 1);

    for (int i = 0; i < NUM_OF_TREADS; i++) {
        args[i].left = i * (graph.size() / NUM_OF_TREADS);

        if (i == NUM_OF_TREADS - 1) {
            args[i].right = graph.size();
        } else {
            args[i].right = (i + 1) * (graph.size() / NUM_OF_TREADS);
        }

        args[i].graph = graph;
        args[i].numOfEdges = numOfEdges;
        pthread_create(&thread[i], nullptr, getSubgraphsMulti,
                       (void *) &args[i]); // передача в поток функции на выполнение и интервала "сортировки"
    }
    for (auto &i : thread) {
        pthread_join(i, nullptr);// запуск потоков
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

bool searchSubgraph(vector<vector<vector<int>>> allSubgraphs, vector<vector<int>> neededSubgraphs) {
    bool check = false;
    someArgsSearch_t args[NUM_OF_TREADS];
    pthread_t thread[NUM_OF_TREADS];// "создание" потоков
    sem_init(&q, 0, 1);
    for (int i = 0; i < NUM_OF_TREADS; i++) {
        args[i].left = i * (allSubgraphs.size() / NUM_OF_TREADS);
        if (i == NUM_OF_TREADS - 1)
            args[i].right = allSubgraphs.size();
        else
            args[i].right = (i + 1) * (allSubgraphs.size() / NUM_OF_TREADS);
        args[i].allSubgraphs = allSubgraphs;
        args[i].neededSubgraphs = neededSubgraphs;
        pthread_create(&thread[i], nullptr, searchSubgraphMulti,
                       (void *) &args[i]); // передача в поток функции на выполнение и интервала "сортировки"
    }
    for (auto &i : thread) {
        pthread_join(i, nullptr); // запуск потоков
    }
    for (auto & allSubgraph : allSubgraphs)
        if (compareSubgraph(allSubgraph, neededSubgraphs)) {
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

void *getSubgraphsMulti(void *args) {
    someArgs_t *arg = (someArgs_t *) args;
    //vector<int> bufCount(N); //создание буферных массивов для каждого потока
    for (int i = arg->left; i < arg->right; i++) {
        vector<vector<int>> subgraphs = gs(arg->graph, i, arg->numOfEdges + 1);
        subgraphs = validSubgraph(subgraphs);

        sem_wait(&q);//если семафор открыт, то он закрывается, и поток подолжает выполнять код
        //если закрыт то поток ждёт
        for (auto g : subgraphs) {
            out.insert(normalize(g));
        }
        sem_post(&q);//открытие семафора
    }
    return nullptr;
}

void *searchSubgraphMulti(void *args) {
    someArgsSearch_t *arg = (someArgsSearch_t *) args;
    for (int i = arg->left; i < arg->right; i++) {
        if (compareSubgraph(arg->allSubgraphs[i], arg->neededSubgraphs)) {
            sem_wait(&q);//если семафор открыт, то он закрывается, и поток подолжает выполнять код
            //если закрыт то поток ждёт
            multiSearchCheck = true;
            sem_post(&q);//открытие семафора
        }
    }
    return nullptr;
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
            //fileOut << str << ";\n";
        } else {
            str[str.size() - 2] = ';';
            fileOut << str << endl;
        }
    }

    fileOut << '}';
}