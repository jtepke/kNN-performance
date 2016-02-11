#include "NaiveMapReduce.h"

#include <cmath>
#include <cstddef>
#include <functional>
#include <thread>
#include <vector>

BPQ<PointArrayAccessor> NaiveMapReduce::kNearestNeighbors(unsigned k,
		PointAccessor* query) {

	unsigned arraySize = numberOfPoints_ * dimension_;

	if (arraySize < singleThreadedThreashold_) {
		return NaiveKnn::kNearestNeighbors(k, query);
	}

	unsigned threadLoad = maxThreadLoad_ < k ? k : maxThreadLoad_;
	std::vector<std::thread> mapThreads;

	unsigned numberOfThreads =
			std::ceil(arraySize / threadLoad) > maxThreads_ ?
					maxThreads_ : (arraySize / threadLoad);

	//Init map result vector
	std::vector<BPQ<PointArrayAccessor>> mapResult;
	mapResult.resize(numberOfThreads, BPQ<PointArrayAccessor> { k });

	//Offset calculations for map chunks
	std::size_t thread_offset = (arraySize / numberOfThreads);
	std::size_t step = thread_offset + dimension_
			- (thread_offset % dimension_);
	std::size_t lastFullStepOffset = (numberOfThreads - 1) * step;
	std::size_t endStep = arraySize - lastFullStepOffset;

	assert(endStep + lastFullStepOffset == dimension_ * numberOfPoints_);
	assert(numberOfThreads * step >= numberOfPoints_);

	//Start the first n-1 threads
	for (unsigned threadId = 0; threadId < numberOfThreads - 1; ++threadId) {
		mapThreads.push_back(
				std::thread(&NaiveMapReduce::map, this,
						&points_[threadId * step], query, k, step, threadId,
						std::ref(mapResult)));
	}

	//Handle last thread separately
	mapThreads.push_back(
			std::thread(&NaiveMapReduce::map, this,
					&points_[lastFullStepOffset], query, k, endStep,
					numberOfThreads - 1, std::ref(mapResult)));

	//Join threads
	for (unsigned threadId = 0; threadId < numberOfThreads; ++threadId) {
		mapThreads[threadId].join();
	}

	return reduce(mapResult, query);
}

void NaiveMapReduce::map(double* points, PointAccessor* query, unsigned k,
		std::size_t step, unsigned storeId,
		std::vector<BPQ<PointArrayAccessor>>& mapResult) {
	assert(dimension_ == query->dimension());

	PointArrayAccessor current_point { points, 0, dimension_ };
	double current_dist = 0.0;

	for (std::size_t pIndex = 0; pIndex < step; pIndex += dimension_) {
		current_point.setOffset(pIndex);
		current_dist = Metrics::squared_euclidean(current_point, query);

		if (current_dist < mapResult[storeId].max_dist()) {
			mapResult[storeId].push(PointArrayAccessor { points, pIndex,
					dimension_ }, current_dist);
		}
	}
}

BPQ<PointArrayAccessor> NaiveMapReduce::reduce(
		std::vector<BPQ<PointArrayAccessor>>& mapResult, PointAccessor* query) {
	unsigned resultQueueIdx = 0;

	for (unsigned bpq_idx = 1; bpq_idx < mapResult.size(); ++bpq_idx) {

		while (!mapResult[bpq_idx].empty()) {
			double topDistance = mapResult[bpq_idx].topDistance();

			if (topDistance < mapResult[resultQueueIdx].topDistance()) {
				mapResult[resultQueueIdx].push(mapResult[bpq_idx].topPoint(),
						topDistance);
			}

			mapResult[bpq_idx].pop();
		}
	}

	return mapResult[resultQueueIdx];
}
