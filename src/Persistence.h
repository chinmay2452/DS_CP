#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <string>
#include <unordered_map>

class CoreGraph; // forward

class Persistence {
public:
    Persistence(CoreGraph *graph);

    bool saveToFile(const std::string &filename);
    bool loadFromFile(const std::string &filename);

    void rebuildNameIndex();
    int findUserIdByName(const std::string &name); // -1 if not found

private:
    CoreGraph *graph;
    std::unordered_map<std::string,int> nameIndex;

    std::string escape(const std::string &s);
    std::string unescape(const std::string &s);
};

#endif // PERSISTENCE_H
