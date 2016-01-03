#include "../knn/NaiveKnn.h"
#include "../knn/Metrics.h"
#include "../model/PointArrayAccessor.h"

#include <functional>
#include <cassert>
#include <queue>
#include <vector>

kNNResultQueue NaiveKnn::kNearestNeighbors(unsigned k,
		PointAccessor& query) {

	std::function<bool (PointAccessor*, PointAccessor*)> cmp =
			[&query](PointAccessor* left, PointAccessor* right) {
				return (
						Metrics::squared_euclidean(*(left),query) < Metrics::squared_euclidean(*(right),query)
				);
			};

	kNNResultQueue candidates(cmp);

	assert(dimension_ == query.dimension());

	for (std::size_t point = 0; point < numberOfPoints_; point++) {
		std::size_t pIndexOffset = point * dimension_;

	}
	return candidates;
}
