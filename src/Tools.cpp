#include "Tools.h"
#include "CoreGraph.h"
#include <fstream>
#include <algorithm>

Tools::Tools(CoreGraph *graph) : root(nullptr), G(graph) {
    root = new TrieNode();
    rebuildTrieFromGraph();
}

Tools::~Tools() {
    freeTrie(root);
}

void Tools::freeTrie(TrieNode *n) {
    if (!n) return;
    for (auto &p : n->next) freeTrie(p.second);
    delete n;
}

void Tools::insertUsername(const std::string &name, int userId) {
    TrieNode *cur = root;
    for (char c : name) {
        if (!cur->next.count(c)) cur->next[c] = new TrieNode();
        cur = cur->next[c];
        cur->ids.push_back(userId);
    }
    cur->end = true;
    idToName[userId] = name;
}

std::vector<int> Tools::suggestByPrefix(const std::string &prefix, int k) {
    std::vector<int> res;
    TrieNode *cur = root;
    for (char c : prefix) {
        if (!cur->next.count(c)) return res;
        cur = cur->next[c];
    }
    // unique and sort by name lexicographically
    std::unordered_set<int> seen;
    for (int id : cur->ids) seen.insert(id);
    std::vector<int> candidates(seen.begin(), seen.end());
    std::sort(candidates.begin(), candidates.end(), [&](int a, int b){
        return idToName[a] < idToName[b];
    });
    if ((int)candidates.size() > k) candidates.resize(k);
    return candidates;
}

bool Tools::exportToDot(const std::string &filename) {
    if (!G) return false;
    std::ofstream ofs(filename);
    if (!ofs.is_open()) return false;
    ofs << "graph SocialNetwork {\n";
    for (int id : G->listAllUsers()) {
        const User* u = G->getUser(id);
        if (!u) continue;
        std::string label = u->name;
        // escape quotes
        for (char &c : label) if (c == '"') c = '\'';
        ofs << "  " << id << " [label=\"" << label << "\"];\n";
    }
    auto adj = G->getAdjacency();
    for (auto &kv : adj) {
        int u = kv.first;
        for (int v : kv.second) {
            if (u < v) ofs << "  " << u << " -- " << v << ";\n";
        }
    }
    ofs << "}\n";
    ofs.close();
    return true;
}

void Tools::rebuildTrieFromGraph() {
    freeTrie(root);
    root = new TrieNode();
    idToName.clear();
    if (!G) return;
    for (int id : G->listAllUsers()) {
        const User* u = G->getUser(id);
        if (!u) continue;
        insertUsername(u->name, id);
    }
}
