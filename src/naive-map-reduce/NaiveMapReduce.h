#ifndef NAIVE_MAP_REDUCE_NAIVEMAPREDUCE_H_
#define NAIVE_MAP_REDUCE_NAIVEMAPREDUCE_H_

#include "../knn/NaiveKnn.h"
#include "../knn/BPQ.h"

#include <vector>

class NaiveMapReduce: public NaiveKnn {
private:
	void map(double* points, PointAccessor* query, unsigned k, std::size_t step,
			unsigned storeId, std::vector<BPQ<PointArrayAccessor>>& mapResult);
	BPQ<PointArrayAccessor> reduce(
			std::vector<BPQ<PointArrayAccessor>>& mapResult,
			PointAccessor* query);

	unsigned maxThreads_;
	unsigned maxThreadLoad_;
	unsigned singleThreadedThreashold_;

public:
	NaiveMapReduce(double * points, std::size_t dimension,
			std::size_t numberOfPoints, unsigned maxThreadNumber =
					MAX_NUMBER_OF_THREADS, unsigned maxThreadLoad =
					MAX_THREAD_LOAD, unsigned singleThreadedThreshold =
					SINGLE_THREADED_THRESHOLD) :
			NaiveKnn(points, dimension, numberOfPoints), maxThreads_(
					maxThreadNumber), maxThreadLoad_(maxThreadLoad), singleThreadedThreashold_(
					singleThreadedThreshold) {

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
