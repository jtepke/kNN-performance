#include "PointAccessor.h"

size_t PointAccessor::dimension() const {
	return this->dimension_;
}

void PointAccessor::set_new_offset(std::size_t new_offset) {
	pIndexOffset_ = new_offset;
}

std::size_t PointAccessor::offset() {
	return pIndexOffset_;
}

void PointAccessor::to_stream(std::ostream& os) {
	os << "Point (";

	for (std::size_t i = 0; i < dimension_; i++) {
		os << (*this)[i];
		if (i != (dimension_ - 1)) {
			os << ", ";
		}
	}

	os << ")\n";
}
