#include "PointContainer.h"
#include <iostream>

std::size_t PointContainer::size() const {
	return (coordinates_.size() / dimension_);
}

void PointContainer::add(const double* p, std::size_t size) {
	for (std::size_t i = 0; i < size; i++) {
		coordinates_.push_back(p[i]);
	}
}

void PointContainer::addPoint(const double* p) {
	for (std::size_t i = 0; i < dimension_; i++) {
		coordinates_.push_back(p[i]);
	}
}

void PointContainer::addPointAtIndex(std::vector<double> point,
		std::size_t indexPosition) {
	std::size_t indexOffset = dimension_ * indexPosition;
	if (coordinates_.capacity() < (indexOffset + point.size())) {
		coordinates_.reserve((indexOffset + point.size()));
	}

	for (const auto& coord : point) {
		coordinates_[indexOffset] = coord;
		indexOffset++;
	}
}

bool PointContainer::empty() {
	return coordinates_.size() < dimension_;
}

PointContainer PointContainer::clonePoint(std::size_t pointIndex) const {
	PointContainer p = PointContainer(dimension_);
	p.add(&coordinates_[dimension_ * pointIndex], dimension_);

	return p;
}

void PointContainer::to_stream(std::ostream& os) {
	os << "PointContainer [\n";

	for (std::size_t i = 0; i < size(); i++) {
		(*this)[i].to_stream(os);
	}

	os << std::endl;
}
