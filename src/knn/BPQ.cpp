#include "BPQ.h"

void BPQ::push(PointAccessor* pa) {
	auto current_distance = Metrics::squared_euclidean(pa, query_);
	if (current_distance < max_distance_) {
		if (candidates_.size() == max_size_) {
			pop();
		}

		candidates_.push(pa);
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
