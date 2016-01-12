#include "BPQ.h"
#include <cassert>

void BPQ::push(PointAccessor* pa, std::size_t distance) {
	assert(distance < max_distance_);
	bool is_full = candidates_.size() == max_size_;
	//Pop top element if BQP is full and a better candidate was found.
	if (is_full) {
		pop();
	}

	candidates_.push(pa);
	//Update max_distance_ iff BPQ is full.
	if (is_full) {
		if (top() == pa) {
			max_distance_ = distance;
		} else {
			max_distance_ = Metrics::squared_euclidean(top(), query_);
		}
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
bool BPQ::notFull() {
	return candidates_.size() < max_size_;
}
