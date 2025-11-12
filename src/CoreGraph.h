#ifndef CORE_GRAPH_H
#define CORE_GRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>

struct User
{
    int id;
    std::string name;
    std::unordered_set<std::string> interests; // user interests
};

class CoreGraph
{
public:
    CoreGraph();

    // ==============================
    //  User Operations
    // ==============================
    int addUser(const std::string &name);               // Auto-assign new ID
    bool addUser(const std::string &name, int fixedId); // Add user with fixed ID (Persistence)
    bool removeUser(int id);
    bool userExists(int id) const;
    const User *getUser(int id) const;

    // ==============================
    //  Friendship Operations
    // ==============================
    bool addFriend(int a, int b); // Add undirected friendship
    bool removeFriend(int a, int b);

    // ==============================
    //  Interest Operations
    // ==============================
    bool addInterest(int userId, const std::string &interest);                // Add one interest
    bool addInterests(int userId, const std::vector<std::string> &interests); // Add multiple
    std::unordered_set<std::string> getInterests(int userId) const;           // Get all interests
    void printInterests(int userId) const;                                    // Print interests

    // ==============================
    //  Accessors
    // ==============================
    std::vector<int> getFriends(int id) const;                             // Return friend IDs (sorted)
    std::vector<int> listAllUsers() const;                                 // Return all user IDs (sorted)
    std::unordered_map<int, std::unordered_set<int>> getAdjacency() const; // Return adjacency

    // ==============================
    //  Helpers
    // ==============================
    void clear();                 // Clear users + edges
    void printUser(int id) const; // Print user details

private:
    int nextId;
    std::unordered_map<int, User> users;
    std::unordered_map<int, std::unordered_set<int>> adj;

    std::string normalize(const std::string &s) const; // lowercase helper
};

#endif // CORE_GRAPH_H