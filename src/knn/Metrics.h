#ifndef KNN_METRICS_H_
#define KNN_METRICS_H_
#include "../model/PointAccessor.h"
#include "../model/PointVectorAccessor.h"
#include "../model/PointArrayAccessor.h"

class Metrics {
public:
	Metrics();
	virtual ~Metrics();
	static double squared_euclidean(const PointAccessor* p, const PointAccessor* q);
	static double squared_euclidean(const PointVectorAccessor& p, const PointAccessor* q);
	static double squared_euclidean(const PointArrayAccessor& p, const PointAccessor* q);
	static double euclidean(const PointAccessor* p, const PointAccessor* q);

};

#endif
