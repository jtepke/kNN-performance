/*
 * NaiveKnn.h
 *
 *  Created on: Dec 27, 2015
 *      Author: jan
 */

#ifndef KNN_NAIVEKNN_H_
#define KNN_NAIVEKNN_H_
#include "../grid/PointAccessor.h"
#include "../grid/PointContainer.h"
#include "KnnProcessor.h"
#include <cstddef>

class NaiveKnn: public KnnProcessor {
private:
	const double * points_;
	const std::size_t dimension_;
	const std::size_t numberOfPoints_;

public:
	NaiveKnn(const double * points, std::size_t dimension,
			std::size_t numberOfPoints) :
			points_(points), dimension_(dimension), numberOfPoints_(
					numberOfPoints) {

	}
	virtual ~NaiveKnn() {
	}

	std::priority_queue<double> kNearestNeighbors(unsigned k,
			const PointAccessor& query) override;

};

#endif /* KNN_NAIVEKNN_H_ */
