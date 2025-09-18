#include "GraphAlgorithms.h"
#include "CoreGraph.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

GraphAlgorithms::GraphAlgorithms(CoreGraph *graph) : G(graph) {}

std::vector<int> GraphAlgorithms::shortestPath(int src, int dst) {
    std::vector<int> empty;
    if (!G) return empty;
    if (!G->getUser(src) || !G->getUser(dst)) return empty;

    std::unordered_map<int,int> parent;
    std::unordered_set<int> visited;
    std::queue<int> q;
    q.push(src);
    visited.insert(src);
    parent[src] = -1;

    bool found = false;
    auto adj = G->getAdjacency();
    while (!q.empty()) {
        int u = q.front(); q.pop();
        if (u == dst) { found = true; break; }
        auto it = adj.find(u);
        if (it == adj.end()) continue;
        for (int v : it->second) {
            if (!visited.count(v)) {
                visited.insert(v);
                parent[v] = u;
                q.push(v);
            }
        }
    }
    if (!found) return empty;
    std::vector<int> path;
    int cur = dst;
    while (cur != -1) {
        path.push_back(cur);
        cur = parent[cur];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<std::vector<int>> GraphAlgorithms::connectedComponents() {
    std::vector<std::vector<int>> comps;
    if (!G) return comps;
    std::unordered_set<int> seen;
    for (int u : G->listAllUsers()) {
        if (seen.count(u)) continue;
        std::vector<int> comp;
        std::queue<int> q;
        q.push(u);
        seen.insert(u);
        auto adj = G->getAdjacency();
        while (!q.empty()) {
            int x = q.front(); q.pop();
            comp.push_back(x);
            auto it = adj.find(x);
            if (it == adj.end()) continue;
            for (int v : it->second) {
                if (!seen.count(v)) {
                    seen.insert(v);
                    q.push(v);
                }
            }
        }
        std::sort(comp.begin(), comp.end());
        comps.push_back(comp);
    }
    return comps;
}

int GraphAlgorithms::influencerByDegree() {
    if (!G) return -1;
    auto adj = G->getAdjacency();
    int best = -1;
    size_t bestDeg = 0;
    for (auto &p : adj) {
        int u = p.first;
        size_t deg = p.second.size();
        if (deg > bestDeg || (deg == bestDeg && (best == -1 || u < best))) {
            bestDeg = deg;
            best = u;
        }
    }
    return best;
}
