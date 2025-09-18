#ifndef GRAPH_ALGORITHMS_H
#define GRAPH_ALGORITHMS_H

#include <vector>

class CoreGraph;

class GraphAlgorithms {
public:
    explicit GraphAlgorithms(CoreGraph *graph);

    std::vector<int> shortestPath(int src, int dst); // BFS unweighted
    std::vector<std::vector<int>> connectedComponents();
    int influencerByDegree(); // id or -1

private:
    CoreGraph *G;
};

#endif // GRAPH_ALGORITHMS_H
