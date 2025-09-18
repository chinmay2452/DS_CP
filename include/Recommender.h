#ifndef RECOMMENDER_H
#define RECOMMENDER_H

#include <vector>
#include <functional>
#include <utility>

class CoreGraph; // forward

class Recommender {
public:
    explicit Recommender(const CoreGraph *graph);

    std::vector<std::pair<int,int>> recommendByMutual(int userId, int topK = 5) const;
    std::vector<std::pair<int,double>> recommendWeighted(int userId, int topK,
        const std::function<double(int,int)> &weightFn) const;

private:
    const CoreGraph *G;
};

#endif // RECOMMENDER_H
