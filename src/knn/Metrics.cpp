#include "Metrics.h"
#include <cmath>
#include <cassert>

Metrics::Metrics() {
}

Metrics::~Metrics() {
}

double Metrics::squared_euclidean(PointAccessor* p, PointAccessor* q) {
	double result = 0.0;
	for (std::size_t dim = 0; dim < (*q).dimension(); dim++) {
		result += std::pow((*p)[dim] - (*q)[dim], 2);
	}

	return result;
}

double Metrics::squared_euclidean(PointVectorAccessor p, PointAccessor* q){
	return Metrics::squared_euclidean(&p,q);
}

double Metrics::euclidean(PointAccessor* p, PointAccessor* q) {
	return std::sqrt(Metrics::squared_euclidean(p, q));
}
