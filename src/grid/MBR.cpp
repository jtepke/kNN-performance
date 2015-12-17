#include "MBR.h"

const double MBR::EPSILON = 0.1;

PointAccessor MBR::getLowerPoint() {
	return (*this)[LOWER_INDEX];
}

PointAccessor MBR::getUpperPoint() {
	return (*this)[UPPERD_INDEX];
}

std::string MBR::to_string() {
	std::string str = "MBR [\n";
	str += "lower: " + getLowerPoint().to_string();
	str += "upper: " + getUpperPoint().to_string();

	return str + "]\n";
}
void MBR::resizeContainerToMBRSize() {
	if (coordinates_.empty()) {
		coordinates_.resize(dimension_);
	}
}

void MBR::addEpsilon(std::vector<double>& upper) {
	for (auto& p : upper) {
		p = p + EPSILON;
	}
}

void MBR::addLower(const std::vector<double>& point) {
	resizeContainerToMBRSize();
	addPointAtIndex(point, LOWER_INDEX);
}

void MBR::addUpper(std::vector<double>& point) {
	resizeContainerToMBRSize();
	addEpsilon(point);
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
MBR MBR::buildMBR(double * coordinates, std::size_t size,
		std::size_t dimension) {
	std::vector<double> lower(dimension);
	std::vector<double> upper(dimension);

	for (std::size_t i = 0; i < dimension; i++) {
		lower[i] = upper[i] = coordinates[i];
	}

	for (std::size_t i = 0; i < size; i++) {
		std::size_t coordinateDimension = (i % dimension);
		if (coordinates[i] < lower[coordinateDimension]) {
			lower[coordinateDimension] = coordinates[i];
		}
		if (coordinates[i] > upper[coordinateDimension]) {
			upper[coordinateDimension] = coordinates[i];
		}
	}

	MBR mbr = MBR(dimension);

	mbr.addLower(lower);
	mbr.addUpper(upper);

	return mbr;
}