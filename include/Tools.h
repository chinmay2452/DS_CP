#ifndef TOOLS_H
#define TOOLS_H

#include <string>
#include <vector>
#include <unordered_map>

class CoreGraph;

class Tools {
public:
    explicit Tools(CoreGraph *graph);
    ~Tools();

    // Trie-based autocomplete
    void insertUsername(const std::string &name, int userId);
    std::vector<int> suggestByPrefix(const std::string &prefix, int k=5);

    // Export to Graphviz DOT
    bool exportToDot(const std::string &filename);

    // Rebuild trie from current graph
    void rebuildTrieFromGraph();

private:
    struct TrieNode {
        std::unordered_map<char, TrieNode*> next;
        std::vector<int> ids;
        bool end;
        TrieNode(): end(false) {}
    };
    TrieNode *root;
    CoreGraph *G;
    std::unordered_map<int,std::string> idToName;
    void freeTrie(TrieNode *n);
};

#endif // TOOLS_H
