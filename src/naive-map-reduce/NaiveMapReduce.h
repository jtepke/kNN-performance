#ifndef NAIVE_MAP_REDUCE_NAIVEMAPREDUCE_H_
#define NAIVE_MAP_REDUCE_NAIVEMAPREDUCE_H_

#include "../knn/NaiveKnn.h"
#include "../knn/BPQ.h"

#include <vector>

enum KNN_STRATEGY {
	NAIVE, GRID
};

class NaiveMapReduce: public NaiveKnn {
private:
	void mapNaive(double* points, PointAccessor* query, unsigned k, std::size_t step,
			unsigned storeId, std::vector<BPQ<PointArrayAccessor>>& mapResult);
	void mapGrid(double* points, PointAccessor* query, unsigned k, std::size_t step,
				unsigned storeId, std::vector<BPQ<PointVectorAccessor>>& mapResult);
	BPQ<PointArrayAccessor> reduceNaive(
			std::vector<BPQ<PointArrayAccessor>>& mapResult,
			PointAccessor* query);
	BPQ<PointArrayAccessor> reduceGrid(
				std::vector<BPQ<PointVectorAccessor>>& mapResult,
				PointAccessor* query, unsigned k);

	unsigned maxThreads_;
	unsigned maxThreadLoad_;
	unsigned singleThreadedThreashold_;
	KNN_STRATEGY knnStrategy_;

public:
	NaiveMapReduce(double * points, std::size_t dimension,
			std::size_t numberOfPoints, unsigned maxThreadNumber =
					MAX_NUMBER_OF_THREADS, unsigned maxThreadLoad =
					MAX_THREAD_LOAD, unsigned singleThreadedThreshold =
					SINGLE_THREADED_THRESHOLD, KNN_STRATEGY knn_strategy =
					KNN_STRATEGY::NAIVE) :
			NaiveKnn(points, dimension, numberOfPoints), maxThreads_(
					maxThreadNumber), maxThreadLoad_(maxThreadLoad), singleThreadedThreashold_(
					singleThreadedThreshold), knnStrategy_(knn_strategy) {

	}

	virtual ~NaiveMapReduce() {
	}

	static const unsigned MAX_NUMBER_OF_THREADS = 20;
	static const unsigned MAX_THREAD_LOAD = 200000; 	//200 k
	static const unsigned SINGLE_THREADED_THRESHOLD = 1000000; //1 Mio.

	virtual BPQ<PointArrayAccessor> kNearestNeighbors(unsigned k,
			PointAccessor* query) override;
};

#endif
