#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <string>
#include <unordered_map>

/**
 * @brief Forward declaration of CoreGraph to avoid circular dependency.
 */
class CoreGraph;

/**
 * @class Persistence
 * @brief Handles saving and loading of the social graph (users, friendships, and interests).
 *
 * The Persistence module ensures all graph data can be stored to disk
 * and reloaded later, including user names, IDs, and their interests.
 * The file format remains backward-compatible with earlier (name-only) datasets.
 */
class Persistence {
public:
    /**
     * @brief Constructs a persistence manager bound to the main graph.
     * @param graph Pointer to the CoreGraph instance.
     */
    explicit Persistence(CoreGraph *graph);

    /**
     * @brief Saves all users, their interests, and friendships to a file.
     * @param filename File path to save to (e.g., "users.txt").
     * @return true if successful, false otherwise.
     */
    bool saveToFile(const std::string &filename);

    /**
     * @brief Loads users, their interests, and friendships from a file.
     * @param filename File path to load from (e.g., "users.txt").
     * @return true if successful, false otherwise.
     */
    bool loadFromFile(const std::string &filename);

    /**
     * @brief Rebuilds the name-to-ID index from the graph data.
     */
    void rebuildNameIndex();

    /**
     * @brief Finds a user's ID by name.
     * @param name User’s name.
     * @return User ID if found, -1 otherwise.
     */
    int findUserIdByName(const std::string &name);

private:
    CoreGraph *graph;  ///< Pointer to the main social graph.
    std::unordered_map<std::string, int> nameIndex;  ///< Name → ID lookup map.

    /**
     * @brief Escapes reserved characters in file output.
     * Used for names and interests containing special symbols.
     */
    std::string escape(const std::string &s);

    /**
     * @brief Unescapes strings when loading from file.
     */
    std::string unescape(const std::string &s);
};

#endif // PERSISTENCE_H