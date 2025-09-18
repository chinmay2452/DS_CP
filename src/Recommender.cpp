#include "Recommender.h"
#include "CoreGraph.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cmath>
#include <algorithm>
#include <iostream>

Recommender::Recommender(const CoreGraph *graph) : G(graph) {}

std::vector<std::pair<int,int>> Recommender::recommendByMutual(int userId, int topK) const {
    std::vector<std::pair<int,int>> empty;
    if (!G) return empty;
    if (!G->getUser(userId)) return empty;

    std::unordered_set<int> friendsSet;
    for (int f : G->getFriends(userId)) friendsSet.insert(f);

    std::unordered_map<int,int> mutualCount;
    auto adj = G->getAdjacency();
    for (int f : friendsSet) {
        auto it = adj.find(f);
        if (it == adj.end()) continue;
        for (int fof : it->second) {
            if (fof == userId) continue;
            if (friendsSet.find(fof) != friendsSet.end()) continue;
            mutualCount[fof] += 1;
        }
    }
    if (mutualCount.empty()) return empty;

    std::priority_queue<std::pair<int,int>> pq;
    for (auto &p : mutualCount) pq.push({p.second, -p.first});

    std::vector<std::pair<int,int>> result;
    while (!pq.empty() && (int)result.size() < topK) {
        auto t = pq.top(); pq.pop();
        int score = t.first;
        int cand = -t.second;
        result.push_back({cand, score});
    }
    return result;
}

std::vector<std::pair<int,double>> Recommender::recommendWeighted(int userId, int topK,
    const std::function<double(int,int)> &weightFn) const {

    std::vector<std::pair<int,double>> empty;
    if (!G) return empty;
    if (!G->getUser(userId)) return empty;

    std::unordered_set<int> friendsSet;
    for (int f : G->getFriends(userId)) friendsSet.insert(f);

    std::unordered_map<int,int> mutualCount;
    auto adj = G->getAdjacency();
    for (int f : friendsSet) {
        auto it = adj.find(f);
        if (it == adj.end()) continue;
        for (int fof : it->second) {
            if (fof == userId) continue;
            if (friendsSet.find(fof) != friendsSet.end()) continue;
            mutualCount[fof] += 1;
        }
    }
    if (mutualCount.empty()) return empty;

    std::vector<std::pair<int,double>> scored;
    scored.reserve(mutualCount.size());
    for (auto &p : mutualCount) {
        int cand = p.first;
        int mc = p.second;
        double sc = weightFn ? weightFn(cand, mc) : (double)mc;
        scored.push_back({cand, sc});
    }
    std::sort(scored.begin(), scored.end(), [](const auto &a, const auto &b){
        if (fabs(a.second - b.second) > 1e-9) return a.second > b.second;
        return a.first < b.first;
    });
    if ((int)scored.size() > topK) scored.resize(topK);
    return scored;
}
