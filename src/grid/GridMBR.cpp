#include "GridMBR.h"

const double GridMBR::EPSILON = 0.1;

void GridMBR::addEpsilon(std::vector<double>& upper) {
	for (auto& p : upper) {
		p = p + EPSILON;
	}
}

void GridMBR::addUpper(std::vector<double>& point) {
	resizeContainerToMBRSize();
	addEpsilon(point);
	addPointAtIndex(point, UPPERD_INDEX);
}
