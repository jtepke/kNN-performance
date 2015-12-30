#ifndef POINTACCESSOR_H_
#define POINTACCESSOR_H_

#include "../util/Representable.h"
#include "PointAccessor.h"
#include <cstddef>
#include <vector>

class PointVectorAccessor: public PointAccessor {
private:
	std::vector<double>& container_;

public:
	PointVectorAccessor(std::vector<double>& container,
			std::size_t pointIndexOffset, std::size_t dimension) :
			PointAccessor(pointIndexOffset, dimension), container_(container) {
	}
	double& operator[](std::size_t idx) override;
};

#endif
