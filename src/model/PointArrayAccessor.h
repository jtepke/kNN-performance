#ifndef MODEL_POINTARRAYACCESSOR_H_
#define MODEL_POINTARRAYACCESSOR_H_

#include "PointAccessor.h"
#include "PointVectorAccessor.h"

class PointArrayAccessor: public PointAccessor {
private:
	double * container_;

public:
	PointArrayAccessor(double * pointsContainer, std::size_t pointIndexOffset,
			std::size_t dimension) :
			PointAccessor(pointIndexOffset, dimension), container_(
					pointsContainer) {
	}

	//TODO: remove constructor hack!
	PointArrayAccessor(const PointVectorAccessor& accessor) :
			PointAccessor(0, accessor.dimension()) {
		container_ = new double[dimension_];

		for (std::size_t i = 0; i < dimension_; ++i) {
			container_[i] = accessor[i];
		}
	}

	double& operator[](std::size_t idx) override;
	const double& operator[](std::size_t idx) const override;
	double* getData() override;
};

inline double& PointArrayAccessor::operator[](std::size_t idx) {
	return container_[pIndexOffset_ + idx];
}

inline const double& PointArrayAccessor::operator[](std::size_t idx) const {
	return container_[pIndexOffset_ + idx];
}

#endif
