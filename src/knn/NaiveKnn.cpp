#include "../knn/NaiveKnn.h"
#include "../knn/Metrics.h"
#include "../model/PointArrayAccessor.h"

#include <functional>
#include <cassert>
#include <queue>
#include <vector>
#include <limits>

kNNResultQueue NaiveKnn::kNearestNeighbors(unsigned k, PointAccessor& query) {

	std::function<bool(PointAccessor*, PointAccessor*)> cmp =
			[&query](PointAccessor* left, PointAccessor* right) {
				return (
						Metrics::squared_euclidean(*(left),query) < Metrics::squared_euclidean(*(right),query)
				);
			};

	kNNResultQueue candidates(cmp);

	assert(dimension_ == query.dimension());

	PointArrayAccessor current_point = PointArrayAccessor(points_, 0,
			dimension_);
	double max_distance = std::numeric_limits<double>::infinity();
	double current_distance;

	for (std::size_t point = 0; point < numberOfPoints_; point++) {
		std::size_t pIndexOffset = point * dimension_;
		current_point.set_new_offset(pIndexOffset);
		current_distance = Metrics::squared_euclidean(current_point, query);

//		std::cout << "p[" << point << "]: " << current_distance << std::endl;
		if (current_distance < max_distance) {
			if (candidates.size() == k) {
//				std::cout << "remove: ";
//				candidates.top()->to_stream(std::cout);
//				std::cout << "with dist: " << max_distance << std::endl;
				PointAccessor* top = candidates.top();
				candidates.pop();
				delete(top);
			}

			candidates.push(
					new PointArrayAccessor(points_, pIndexOffset, dimension_));
			max_distance = Metrics::squared_euclidean(*(candidates.top()),
					query);
		}

	}

	return candidates;
}
