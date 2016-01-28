#ifndef KNN_NAIVEKNN_H_
#define KNN_NAIVEKNN_H_
#include "../model/PointAccessor.h"
#include "../model/PointContainer.h"

#include "KnnProcessor.h"
#include <cstddef>

class NaiveKnn: public KnnProcessor<PointAccessor*> {
private:
	double * points_;
	const std::size_t dimension_;
	const std::size_t numberOfPoints_;

public:
	NaiveKnn(double * points, std::size_t dimension, std::size_t numberOfPoints) :
			points_(points), dimension_(dimension), numberOfPoints_(
					numberOfPoints) {

	}

	BPQ<PointAccessor*> kNearestNeighbors(unsigned k, PointAccessor* query);

};

#endif
