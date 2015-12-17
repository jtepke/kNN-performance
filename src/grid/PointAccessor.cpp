/*
 * PointReference.cpp
 *
 *  Created on: Dec 15, 2015
 *      Author: d065325
 */

#include "PointAccessor.h"
#include "PointContainer.h"
#include <string>
#include <cstddef>

size_t PointAccessor::dimension() const {
	return this->dimension_;
}

std::string PointAccessor::to_string() {
	std::string str = "Point (";

	for (std::size_t i = 0; i < dimension_; i++) {
		str += std::to_string((*this)[i]);
		if (i != (dimension_ - 1)) {
			str += ", ";
		}
	}

	return str + ")\n";
}
