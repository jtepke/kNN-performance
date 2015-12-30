#ifndef POINTCONTAINER_H_
#define POINTCONTAINER_H_

#include "../util/Representable.h"
#include <vector>
#include <cstddef>
#include "PointVectorAccessor.h"

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
	void to_stream(std::ostream& os) override;
};

#endif
