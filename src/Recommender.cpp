#include "Recommender.h"
#include "CoreGraph.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cmath>
#include <algorithm>
#include <iostream>

// Constructor
Recommender::Recommender(const CoreGraph *graph) : G(graph) {}


// =============================================================
// 1️⃣ Basic Recommendation Based on Mutual Friends
// =============================================================
std::vector<std::pair<int,int>> Recommender::recommendByMutual(int userId, int topK) const {
    std::vector<std::pair<int,int>> empty;
    if (!G || !G->getUser(userId)) return empty;

    // Step 1: Collect all current friends
    std::unordered_set<int> friendsSet(G->getFriends(userId).begin(), G->getFriends(userId).end());

    // Step 2: Count mutual friends for each candidate
    std::unordered_map<int,int> mutualCount;
    auto adj = G->getAdjacency();

    for (int f : friendsSet) {
        auto it = adj.find(f);
        if (it == adj.end()) continue;

        for (int fof : it->second) {
            if (fof == userId) continue;
            if (friendsSet.find(fof) != friendsSet.end()) continue;
            mutualCount[fof] += 1; // count mutual friend
        }
    }

    if (mutualCount.empty()) return empty;

    // Step 3: Rank candidates by number of mutual friends
    std::priority_queue<std::pair<int,int>> pq;
    for (auto &p : mutualCount) pq.push({p.second, -p.first});

    // Step 4: Pick Top-K recommendations
    std::vector<std::pair<int,int>> result;
    while (!pq.empty() && (int)result.size() < topK) {
        auto t = pq.top(); pq.pop();
        int score = t.first;
        int cand = -t.second;
        result.push_back({cand, score});
    }

    return result;
}


// =============================================================
// 2️⃣ Helper Function — Jaccard Similarity for Interests
// =============================================================
static double jaccardSimilarity(const std::unordered_set<std::string> &A,
                                const std::unordered_set<std::string> &B) {
    if (A.empty() || B.empty()) return 0.0;

    int common = 0;
    for (auto &x : A) if (B.find(x) != B.end()) common++;

    return (double)common / (A.size() + B.size() - common);
}


// =============================================================
// 3️⃣ Enhanced Weighted Recommendation (Mutual + Interests)
// =============================================================
std::vector<std::pair<int,double>> Recommender::recommendWeighted(int userId, int topK,
    const std::function<double(int,int)> &weightFn) const {

    std::vector<std::pair<int,double>> empty;
    if (!G || !G->getUser(userId)) return empty;

    // Step 1: Gather user’s current friends
    std::unordered_set<int> friendsSet(G->getFriends(userId).begin(), G->getFriends(userId).end());

    // Step 2: Compute mutual friend count
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

    // Step 3: Combine with interest similarity
    const User *mainUser = G->getUser(userId);
    std::vector<std::pair<int,double>> scored;

    for (auto &p : mutualCount) {
        int cand = p.first;
        int mc = p.second;

        const User *candUser = G->getUser(cand);
        if (!candUser) continue;

        double interestSim = jaccardSimilarity(mainUser->interests, candUser->interests);

        // Weighted score formula
        // α = 1.0 for mutual count
        // β = 2.0 for interest similarity
        double score = (1.0 * mc) + (2.0 * interestSim);

        scored.push_back({cand, score});
    }

    // Step 4: Sort descending by score
    std::sort(scored.begin(), scored.end(), [](const auto &a, const auto &b) {
        if (fabs(a.second - b.second) > 1e-9) return a.second > b.second;
        return a.first < b.first;
    });

    if ((int)scored.size() > topK) scored.resize(topK);
    return scored;
}