#ifndef RECOMMENDER_H
#define RECOMMENDER_H

#include <vector>
#include <functional>
#include <utility>

/**
 * @class Recommender
 * @brief Suggests friend recommendations based on mutual connections and shared interests.
 *
 * This module implements two core recommendation strategies:
 * 1. Mutual-friend based recommendation
 * 2. Weighted recommendation that incorporates both mutual count and interest overlap
 */
class CoreGraph; // forward declaration

class Recommender {
public:
    /**
     * @brief Constructs the recommender using an existing CoreGraph instance.
     * @param graph Pointer to the CoreGraph (the main user network).
     */
    explicit Recommender(const CoreGraph *graph);

    /**
     * @brief Recommends top-K users based purely on mutual friends.
     * @param userId The user for whom recommendations are generated.
     * @param topK Number of top users to return (default = 5).
     * @return Vector of (userID, mutualCount) pairs sorted by descending score.
     */
    std::vector<std::pair<int, int>> recommendByMutual(int userId, int topK = 5) const;

    /**
     * @brief Recommends top-K users using a custom weighting function that
     * can consider mutual friends, interests, or other criteria.
     *
     * @param userId The user for whom recommendations are generated.
     * @param topK Number of top users to return.
     * @param weightFn A lambda or function that computes a score given (candidateID, mutualCount).
     * @return Vector of (userID, weightedScore) pairs sorted by descending score.
     *
     * Example usage:
     * ```cpp
     * auto weightFn = [&](int cand, int mutual) {
     *     double interestScore = computeInterestOverlap(userId, cand);
     *     return mutual * 0.7 + interestScore * 0.3;  // Combined weight
     * };
     * recommender.recommendWeighted(id, 5, weightFn);
     * ```
     */
    std::vector<std::pair<int, double>> recommendWeighted(
        int userId,
        int topK,
        const std::function<double(int, int)> &weightFn
    ) const;

private:
    const CoreGraph *G; ///< Pointer to the main user graph (read-only).
};

#endif // RECOMMENDER_H