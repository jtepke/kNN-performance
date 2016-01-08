#ifndef KNN_KNNPROCESSOR_H_
#define KNN_KNNPROCESSOR_H_

#include "../model/PointAccessor.h"
#include "BPQ.h"

#include <queue>
#include <functional>
#include <vector>
#include <queue>

class BPQ;

class KnnProcessor {

public:
	KnnProcessor();
	virtual ~KnnProcessor();

	/** Returns a vector of the k-nearest neighbors for a given query point. */
	virtual BPQ kNearestNeighbors(unsigned k,
			PointAccessor* query) = 0;
	/** Lookup the closest point for input query point. */
	BPQ nearestNeighbor(PointAccessor* query);
};

#endif
