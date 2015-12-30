#include "../knn/NaiveKnn.h"

#include <cassert>
#include <queue>
#include <vector>

std::priority_queue<double> NaiveKnn::kNearestNeighbors(unsigned k,
		const PointAccessor& query) {
	std::priority_queue<double, std::vector<double>, std::less<double> > candidates;
	assert(dimension_ == query.dimension());

	for (std::size_t point = 0; point < numberOfPoints_; point++) {
		std::size_t pIndexOffset = point * dimension_;

	}
	return candidates;
}
