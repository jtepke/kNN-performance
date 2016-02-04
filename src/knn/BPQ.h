#ifndef KNN_BPQ_H_
#define KNN_BPQ_H_

#include "../knn/KnnProcessor.h"
#include "../knn/Metrics.h"

#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <queue>
#include <utility>

template<class T>
class BPQ {
	typedef std::pair<T, double> PointDistance;
	typedef std::function<bool(PointDistance, PointDistance)> MetricFunction;
	typedef std::vector<PointDistance> QueueContainer;
	typedef std::priority_queue<PointDistance, QueueContainer, MetricFunction> kNNResultQueue;

private:
	std::size_t max_size_;
	double max_distance_;
	MetricFunction cmp_;

	kNNResultQueue candidates_;

public:
	BPQ(std::size_t max_size) :
			max_size_(max_size), max_distance_(
					std::numeric_limits<double>::infinity()), cmp_(
					[](PointDistance left, PointDistance right) {
						return (
								left.second < right.second
						);
					}), candidates_(cmp_) {

	}

	void push(T pa, double distance);
	void pop();
	T topPoint();
	double topDistance();
	double max_dist();
	std::size_t size();
	bool empty();
	bool notFull();
};

template<class T>
void BPQ<T>::push(T pa, double distance) {
	assert(distance < max_distance_);

	//Pop top element if BQP is full and a better candidate exists.
	if (candidates_.size() == max_size_) {
		pop();
		assert(candidates_.size() == max_size_ - 1);
	}

	candidates_.push(std::make_pair(pa, distance));

	//Update max_distance_ iff BPQ is full.
	if (candidates_.size() == max_size_) {
		max_distance_ = topDistance();
		assert(max_distance_ < std::numeric_limits<double>::infinity());
	}
}

template<class T>
void BPQ<T>::pop() {
	candidates_.pop();
}

template<class T>
T BPQ<T>::topPoint() {
	return candidates_.top().first;
}

template<class T>
double BPQ<T>::topDistance() {
	return candidates_.top().second;
}

template<class T>
double BPQ<T>::max_dist() {
	return max_distance_;
}

template<class T>
std::size_t BPQ<T>::size() {
	return candidates_.size();
}

template<class T>
bool BPQ<T>::empty() {
	return candidates_.empty();
}

template<class T>
bool BPQ<T>::notFull() {
	return candidates_.size() < max_size_;
}

#endif
