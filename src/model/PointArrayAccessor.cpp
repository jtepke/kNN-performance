#include "PointArrayAccessor.h"

inline double& PointVectorAccessor::operator[](std::size_t idx) {
	return container_[pIndexOffset_ + idx];
}

