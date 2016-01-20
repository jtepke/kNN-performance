#ifndef KNN_BPQ_H_
#define KNN_BPQ_H_

#include "../knn/KnnProcessor.h"
#include "../knn/Metrics.h"

#include <cstddef>
#include <functional>
#include <limits>
#include <queue>

class BPQ {
	typedef std::function<bool(PointAccessor*, PointAccessor*)> MetricFuction;
	typedef std::vector<PointAccessor*> QueueContainer;
	typedef std::priority_queue<PointAccessor*, QueueContainer, MetricFuction> kNNResultQueue;

private:
	std::size_t max_size_;
	double max_distance_;
	PointAccessor* query_;
	MetricFuction cmp_;

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
		while (!empty()) {
			pop();
		}
	}

	void push(PointAccessor* pa, double distance);
	void pop();
	PointAccessor* top();
	double max_dist();
	std::size_t size();
	bool empty();
	bool notFull();
};

#endif
