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
			std::vector<BPQ<PointArrayAccessor>>& mapResult, PointAccessor* query);

public:
	NaiveMapReduce(double * points, std::size_t dimension,
			std::size_t numberOfPoints);
	virtual ~NaiveMapReduce();

	virtual BPQ<PointArrayAccessor> kNearestNeighbors(unsigned k,
			PointAccessor* query) override;
};

#endif
