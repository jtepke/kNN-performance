#ifndef KNN_KNNPROCESSOR_H_
#define KNN_KNNPROCESSOR_H_
#include "../grid/PointAccessor.h"
#include "../grid/PointContainer.h"
#include <queue>

class KnnProcessor {
public:
	KnnProcessor();
	virtual ~KnnProcessor();
	/** Returns a vector of the k-nearest neighbors for a given query point. */
	virtual std::priority_queue<double> kNearestNeighbors(unsigned k,
			const PointVectorAccessor& query) = 0;
	/** Lookup the closest point for input query point. */
	std::priority_queue<double> nearestNeighbor(const PointVectorAccessor& query);
};

#endif /* KNN_KNNPROCESSOR_H_ */
