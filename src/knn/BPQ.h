#ifndef KNN_BPQ_H_
#define KNN_BPQ_H_

#include "../knn/KnnProcessor.h"
#include "../knn/Metrics.h"

#include <cstddef>
#include <functional>
#include <limits>
#include <queue>

class BPQ {
private:
	std::size_t max_size_;
	double max_distance_;
	PointAccessor* query_;
	std::function<bool(PointAccessor*, PointAccessor*)> cmp_;

	kNNResultQueue candidates_;

public:
	BPQ(std::size_t max_size, PointAccessor* query) :
			max_size_(max_size), max_distance_(
					std::numeric_limits<double>::infinity()), query_(query), cmp_(
					[this](PointAccessor* left, PointAccessor* right) {
						return (
								Metrics::squared_euclidean(left, this->query_) < Metrics::squared_euclidean(right, this->query_)
						);
					}), candidates_(cmp_) {

	}

	virtual ~BPQ() {

	}

	void push(PointAccessor* pa);
	void pop();
	PointAccessor* top();
	double max_dist();
};

#endif
