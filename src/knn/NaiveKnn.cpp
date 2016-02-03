#include "../knn/BPQ.h"
#include "../knn/Metrics.h"
#include "../knn/NaiveKnn.h"
#include "../model/PointArrayAccessor.h"

#include <functional>
#include <cassert>
#include <queue>
#include <vector>
#include <limits>

BPQ<PointArrayAccessor> NaiveKnn::kNearestNeighbors(unsigned k,
		PointAccessor* query) {
	assert(dimension_ == query->dimension());

	BPQ<PointArrayAccessor> candidates(k, query);

	PointArrayAccessor current_point(points_, 0, dimension_);
	double current_dist;

	for (std::size_t point = 0; point < numberOfPoints_; point++) {
		std::size_t pIndexOffset = point * dimension_;
		current_point.setOffset(pIndexOffset);
		current_dist = Metrics::squared_euclidean(&current_point, query);

		if (current_dist < candidates.max_dist()) {
			candidates.push(PointArrayAccessor { points_, pIndexOffset,
					dimension_ }, current_dist);
		}
	}

	return candidates;
}
