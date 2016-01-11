#ifndef MODEL_POINTARRAYACCESSOR_H_
#define MODEL_POINTARRAYACCESSOR_H_

#include "PointAccessor.h"

class PointArrayAccessor: public PointAccessor {
private:
	double * container_;
public:
	PointArrayAccessor(double * pointsContainer, std::size_t pointIndexOffset,
			std::size_t dimension) :
			PointAccessor(pointIndexOffset, dimension), container_(
					pointsContainer) {
	}

	virtual ~PointArrayAccessor() {
	}

	double& operator[](std::size_t idx) override;
	double* getData() override;
};

#endif
