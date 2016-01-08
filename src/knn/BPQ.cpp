#include "BPQ.h"
#include <cassert>

void BPQ::push(PointAccessor* pa, std::size_t distance) {
	assert(distance < max_distance_);
	if (candidates_.size() == max_size_) {
		pop();
	}

	candidates_.push(pa);

	if (top() == pa) {
		max_distance_ = distance;
	} else {
		max_distance_ = Metrics::squared_euclidean(top(), query_);
	}
}

void BPQ::pop() {
	delete (candidates_.top());
	candidates_.pop();
}

PointAccessor* BPQ::top() {
	return candidates_.top();
}

double BPQ::max_dist() {
	return max_distance_;
}

std::size_t BPQ::size() {
	return candidates_.size();
}

bool BPQ::empty() {
	return candidates_.empty();
}
