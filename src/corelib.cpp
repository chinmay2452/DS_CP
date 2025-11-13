// corelib.cpp
#include "corelib.hpp"

#include "CoreGraph.h"
#include "Persistence.h"
#include "Recommender.h"
#include "Tools.h"
#include "GraphAlgorithms.h"

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <iostream>

// Single global objects (one graph in memory)
static CoreGraph G;
static Persistence P(&G);
static Recommender R(&G);
static Tools T(&G);
static GraphAlgorithms A(&G);

// helper to strdup string for C ABI
static char* cstrdup(const std::string &s) {
    char *p = (char*)std::malloc(s.size() + 1);
    if (!p) return nullptr;
    std::memcpy(p, s.c_str(), s.size());
    p[s.size()] = '\0';
    return p;
}

// JSON helpers (very small helpers - build strings manually)
static std::string json_escape(const std::string &in) {
    std::string out;
    out.reserve(in.size() + 10);
    for (char c : in) {
        switch (c) {
            case '\"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(c);
        }
    }
    return out;
}

// ---------------- basic ops ----------------
extern "C" {

int api_add_user(const char* name) {
    if (!name) return -1;
    std::string sname(name);
    int id = G.addUser(sname);
    // keep tools and persistence indices updated
    P.rebuildNameIndex();
    T.insertUsername(sname, id);
    return id;
}

int api_add_user_with_id(const char* name, int fixedId) {
    if (!name) return -1;
    std::string sname(name);
    if (G.addUser(sname, fixedId)) {
        P.rebuildNameIndex();
        T.insertUsername(sname, fixedId);
        return fixedId;
    }
    return -1;
}

bool api_add_friend(int a, int b) {
    return G.addFriend(a, b);
}

bool api_remove_friend(int a, int b) {
    return G.removeFriend(a, b);
}

bool api_remove_user(int id) {
    bool ok = G.removeUser(id);
    if (ok) {
        P.rebuildNameIndex();
        T.rebuildTrieFromGraph();
    }
    return ok;
}

// ---------------- interests ----------------
bool api_add_interests(int id, const char* csv) {
    if (!csv) return false;
    // split by comma
    std::string s(csv);
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> arr;
    while (std::getline(ss, item, ',')) {
        // trim spaces
        size_t b = item.find_first_not_of(" \t");
        if (b == std::string::npos) continue;
        size_t e = item.find_last_not_of(" \t");
        std::string it = item.substr(b, e-b+1);
        G.addInterest(id, it);
    }
    return true;
}

char* api_get_user_interests(int id) {
    const User* u = G.getUser(id);
    if (!u) return cstrdup("null");
    std::string out = "[";
    bool first = true;
    for (auto &i : u->interests) {
        if (!first) out += ",";
        out += "\"" + json_escape(i) + "\"";
        first = false;
    }
    out += "]";
    return cstrdup(out);
}

// ---------------- queries / algorithms ----------------

char* api_list_all_users() {
    auto ids = G.listAllUsers();
    std::ostringstream oss;
    oss << "[";
    bool first = true;
    for (int id : ids) {
        const User* u = G.getUser(id);
        if (!u) continue;
        if (!first) oss << ",";
        oss << "{";
        oss << "\"id\":" << id << ",";
        oss << "\"name\":\"" << json_escape(u->name) << "\"";
        oss << "}";
        first = false;
    }
    oss << "]";
    return cstrdup(oss.str());
}

char* api_print_user_info(int id) {
    const User* u = G.getUser(id);
    if (!u) return cstrdup("null");
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":" << u->id << ",";
    oss << "\"name\":\"" << json_escape(u->name) << "\",";
    // friends
    auto fr = G.getFriends(id);
    oss << "\"friends\":[";
    for (size_t i=0;i<fr.size();++i) {
        if (i) oss << ",";
        oss << fr[i];
    }
    oss << "],";
    // interests
    oss << "\"interests\":[";
    bool first = true;
    for (auto &it : u->interests) {
        if (!first) oss << ",";
        oss << "\"" << json_escape(it) << "\"";
        first = false;
    }
    oss << "]";
    oss << "}";
    return cstrdup(oss.str());
}

char* api_recommend_mutual(int userId, int topK) {
    std::ostringstream oss;
    auto recs = R.recommendByMutual(userId, topK);
    oss << "[";
    bool first = true;
    for (auto &p : recs) {
        int cand = p.first;
        int score = p.second;
        const User* u = G.getUser(cand);
        if (!u) continue;
        if (!first) oss << ",";
        oss << "{";
        oss << "\"id\":" << cand << ",";
        oss << "\"name\":\"" << json_escape(u->name) << "\",";
        oss << "\"score\":" << score;
        oss << "}";
        first = false;
    }
    oss << "]";
    return cstrdup(oss.str());
}

char* api_recommend_weighted(int userId, int topK) {
    std::ostringstream oss;
    auto recs = R.recommendWeighted(userId, topK, nullptr);
    const User* target = G.getUser(userId);
    oss << "[";
    bool first = true;
    for (auto &p : recs) {
        int cand = p.first;
        double score = p.second;
        const User* u = G.getUser(cand);
        if (!u) continue;
        if (!first) oss << ",";
        oss << "{";
        oss << "\"id\":" << cand << ",";
        oss << "\"name\":\"" << json_escape(u->name) << "\",";
        oss << "\"score\":" << score << ",";
        // mutuals
        auto f1 = G.getFriends(userId);
        auto f2 = G.getFriends(cand);
        int mutuals = 0;
        for (int a : f1) for (int b : f2) if (a == b) ++mutuals;
        oss << "\"mutuals\":" << mutuals << ",";
        // shared interests
        oss << "\"shared_interests\":[";
        bool firstI = true;
        if (target) {
            for (auto &it: target->interests) {
                if (u->interests.count(it)) {
                    if (!firstI) oss << ",";
                    oss << "\"" << json_escape(it) << "\"";
                    firstI = false;
                }
            }
        }
        oss << "]";
        oss << "}";
        first = false;
    }
    oss << "]";
    return cstrdup(oss.str());
}

char* api_shortest_path(int src, int dst) {
    auto path = A.shortestPath(src, dst);
    std::ostringstream oss;
    oss << "{ \"path\": [";
    for (size_t i=0;i<path.size();++i) {
        if (i) oss << ",";
        oss << path[i];
    }
    oss << "] }";
    return cstrdup(oss.str());
}

char* api_connected_components() {
    auto comps = A.connectedComponents();
    std::ostringstream oss;
    oss << "[";
    for (size_t i=0;i<comps.size();++i) {
        if (i) oss << ",";
        oss << "[";
        for (size_t j=0;j<comps[i].size();++j) {
            if (j) oss << ",";
            oss << comps[i][j];
        }
        oss << "]";
    }
    oss << "]";
    return cstrdup(oss.str());
}

char* api_suggest_prefix(const char* prefix, int k) {
    if (!prefix) return cstrdup("[]");
    auto v = T.suggestByPrefix(std::string(prefix), k);
    std::ostringstream oss;
    oss << "[";
    bool first = true;
    for (int id : v) {
        const User* u = G.getUser(id);
        if (!u) continue;
        if (!first) oss << ",";
        oss << "{";
        oss << "\"id\":" << id << ",";
        oss << "\"name\":\"" << json_escape(u->name) << "\"";
        oss << "}";
        first = false;
    }
    oss << "]";
    return cstrdup(oss.str());
}

// ---------------- persistence ----------------
bool api_save_network(const char* filename) {
    if (!filename) return false;
    return P.saveToFile(std::string(filename));
}

bool api_load_network(const char* filename) {
    if (!filename) return false;
    bool ok = P.loadFromFile(std::string(filename));
    if (ok) {
        P.rebuildNameIndex();
        T.rebuildTrieFromGraph();
    }
    return ok;
}

// free helper
void api_free_string(char* s) {
    if (!s) return;
    std::free(s);
}

} // extern "C"