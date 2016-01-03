#ifndef KNN_KNNPROCESSOR_H_
#define KNN_KNNPROCESSOR_H_

#include "../model/PointAccessor.h"
#include <queue>
#include <functional>
#include <vector>
#include <queue>

typedef std::function<bool(PointAccessor*, PointAccessor*)> MetricFn;
typedef std::vector<PointAccessor*> QueueContainer;
typedef std::priority_queue<PointAccessor*, QueueContainer, MetricFn> kNNResultQueue;

class KnnProcessor {

public:
	KnnProcessor();
	virtual ~KnnProcessor();

	/** Returns a vector of the k-nearest neighbors for a given query point. */
	virtual kNNResultQueue kNearestNeighbors(unsigned k,
			PointAccessor& query) = 0;
	/** Lookup the closest point for input query point. */
	kNNResultQueue nearestNeighbor(PointAccessor& query);
};

#endif
