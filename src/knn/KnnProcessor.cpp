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

const PointAccessor KnnProcessor::nearestNeighbor(const PointAccessor& query) {
	PointContainer pc = kNearestNeighbors(1, query);
	assert(pc.size() == 1);
	return pc[0];
}
