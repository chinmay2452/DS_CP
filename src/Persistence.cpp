#include "Persistence.h"
#include "CoreGraph.h"
#include <fstream>
#include <sstream>

Persistence::Persistence(CoreGraph *g) : graph(g) {
    rebuildNameIndex();
}

std::string Persistence::escape(const std::string &s) {
    std::string out;
    for (char c : s) {
        if (c == '|' || c == '\\') out.push_back('\\');
        out.push_back(c);
    }
    return out;
}

std::string Persistence::unescape(const std::string &s) {
    std::string out;
    bool esc = false;
    for (char c : s) {
        if (esc) { out.push_back(c); esc = false; }
        else if (c == '\\') esc = true;
        else out.push_back(c);
    }
    return out;
}

bool Persistence::saveToFile(const std::string &filename) {
    if (!graph) return false;
    std::ofstream ofs(filename);
    if (!ofs.is_open()) return false;

    auto ids = graph->listAllUsers();
    ofs << "USERS " << ids.size() << "\n";
    for (int id : ids) {
        const User* u = graph->getUser(id);
        if (!u) continue;
        ofs << id << "|" << escape(u->name) << "\n";
    }
    ofs << "EDGES\n";
    auto adj = graph->getAdjacency();
    for (auto &kv : adj) {
        int u = kv.first;
        for (int v : kv.second) {
            if (u < v) ofs << u << " " << v << "\n";
        }
    }
    ofs.close();
    return true;
}

bool Persistence::loadFromFile(const std::string &filename) {
    if (!graph) return false;
    std::ifstream ifs(filename);
    if (!ifs.is_open()) return false;

    graph->clear();
    std::string line;
    if (!std::getline(ifs, line)) return false;
    std::stringstream ss(line);
    std::string tag;
    int userCount = 0;
    ss >> tag >> userCount;
    if (tag != "USERS") return false;

    for (int i = 0; i < userCount; ++i) {
        if (!std::getline(ifs, line)) return false;
        size_t pos = line.find('|');
        if (pos == std::string::npos) return false;
        int id = std::stoi(line.substr(0, pos));
        std::string name = unescape(line.substr(pos + 1));
        graph->addUser(name, id);
    }

    if (!std::getline(ifs, line)) return false;
    if (line != "EDGES") return false;

    while (std::getline(ifs, line)) {
        if (line.size() == 0) continue;
        std::stringstream ss2(line);
        int u, v;
        if (ss2 >> u >> v) {
            graph->addFriend(u, v);
        }
    }

    rebuildNameIndex();
    return true;
}

void Persistence::rebuildNameIndex() {
    nameIndex.clear();
    if (!graph) return;
    auto ids = graph->listAllUsers();
    for (int id : ids) {
        const User* u = graph->getUser(id);
        if (!u) continue;
        nameIndex[u->name] = id;
    }
}

int Persistence::findUserIdByName(const std::string &name) {
    auto it = nameIndex.find(name);
    if (it == nameIndex.end()) return -1;
    return it->second;
}
