#include "MBR.h"

PointVectorAccessor MBR::getLowerPoint() {
	return (*this)[LOWER_INDEX];
}

PointVectorAccessor MBR::getUpperPoint() {
	return (*this)[UPPERD_INDEX];
}

void MBR::to_stream(std::ostream& os) {
	os << "MBR [\n";
	os << "lower: ";
	getLowerPoint().to_stream(os);
	os << "upper: ";
	getUpperPoint().to_stream(os);

	os << "]" << std::endl;
}

bool MBR::empty() {
	return coordinates_.size() < (2 * dimension_);
}

void MBR::resizeContainerToMBRSize() {
	if (coordinates_.empty()) {
		coordinates_.resize(2 * dimension_);
	}
}

void MBR::addLower(std::vector<double>& point) {
	resizeContainerToMBRSize();
	addPointAtIndex(point, LOWER_INDEX);
}

void MBR::addUpper(std::vector<double>& point) {
	resizeContainerToMBRSize();
	addPointAtIndex(point, UPPERD_INDEX);
}

bool MBR::isWithin(double * point) {
	bool isWithin = true;

	for (std::size_t i = 0; i < dimension_; i++) {
		isWithin = isWithin && point[i] >= getLowerPoint()[i];
		isWithin = isWithin && point[i] <= getUpperPoint()[i];
	}

	return isWithin;
}

MBR MBR::createMBR(double * coordinates) {
	std::vector<double> lower(dimension_);
	std::vector<double> upper(dimension_);

	for (std::size_t i = 0; i < dimension_; i++) {
		lower[i] = upper[i] = coordinates[i];
	}

	std::size_t array_size = 2 * dimension_;

	for (std::size_t i = 0; i < array_size; i++) {
		std::size_t coordinateDimension = (i % dimension_);
		if (coordinates[i] < lower[coordinateDimension]) {
			lower[coordinateDimension] = coordinates[i];
		}
		if (coordinates[i] > upper[coordinateDimension]) {
			upper[coordinateDimension] = coordinates[i];
		}
	}

	this->addLower(lower);
	this->addUpper(upper);

	return (*this);
}
