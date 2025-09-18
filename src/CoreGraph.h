#ifndef CORE_GRAPH_H
#define CORE_GRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

struct User {
    int id;
    std::string name;
};

class CoreGraph {
public:
    CoreGraph();

    // Primary user operations
    int addUser(const std::string &name);                 // returns new auto-assigned id
    bool addUser(const std::string &name, int fixedId);  // add with fixed id (used by Persistence)
    bool removeUser(int id);
    bool userExists(int id) const;
    const User* getUser(int id) const;

    // Friendship operations
    bool addFriend(int a, int b);      // undirected
    bool removeFriend(int a, int b);

    // Accessors
    std::vector<int> getFriends(int id) const;                 // sorted
    std::vector<int> listAllUsers() const;                     // sorted ids
    std::unordered_map<int, std::unordered_set<int>> getAdjacency() const; // copy (for persistence)

    // Helpers
    void clear();                       // clear all users/edges
    void printUser(int id) const;

private:
    int nextId;
    std::unordered_map<int, User> users;
    std::unordered_map<int, std::unordered_set<int>> adj;
};

#endif // CORE_GRAPH_H
