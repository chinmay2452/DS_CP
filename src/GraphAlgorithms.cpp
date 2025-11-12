#include "GraphAlgorithms.h"
#include "CoreGraph.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>

GraphAlgorithms::GraphAlgorithms(CoreGraph *graph) : G(graph) {}


// =============================================================
// 1️⃣ Shortest Path (Breadth-First Search)
// =============================================================
// Finds the minimum friendship path between two users.
std::vector<int> GraphAlgorithms::shortestPath(int src, int dst) {
    std::vector<int> empty;
    if (!G || !G->getUser(src) || !G->getUser(dst)) return empty;

    std::unordered_map<int, int> parent;
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

    // Reconstruct the path
    std::vector<int> path;
    for (int cur = dst; cur != -1; cur = parent[cur]) path.push_back(cur);
    std::reverse(path.begin(), path.end());
    return path;
}


// =============================================================
// 2️⃣ Connected Components (Community Detection)
// =============================================================
// Groups users into disconnected friendship communities.
std::vector<std::vector<int>> GraphAlgorithms::connectedComponents() {
    std::vector<std::vector<int>> comps;
    if (!G) return comps;

    std::unordered_set<int> seen;
    auto adj = G->getAdjacency();

    for (int u : G->listAllUsers()) {
        if (seen.count(u)) continue;

        std::vector<int> comp;
        std::queue<int> q;
        q.push(u);
        seen.insert(u);

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


// =============================================================
// 3️⃣ Influencer by Degree
// =============================================================
// Finds the user with the highest number of friends (degree centrality)
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


// =============================================================
// 4️⃣ Influencer by Interest Overlap (Bonus)
// =============================================================
// Finds the user who shares the most interests with others.
int GraphAlgorithms::influencerByInterestOverlap() {
    if (!G) return -1;

    auto users = G->listAllUsers();
    double bestScore = -1.0;
    int bestUser = -1;

    for (int u : users) {
        const User* U = G->getUser(u);
        if (!U) continue;

        double totalOverlap = 0.0;
        int count = 0;

        for (int v : users) {
            if (u == v) continue;
            const User* V = G->getUser(v);
            if (!V) continue;

            // Compute interest overlap (Jaccard)
            int common = 0;
            for (auto &i : U->interests) {
                if (V->interests.count(i)) common++;
            }
            if (!U->interests.empty() || !V->interests.empty()) {
                double overlap = (double)common / (U->interests.size() + V->interests.size() - common);
                totalOverlap += overlap;
                count++;
            }
        }

        double avgOverlap = (count > 0) ? totalOverlap / count : 0.0;
        if (avgOverlap > bestScore) {
            bestScore = avgOverlap;
            bestUser = u;
        }
    }

    return bestUser;
}