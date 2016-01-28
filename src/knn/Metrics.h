#ifndef KNN_METRICS_H_
#define KNN_METRICS_H_
#include "../model/PointAccessor.h"
#include "../model/PointVectorAccessor.h"

class Metrics {
public:
	Metrics();
	virtual ~Metrics();
	static double squared_euclidean(PointAccessor* p, PointAccessor* q);
	static double squared_euclidean(PointVectorAccessor& p, PointAccessor* q);
	static double euclidean(PointAccessor* p, PointAccessor* q);

};

#endif
