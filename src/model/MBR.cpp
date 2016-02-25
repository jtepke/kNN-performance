#include "MBR.h"
#include "PointAccessor.h"

#include <cassert>

PointVectorAccessor MBR::getLowPoint() {
	return (*this)[LOW_INDEX];
}

PointVectorAccessor MBR::getHighPoint() {
	return (*this)[HIGH_INDEX];
}

void MBR::to_stream(std::ostream& os) {
	os << "MBR [\n";
	os << "lower: ";
	getLowPoint().to_stream(os);
	os << "upper: ";
	getHighPoint().to_stream(os);

	os << "]" << std::endl;
}

MBR MBR::merge(std::vector<MBR>& mbrs) {
	assert(mbrs.size() > 0);
	//start flattening list of MBRs
	std::size_t dim = mbrs[0].dimension_;
	std::size_t flatCoordinateContainerSize = mbrs.size() * 2 * dim;
	double* flatCoordinates = new double[flatCoordinateContainerSize];

	std::size_t currentIdx = 0;
	for (auto& mbr : mbrs) {
		auto lowPoint = mbr.getHighPoint();
		auto highPoint = mbr.getLowPoint();
		for (std::size_t i = 0; i < dim; ++i) {
			flatCoordinates[currentIdx] = lowPoint[i];
			++currentIdx;
		}
		for (std::size_t i = 0; i < dim; ++i) {
			flatCoordinates[currentIdx] = highPoint[i];
			++currentIdx;
		}
	}

	return mbrs[0].createMBR(flatCoordinates, flatCoordinateContainerSize);
}

bool MBR::empty() {
	return coordinates_.size() < (2 * dimension_);
}

void MBR::resizeContainerToMBRSize() {
	if (coordinates_.empty()) {
		coordinates_.resize(2 * dimension_);
	}
}

void MBR::addLowPoint(std::vector<double>& point) {
	resizeContainerToMBRSize();
	addPointAtIndex(point, LOW_INDEX);
}

void MBR::addHighPoint(std::vector<double>& point) {
	resizeContainerToMBRSize();
	addPointAtIndex(point, HIGH_INDEX);
}

bool MBR::isWithin(double * point) {
	bool isWithin = true;

	for (std::size_t i = 0; (i < dimension_) && isWithin; i++) {
		isWithin = isWithin && point[i] >= getLowPoint()[i];
		isWithin = isWithin && point[i] <= getHighPoint()[i];
	}

	return isWithin;
}

bool MBR::isWithin(PointAccessor * point) {
	return isWithin(&(*point)[0]);
}

MBR MBR::createMBR(double * coordinates, std::size_t input_array_size) {
	assert((input_array_size % dimension_) == 0);
	std::vector<double> lower(dimension_);
	std::vector<double> upper(dimension_);

	for (std::size_t i = 0; i < dimension_; ++i) {
		lower[i] = upper[i] = coordinates[i];
	}

	for (std::size_t i = 0; i < input_array_size; ++i) {
		std::size_t coordinateDimension = (i % dimension_);
		if (coordinates[i] < lower[coordinateDimension]) {
			lower[coordinateDimension] = coordinates[i];
		}
		if (coordinates[i] > upper[coordinateDimension]) {
			upper[coordinateDimension] = coordinates[i];
		}
	}

	this->addLowPoint(lower);
	this->addHighPoint(upper);

	return (*this);
}
