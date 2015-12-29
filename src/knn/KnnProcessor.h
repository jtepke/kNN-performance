#ifndef KNN_KNNPROCESSOR_H_
#define KNN_KNNPROCESSOR_H_
#include "../grid/PointAccessor.h"
#include "../grid/PointContainer.h"

class KnnProcessor {
private:
public:
	KnnProcessor();
	virtual ~KnnProcessor();
	/** Returns a vector of the k-nearest neighbors for a given query point. */
	virtual const PointContainer kNearestNeighbors(unsigned k,
			const PointAccessor& query) = 0;
	/** Lookup the closest point for input query point. */
	const PointAccessor nearestNeighbor(const PointAccessor& query);
};

#endif /* KNN_KNNPROCESSOR_H_ */
