#include "Persistence.h"
#include "CoreGraph.h"
#include <fstream>
#include <sstream>
#include <iostream>

Persistence::Persistence(CoreGraph *g) : graph(g) {
    rebuildNameIndex();
}

// Escape reserved characters
std::string Persistence::escape(const std::string &s) {
    std::string out;
    for (char c : s) {
        if (c == '|' || c == '\\' || c == ',') out.push_back('\\');
        out.push_back(c);
    }
    return out;
}

// Unescape reserved characters
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

// =============================================================
// SAVE TO FILE
// Format:
// USERS <count>
// id|name|interest1,interest2,...
// EDGES
// u v
// =============================================================
bool Persistence::saveToFile(const std::string &filename) {
    if (!graph) return false;
    std::ofstream ofs(filename);
    if (!ofs.is_open()) return false;

    auto ids = graph->listAllUsers();
    ofs << "USERS " << ids.size() << "\n";
    for (int id : ids) {
        const User* u = graph->getUser(id);
        if (!u) continue;
        ofs << id << "|" << escape(u->name) << "|";

        // Save interests (comma-separated)
        bool first = true;
        for (auto &intr : u->interests) {
            if (!first) ofs << ",";
            ofs << escape(intr);
            first = false;
        }
        ofs << "\n";
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

// =============================================================
// LOAD FROM FILE
// Compatible with old files (without interests)
// =============================================================
bool Persistence::loadFromFile(const std::string &filename) {
    if (!graph) return false;
    std::ifstream ifs(filename);
    if (!ifs.is_open()) return false;

    graph->clear();
    std::string line;

    // USERS section
    if (!std::getline(ifs, line)) return false;
    std::stringstream ss(line);
    std::string tag;
    int userCount = 0;
    ss >> tag >> userCount;
    if (tag != "USERS") return false;

    for (int i = 0; i < userCount; ++i) {
        if (!std::getline(ifs, line)) return false;

        size_t pos1 = line.find('|');
        if (pos1 == std::string::npos) return false;
        size_t pos2 = line.find('|', pos1 + 1);

        int id = std::stoi(line.substr(0, pos1));
        std::string name;
        std::string interestStr;

        // Handle both old and new formats
        if (pos2 == std::string::npos) {
            name = unescape(line.substr(pos1 + 1));
        } else {
            name = unescape(line.substr(pos1 + 1, pos2 - pos1 - 1));
            interestStr = line.substr(pos2 + 1);
        }

        graph->addUser(name, id);

        // Add interests (if available)
        if (!interestStr.empty()) {
            std::stringstream ss2(interestStr);
            std::string intr;
            while (std::getline(ss2, intr, ',')) {
                if (!intr.empty()) graph->addInterest(id, unescape(intr));
            }
        }
    }

    // EDGES section
    if (!std::getline(ifs, line)) return false;
    if (line != "EDGES") return false;

    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        std::stringstream ss2(line);
        int u, v;
        if (ss2 >> u >> v) graph->addFriend(u, v);
    }

    rebuildNameIndex();
    return true;
}

// =============================================================
// Rebuild in-memory name index
// =============================================================
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

// =============================================================
// Lookup user ID by name
// =============================================================
int Persistence::findUserIdByName(const std::string &name) {
    auto it = nameIndex.find(name);
    if (it == nameIndex.end()) return -1;
    return it->second;
}