#include <iostream>
#include <sstream>
#include "CoreGraph.h"
#include "Persistence.h"
#include "Recommender.h"
#include "GraphAlgorithms.h"
#include "Tools.h"

void printMenu()
{
    std::cout << "\n========== Social Network CLI ==========\n"
              << "1. Add User\n"
              << "2. Add Friend\n"
              << "3. Add Interests\n"
              << "4. Show User Interests\n"
              << "5. Remove User\n"
              << "6. Remove Friend\n"
              << "7. Recommend Friends (Mutual)\n"
              << "8. Recommend Friends (Weighted)\n"
              << "9. Save Network\n"
              << "10. Load Network\n"
              << "11. Show Communities\n"
              << "12. Shortest Path\n"
              << "13. Export DOT File\n"
              << "14. Suggest Username by Prefix\n"
              << "15. Print User Info\n"
              << "16. List All Users\n"
              << "0. Exit\n"
              << "========================================\n"
              << "Enter choice: ";
}

int main()
{
    CoreGraph graph;
    Persistence persistence(&graph);
    Recommender recommender(&graph);
    GraphAlgorithms algos(&graph);
    Tools tools(&graph);

    int choice;
    std::cout << "Welcome to FriendFinder Social Network CLI!\n";

    while (true)
    {
        printMenu();
        std::cin >> choice;

        if (choice == 0)
        {
            std::cout << "Bye!\n";
            break;
        }

        switch (choice)
        {
        // ---------------------------------------------
        case 1: // Add User
        {
            std::string name;
            std::cout << "Enter username: ";
            std::cin >> name;

            int id = graph.addUser(name);
            persistence.rebuildNameIndex();
            tools.insertUsername(name, id);

            std::cout << "Added user " << name << " with ID " << id << "\n";
            break;
        }

        // ---------------------------------------------
        case 2: // Add Friend
        {
            int a, b;
            std::cout << "Enter two user IDs: ";
            std::cin >> a >> b;

            if (graph.addFriend(a, b))
                std::cout << "Friendship added successfully!\n";
            else
                std::cout << "Failed to add friendship.\n";

            break;
        }

        // ---------------------------------------------
        case 3: // Add Interests
        {
            int id;
            std::cout << "Enter user ID: ";
            std::cin >> id;
            std::cin.ignore();

            std::string line;
            std::cout << "Enter comma-separated interests (e.g. AI, coding, music): ";
            std::getline(std::cin, line);

            std::stringstream ss(line);
            std::string interest;
            std::vector<std::string> interests;

            while (std::getline(ss, interest, ','))
            {
                interest.erase(0, interest.find_first_not_of(" \t"));
                interest.erase(interest.find_last_not_of(" \t") + 1);
                interests.push_back(interest);
            }

            graph.addInterests(id, interests);
            std::cout << "Interests added successfully.\n";
            break;
        }

        // ---------------------------------------------
        case 4: // Show Interests
        {
            int id;
            std::cout << "Enter user ID: ";
            std::cin >> id;
            graph.printInterests(id);
            break;
        }

        // ---------------------------------------------
        case 5: // Remove User
        {
            int id;
            std::cout << "Enter user ID to remove: ";
            std::cin >> id;

            if (graph.removeUser(id))
                std::cout << "User removed successfully.\n";
            else
                std::cout << "User not found.\n";

            break;
        }

        // ---------------------------------------------
        case 6: // Remove Friend
        {
            int a, b;
            std::cout << "Enter two user IDs to remove friendship: ";
            std::cin >> a >> b;

            if (graph.removeFriend(a, b))
                std::cout << "Friendship removed.\n";
            else
                std::cout << "Users not found or already not friends.\n";

            break;
        }

        // ---------------------------------------------
        case 7: // Recommend (Mutual)
        {
            int id, k;
            std::cout << "Enter user ID and top K: ";
            std::cin >> id >> k;

            auto recs = recommender.recommendByMutual(id, k);

            if (recs.empty())
                std::cout << "No recommendations found.\n";
            else
            {
                std::cout << "\nTop " << k << " Recommendations (Mutual-based):\n";
                for (auto &p : recs)
                    std::cout << "User " << p.first << " (mutuals=" << p.second << ")\n";
            }

            break;
        }

        // ---------------------------------------------
        case 8: // Recommend (Weighted)
        {
            int id, k;
            std::cout << "Enter user ID and top K: ";
            std::cin >> id >> k;

            auto recs = recommender.recommendWeighted(id, k, nullptr);

            if (recs.empty())
                std::cout << "No recommendations found.\n";
            else
            {
                std::cout << "Top Recommendations for " << graph.getUser(id)->name << ":\n";

                for (auto &p : recs)
                {
                    int candId = p.first;
                    const User *target = graph.getUser(id);
                    const User *cand = graph.getUser(candId);

                    if (!cand || !target) continue;

                    // Count mutuals
                    auto f1 = graph.getFriends(id);
                    auto f2 = graph.getFriends(candId);
                    int mutuals = 0;
                    for (int a : f1)
                        for (int b : f2)
                            if (a == b) mutuals++;

                    // Shared interests
                    std::vector<std::string> shared;
                    for (auto &i : target->interests)
                        if (cand->interests.count(i))
                            shared.push_back(i);

                    std::cout << candId << " (";
                    bool printed = false;

                    if (mutuals > 0)
                    {
                        std::cout << "mutuals: " << mutuals;
                        printed = true;
                    }

                    if (!shared.empty())
                    {
                        if (printed) std::cout << ", ";
                        std::cout << "shared interests: ";
                        for (size_t i = 0; i < shared.size(); ++i)
                        {
                            std::cout << shared[i];
                            if (i + 1 < shared.size()) std::cout << ", ";
                        }
                    }

                    std::cout << ")\n";
                }
            }

            break;
        }

        // ---------------------------------------------
        case 9: // Save Network
        {
            std::string fn;
            std::cout << "Enter filename: ";
            std::cin >> fn;

            if (persistence.saveToFile(fn))
                std::cout << "Saved to " << fn << "\n";
            else
                std::cout << "Save failed.\n";
            break;
        }

        // ---------------------------------------------
        case 10: // Load Network
        {
            std::string fn;
            std::cout << "Enter filename: ";
            std::cin >> fn;

            if (persistence.loadFromFile(fn))
            {
                persistence.rebuildNameIndex();
                tools.rebuildTrieFromGraph();
                std::cout << "Loaded " << fn << " successfully.\n";
            }
            else
                std::cout << "Load failed.\n";

            break;
        }

        // ---------------------------------------------
        case 11: // Communities
        {
            auto comps = algos.connectedComponents();
            for (size_t i = 0; i < comps.size(); ++i)
            {
                std::cout << "Community " << i + 1 << ": ";
                for (int id : comps[i])
                    std::cout << id << " ";
                std::cout << "\n";
            }
            break;
        }

        // ---------------------------------------------
        case 12: // Shortest Path
        {
            int a, b;
            std::cout << "Enter source and destination IDs: ";
            std::cin >> a >> b;

            auto path = algos.shortestPath(a, b);

            if (path.empty())
                std::cout << "No path found.\n";
            else
            {
                std::cout << "Shortest Path: ";
                for (size_t i = 0; i < path.size(); ++i)
                    std::cout << path[i] << (i + 1 < path.size() ? " -> " : "\n");
            }
            break;
        }

        // ---------------------------------------------
        case 13: // Export DOT
        {
            std::string fn;
            std::cout << "Enter filename: ";
            std::cin >> fn;

            if (tools.exportToDot(fn))
                std::cout << "DOT exported to " << fn << "\n";
            else
                std::cout << "Export failed.\n";

            break;
        }

        // ---------------------------------------------
        case 14: // Suggest Username Prefix
        {
            std::string prefix;
            int k;

            std::cout << "Enter prefix and top K: ";
            std::cin >> prefix >> k;

            auto sug = tools.suggestByPrefix(prefix, k);

            if (sug.empty())
                std::cout << "No suggestions.\n";
            else
                for (int id : sug)
                    std::cout << id << " : " << graph.getUser(id)->name << "\n";

            break;
        }

        // ---------------------------------------------
        case 15: // Print User Info
        {
            int id;
            std::cout << "Enter user ID: ";
            std::cin >> id;

            graph.printUser(id);
            graph.printInterests(id);

            break;
        }

        // ---------------------------------------------
        case 16: // List All Users
        {
            auto all = graph.listAllUsers();

            if (all.empty())
            {
                std::cout << "No users in the network.\n";
                break;
            }

            std::cout << "All Users:\n";
            for (int id : all)
            {
                const User* u = graph.getUser(id);
                if (u)
                    std::cout << id << " -> " << u->name << "\n";
            }

            break;
        }

        // ---------------------------------------------
        default:
            std::cout << "Invalid choice! Please try again.\n";
        }
    }

    return 0;
}