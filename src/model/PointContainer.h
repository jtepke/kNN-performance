#ifndef POINTCONTAINER_H_
#define POINTCONTAINER_H_

#include <cstddef>
#include <iterator>
#include <vector>

#include "PointVectorAccessor.h"
#include "../util/Representable.h"

class PointContainer: public Representable {
protected:
	static const std::size_t DEFAULT_DIMENSION = 1;
	std::size_t dimension_;
	std::vector<double> coordinates_;

public:
	PointContainer() :
			dimension_(DEFAULT_DIMENSION) {
	}
	PointContainer(const std::size_t dimension) :
			dimension_(dimension) {
	}

	PointContainer(const std::size_t dimension,
			const std::size_t vectorInitSize) :
			dimension_(dimension), coordinates_(vectorInitSize) {
	}
	PointContainer(const std::size_t dimension, double* pts,
			std::size_t numberOfPoints) :
			dimension_(dimension), coordinates_(pts,
					(pts + (numberOfPoints * dimension))) {

	}

	PointContainer(const std::size_t dimension,
			std::vector<double>& coordinates) :
			dimension_(dimension), coordinates_(coordinates) {

	}

	PointVectorAccessor operator[](std::size_t idx) {
		const std::size_t pIndex = dimension_ * idx;

		return PointVectorAccessor(coordinates_, pIndex, dimension_);
	}

	void add(const double* p, std::size_t size);
	void addPoint(const double* p);
	void addPointAtIndex(std::vector<double> point, std::size_t indexPosition);
	std::size_t size() const;
	virtual bool empty();
	PointContainer clonePoint(std::size_t pointIndex) const;
	PointContainer append(PointContainer& pc);
	double* data();

	void to_stream(std::ostream& os) override;

	std::vector<double>::iterator begin() {
		return coordinates_.begin();
	}

	std::vector<double>::iterator end() {
		return coordinates_.end();
	}
};

#endif
