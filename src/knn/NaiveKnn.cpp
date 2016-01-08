#include "../knn/BPQ.h"
#include "../knn/Metrics.h"
#include "../knn/NaiveKnn.h"
#include "../model/PointArrayAccessor.h"

#include <functional>
#include <cassert>
#include <queue>
#include <vector>
#include <limits>

BPQ NaiveKnn::kNearestNeighbors(unsigned k, PointAccessor* query) {
	assert(dimension_ == query->dimension());

	BPQ candidates(k, query);

	PointArrayAccessor current_point(points_, 0, dimension_);
	double current_dist;
	for (std::size_t point = 0; point < numberOfPoints_; point++) {
		std::size_t pIndexOffset = point * dimension_;
		current_point.set_new_offset(pIndexOffset);
		current_dist = Metrics::squared_euclidean(&current_point,query);

		if (current_dist < candidates.max_dist()) {
			PointArrayAccessor* current_pa = new PointArrayAccessor(points_, pIndexOffset,
					dimension_);
			candidates.push(current_pa, current_dist);
		}
	}

	return candidates;
}
