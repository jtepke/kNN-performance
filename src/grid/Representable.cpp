/*
 * Representable.cpp
 *
 *  Created on: Dec 9, 2015
 *      Author: d065325
 */

#include "Representable.h"

Representable::Representable() {
}

Representable::~Representable() {
}

void Representable::to_stream(std::ostream& os) {
	os << this->to_string() << std::endl;
}

