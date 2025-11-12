#ifndef GRAPH_ALGORITHMS_H
#define GRAPH_ALGORITHMS_H

#include <vector>

// Forward declaration to avoid circular include
class CoreGraph;

/**
 * @brief The GraphAlgorithms class implements
 * various graph-based computations for the social network.
 */
class GraphAlgorithms {
public:
    explicit GraphAlgorithms(CoreGraph *graph);

    // ----------------------------
    // Core Graph Operations
    // ----------------------------

    /**
     * @brief Finds the shortest path between two users (unweighted).
     * @param src Source user ID
     * @param dst Destination user ID
     * @return List of user IDs representing the path
     */
    std::vector<int> shortestPath(int src, int dst);

    /**
     * @brief Finds all connected components (communities) in the network.
     * @return A vector of components (each component is a vector of user IDs)
     */
    std::vector<std::vector<int>> connectedComponents();

    /**
     * @brief Finds the user with the highest number of friends.
     * @return User ID of influencer, or -1 if no users
     */
    int influencerByDegree();

    /**
     * @brief Finds the user with the highest average interest overlap with others.
     * @return User ID of interest-based influencer, or -1 if no users
     */
    int influencerByInterestOverlap();

private:
    CoreGraph *G;  // Pointer to the main graph structure
};

#endif // GRAPH_ALGORITHMS_H