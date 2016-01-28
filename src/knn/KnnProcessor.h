#ifndef KNN_KNNPROCESSOR_H_
#define KNN_KNNPROCESSOR_H_

#include "../model/PointAccessor.h"
#include "BPQ.h"

#include <cassert>
#include <queue>
#include <functional>
#include <vector>
#include <queue>

template<class T_BPQ>
class BPQ;

template<class T>
class KnnProcessor {

public:
	KnnProcessor();
	virtual ~KnnProcessor();

	/** Returns a vector of the k-nearest neighbors for a given query point. */
	virtual BPQ<T> kNearestNeighbors(unsigned k,
			PointAccessor* query) = 0;

	/** Lookup the closest point for input query point. */
	BPQ<T> nearestNeighbor(PointAccessor* query);
};

template<class T>
KnnProcessor<T>::KnnProcessor() {
}

template<class T>
KnnProcessor<T>::~KnnProcessor() {
}

template<class T>
BPQ<T> KnnProcessor<T>::nearestNeighbor(PointAccessor* query) {
	BPQ<T> nNQueue = kNearestNeighbors(1, query);
	assert(nNQueue.size() == 1);
	return nNQueue;
}
#endif
