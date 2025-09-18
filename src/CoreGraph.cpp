#include "CoreGraph.h"
#include <algorithm>
#include <iostream>

CoreGraph::CoreGraph() : nextId(1) {}

int CoreGraph::addUser(const std::string &name) {
    int id = nextId++;
    users[id] = User{id, name};
    if (adj.find(id) == adj.end()) adj[id] = {};
    return id;
}

bool CoreGraph::addUser(const std::string &name, int fixedId) {
    if (fixedId <= 0) return false;
    if (users.find(fixedId) != users.end()) return false; // already present
    users[fixedId] = User{fixedId, name};
    if (adj.find(fixedId) == adj.end()) adj[fixedId] = {};
    if (fixedId >= nextId) nextId = fixedId + 1;
    return true;
}

bool CoreGraph::removeUser(int id) {
    if (users.find(id) == users.end()) return false;
    // remove id from neighbors
    if (adj.find(id) != adj.end()) {
        for (int v : adj[id]) {
            adj[v].erase(id);
        }
        adj.erase(id);
    }
    users.erase(id);
    return true;
}

bool CoreGraph::userExists(int id) const {
    return users.find(id) != users.end();
}

const User* CoreGraph::getUser(int id) const {
    auto it = users.find(id);
    if (it == users.end()) return nullptr;
    return &it->second;
}

bool CoreGraph::addFriend(int a, int b) {
    if (a == b) return false;
    if (!userExists(a) || !userExists(b)) return false;
    bool insertedA = adj[a].insert(b).second;
    bool insertedB = adj[b].insert(a).second;
    // ensure map entries exist even if no insertion done
    if (adj.find(a) == adj.end()) adj[a] = {};
    if (adj.find(b) == adj.end()) adj[b] = {};
    return insertedA || insertedB;
}

bool CoreGraph::removeFriend(int a, int b) {
    if (!userExists(a) || !userExists(b)) return false;
    size_t ra = 0, rb = 0;
    if (adj.find(a) != adj.end()) ra = adj[a].erase(b);
    if (adj.find(b) != adj.end()) rb = adj[b].erase(a);
    return (ra > 0 || rb > 0);
}

std::vector<int> CoreGraph::getFriends(int id) const {
    std::vector<int> res;
    auto it = adj.find(id);
    if (it == adj.end()) return res;
    res.insert(res.end(), it->second.begin(), it->second.end());
    std::sort(res.begin(), res.end());
    return res;
}

std::vector<int> CoreGraph::listAllUsers() const {
    std::vector<int> res;
    res.reserve(users.size());
    for (auto &p : users) res.push_back(p.first);
    std::sort(res.begin(), res.end());
    return res;
}

std::unordered_map<int, std::unordered_set<int>> CoreGraph::getAdjacency() const {
    return adj; // copy
}

void CoreGraph::clear() {
    users.clear();
    adj.clear();
    nextId = 1;
}

void CoreGraph::printUser(int id) const {
    const User* u = getUser(id);
    if (!u) {
        std::cout << "User not found\n";
        return;
    }
    std::cout << "User(" << u->id << ", " << u->name << ") Friends: ";
    auto f = getFriends(id);
    for (int fid : f) std::cout << fid << " ";
    std::cout << "\n";
}
