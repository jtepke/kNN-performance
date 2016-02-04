#include "Metrics.h"
#include <cmath>
#include <cassert>

Metrics::Metrics() {
}

Metrics::~Metrics() {
}

double Metrics::squared_euclidean(const PointAccessor* p,
		const PointAccessor* q) {
	double result = 0.0;
	for (std::size_t dim = 0; dim < (*q).dimension(); dim++) {
		result += std::pow((*p)[dim] - (*q)[dim], 2);
	}

	return result;
}

double Metrics::squared_euclidean(const PointVectorAccessor& p,
		const PointAccessor* q) {
	return Metrics::squared_euclidean(&p, q);
}

double Metrics::squared_euclidean(const PointArrayAccessor& p,
		const PointAccessor* q) {
	return Metrics::squared_euclidean(&p, q);
}

double Metrics::euclidean(const PointAccessor* p, const PointAccessor* q) {
	return std::sqrt(Metrics::squared_euclidean(p, q));
}
