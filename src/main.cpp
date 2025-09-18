#include <iostream>
#include <sstream>
#include "CoreGraph.h"
#include "Persistence.h"
#include "Recommender.h"
#include "GraphAlgorithms.h"
#include "Tools.h"

void printMenu() {
    std::cout << "\n========== Social Network CLI ==========\n"
              << "1. Add User\n"
              << "2. Add Friend\n"
              << "3. Recommend Friends (Mutual)\n"
              << "4. Recommend Friends (Weighted)\n"
              << "5. Save Network\n"
              << "6. Load Network\n"
              << "7. Show Communities\n"
              << "8. Shortest Path\n"
              << "9. Export DOT File\n"
              << "10. Suggest Username by Prefix\n"
              << "11. Print User Info\n"
              << "12. Help (Show Commands)\n"
              << "0. Exit\n"
              << "========================================\n"
              << "Enter choice: ";
}

int main() {
    CoreGraph graph;
    Persistence persistence(&graph);
    Recommender recommender(&graph);
    GraphAlgorithms algos(&graph);
    Tools tools(&graph);

    int choice;
    std::cout << "Welcome to Social Network CLI!\n";

    while (true) {
        printMenu();
        std::cin >> choice;

        if (choice == 0) {
            std::cout << "Bye!\n";
            break;
        }

        switch (choice) {
            case 1: { // Add User
                std::string name;
                std::cout << "Enter username: ";
                std::cin >> name;
                int id = graph.addUser(name);
                persistence.rebuildNameIndex();
                tools.insertUsername(name, id);
                std::cout << "Added user " << name << " with ID " << id << "\n";
                break;
            }
            case 2: { // Add Friend
                int a, b;
                std::cout << "Enter two user IDs: ";
                std::cin >> a >> b;
                if (graph.addFriend(a, b)) std::cout << "Friendship added!\n";
                else std::cout << "Failed to add friendship.\n";
                break;
            }
            case 3: { // Recommend mutual
                int id, k;
                std::cout << "Enter user ID and top K: ";
                std::cin >> id >> k;
                auto recs = recommender.recommendByMutual(id, k);
                if (recs.empty()) std::cout << "No recommendations.\n";
                else for (auto &p : recs) std::cout << "User " << p.first << " (score=" << p.second << ")\n";
                break;
            }
            case 4: { // Recommend weighted
                int id, k;
                std::cout << "Enter user ID and top K: ";
                std::cin >> id >> k;
                auto weightFn = [&](int cand, int mutual)->double {
                    return mutual * (1.0 + 0.01 * cand);
                };
                auto recs = recommender.recommendWeighted(id, k, weightFn);
                if (recs.empty()) std::cout << "No recommendations.\n";
                else for (auto &p : recs) std::cout << "User " << p.first << " (score=" << p.second << ")\n";
                break;
            }
            case 5: { // Save
                std::string fn;
                std::cout << "Enter filename: ";
                std::cin >> fn;
                if (persistence.saveToFile(fn)) std::cout << "Saved to " << fn << "\n";
                else std::cout << "Save failed.\n";
                break;
            }
            case 6: { // Load
                std::string fn;
                std::cout << "Enter filename: ";
                std::cin >> fn;
                if (persistence.loadFromFile(fn)) {
                    persistence.rebuildNameIndex();
                    tools.rebuildTrieFromGraph();
                    std::cout << "Loaded " << fn << "\n";
                } else std::cout << "Load failed.\n";
                break;
            }
            case 7: { // Communities
                auto comps = algos.connectedComponents();
                for (size_t i = 0; i < comps.size(); ++i) {
                    std::cout << "Community " << i + 1 << ": ";
                    for (int id : comps[i]) std::cout << id << " ";
                    std::cout << "\n";
                }
                break;
            }
            case 8: { // Shortest Path
                int a, b;
                std::cout << "Enter source and destination IDs: ";
                std::cin >> a >> b;
                auto path = algos.shortestPath(a, b);
                if (path.empty()) std::cout << "No path found.\n";
                else {
                    for (size_t i = 0; i < path.size(); ++i) {
                        std::cout << path[i] << (i + 1 < path.size() ? " -> " : "\n");
                    }
                }
                break;
            }
            case 9: { // Export DOT
                std::string fn;
                std::cout << "Enter filename: ";
                std::cin >> fn;
                if (tools.exportToDot(fn)) std::cout << "DOT exported to " << fn << "\n";
                else std::cout << "Export failed.\n";
                break;
            }
            case 10: { // Suggest prefix
                std::string prefix; int k;
                std::cout << "Enter prefix and top K: ";
                std::cin >> prefix >> k;
                auto sug = tools.suggestByPrefix(prefix, k);
                if (sug.empty()) std::cout << "No suggestions.\n";
                else for (int id : sug) std::cout << id << " : " << graph.getUser(id)->name << "\n";
                break;
            }
            case 11: { // Print user
                int id;
                std::cout << "Enter user ID: ";
                std::cin >> id;
                graph.printUser(id);
                break;
            }
            case 12: { // Help
                printMenu();
                break;
            }
            default:
                std::cout << "Invalid choice! Try again.\n";
        }
    }

    return 0;
}