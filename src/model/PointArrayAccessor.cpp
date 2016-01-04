#include "PointArrayAccessor.h"

inline double& PointArrayAccessor::operator[](std::size_t idx) {
	return container_[pIndexOffset_ + idx];
}
