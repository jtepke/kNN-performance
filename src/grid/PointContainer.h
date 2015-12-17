/*
 * PointContainer.h
 *
 *  Created on: Dec 14, 2015
 *      Author: d065325
 */

#ifndef POINTCONTAINER_H_
#define POINTCONTAINER_H_

#include "Representable.h"
#include "PointAccessor.h"
#include <vector>
#include <cstddef>

class PointContainer: public Representable {
protected:
	static const std::size_t DEFAULT_DIMENSION = 1;
	std::size_t dimension_;
	std::vector<double> coordinates_;

public:
	PointContainer(): dimension_(1) {
	}
	PointContainer(const size_t dimension) :
			dimension_(dimension) {
	}

	PointAccessor operator[](std::size_t idx) {
		const std::size_t pIndex = dimension_ * idx;

		return PointAccessor(coordinates_, pIndex, dimension_);
	}

	void add(const double* p, std::size_t size);
	void addPoint(const double* p);
	void addPointAtIndex(std::vector<double> point, std::size_t indexPosition);
	std::size_t size() const;
	bool empty();
	PointContainer clonePoint(std::size_t pointIndex) const;
	virtual std::string to_string();
};

#endif /* POINTCONTAINER_H_ */
