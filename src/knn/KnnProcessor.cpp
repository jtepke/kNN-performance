/*
 * KnnProcessor.cpp
 *
 *  Created on: Dec 27, 2015
 *      Author: jan
 */

#include "../knn/KnnProcessor.h"
#include <cassert>

KnnProcessor::KnnProcessor() {
}

KnnProcessor::~KnnProcessor() {
}

std::priority_queue<double> KnnProcessor::nearestNeighbor(const PointVectorAccessor& query) {
	auto nNQueue = kNearestNeighbors(1, query);
	assert(nNQueue.size() == 1);
	return nNQueue;
}
