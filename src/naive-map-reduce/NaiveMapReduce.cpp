#include "NaiveMapReduce.h"
#include "../grid/Grid.h"

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
	std::vector<BPQ<PointArrayAccessor>> mapNaiveResult;
	std::vector<BPQ<PointVectorAccessor>> mapGridResult;
	if (knnStrategy_ == NAIVE) {
		mapNaiveResult.resize(numberOfThreads, BPQ<PointArrayAccessor> { k });
	} else {
		mapGridResult.resize(numberOfThreads, BPQ<PointVectorAccessor> { k });
	}

	//Offset calculations for map chunks
	assert(numberOfThreads > 0);

	std::size_t thread_offset = (arraySize / numberOfThreads);
	std::size_t step = thread_offset + dimension_
			- (thread_offset % dimension_);
	std::size_t lastFullStepOffset = (numberOfThreads - 1) * step;
	std::size_t endStep = arraySize - lastFullStepOffset;

	assert(thread_offset > 0);
	assert(step / numberOfThreads > 0);
	assert(lastFullStepOffset > 0);
	assert(endStep > 0);

	assert(endStep + lastFullStepOffset == dimension_ * numberOfPoints_);
	assert(numberOfThreads * step >= numberOfPoints_);

	//Start the first n-1 threads
	switch (knnStrategy_) {
	case NAIVE: {
		for (unsigned threadId = 0; threadId < numberOfThreads - 1;
				++threadId) {
			mapThreads.push_back(
					std::thread(&NaiveMapReduce::mapNaive, this,
							&points_[threadId * step], query, k, step, threadId,
							std::ref(mapNaiveResult)));
		}
	}
		break;
	case GRID: {
		for (unsigned threadId = 0; threadId < numberOfThreads - 1;
				++threadId) {
			mapThreads.push_back(
					std::thread(&NaiveMapReduce::mapGrid, this,
							&points_[threadId * step], query, k, step, threadId,
							std::ref(mapGridResult)));
		}
	}
		break;
	default:
		throw std::runtime_error("Specified kNN strategy not supported!");
	}

	//Handle last thread separately
	switch (knnStrategy_) {
	case NAIVE: {
		mapThreads.push_back(
				std::thread(&NaiveMapReduce::mapNaive, this,
						&points_[lastFullStepOffset], query, k, endStep,
						numberOfThreads - 1, std::ref(mapNaiveResult)));
	}
		break;
	case GRID: {
		mapThreads.push_back(
				std::thread(&NaiveMapReduce::mapGrid, this,
						&points_[lastFullStepOffset], query, k, endStep,
						numberOfThreads - 1, std::ref(mapGridResult)));
	}
		break;
	}

	//Join threads
	for (unsigned threadId = 0; threadId < numberOfThreads; ++threadId) {
		mapThreads[threadId].join();
	}

	//Switch to appropriate kNN strategy
	switch (knnStrategy_) {
	case NAIVE: {
		return reduceNaive(mapNaiveResult, query);
	}
		break;
	case GRID: {
		return reduceGrid(mapGridResult, query, k);
	}
		break;
	default:
		//It should crash much earlier, this is just to
		//get rid compiler warnings.
		throw std::runtime_error("Specified kNN strategy not supported!");
	}

}

void NaiveMapReduce::mapNaive(double* points, PointAccessor* query, unsigned k,
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

void NaiveMapReduce::mapGrid(double* points, PointAccessor* query, unsigned k,
		std::size_t step, unsigned storeId,
		std::vector<BPQ<PointVectorAccessor>>& mapResult) {
	assert(dimension_ == query->dimension());
	Grid grid = Grid { dimension_, points, step, Grid::determineCellSize(k) };
	mapResult[storeId] = grid.kNearestNeighbors(k, query);
}

BPQ<PointArrayAccessor> NaiveMapReduce::reduceNaive(
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

BPQ<PointArrayAccessor> NaiveMapReduce::reduceGrid(
		std::vector<BPQ<PointVectorAccessor>>& mapResult, PointAccessor* query,
		unsigned k) {
	BPQ<PointArrayAccessor> result { k };
	PointArrayAccessor pa = PointArrayAccessor{query->getData(),query->getOffset(),query->dimension()};
	for (unsigned bpq_idx = 0; bpq_idx < mapResult.size(); ++bpq_idx) {

		while (!(mapResult[bpq_idx].empty())) {
			double topDistance = mapResult[bpq_idx].topDistance();

			if (topDistance < result.max_dist()) {
				result.push(pa, topDistance);
			}

			mapResult[bpq_idx].pop();
		}
	}

	return result;
}
