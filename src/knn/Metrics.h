/*
 * Metrics.h
 *
 *  Created on: Dec 30, 2015
 *      Author: jan
 */

#ifndef KNN_METRICS_H_
#define KNN_METRICS_H_
#include "../grid/PointAccessor.h"

class Metrics {
public:
	Metrics();
	virtual ~Metrics();
	static double squared_euclidean(PointAccessor& p, PointAccessor& q);
	static double euclidean(PointAccessor& p, PointAccessor& q);

};

#endif /* KNN_METRICS_H_ */
