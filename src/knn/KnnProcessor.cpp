#include "../knn/KnnProcessor.h"
#include <cassert>

KnnProcessor::KnnProcessor() {
}

KnnProcessor::~KnnProcessor() {
}

BPQ KnnProcessor::nearestNeighbor(PointAccessor* query) {
	auto nNQueue = kNearestNeighbors(1, query);
	assert(nNQueue.size() == 1);
	return nNQueue;
}
