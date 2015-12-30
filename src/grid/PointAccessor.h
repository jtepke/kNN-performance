/*
 * PointReference.h
 *
 *  Created on: Dec 15, 2015
 *      Author: d065325
 */

#ifndef POINTACCESSOR_H_
#define POINTACCESSOR_H_

#include "../util/Representable.h"
#include <cstddef>
#include <vector>

class PointAccessor: public Representable {
private:
	std::vector<double>& container_;
	const std::size_t pIndexOffset_;
	const std::size_t dimension_;

public:
	PointAccessor(std::vector<double>& container, std::size_t pointIndexOffset,
			std::size_t dimension) :
			container_(container), pIndexOffset_(pointIndexOffset), dimension_(
					dimension) {
	}

	double& operator[](std::size_t idx) {
		return container_[pIndexOffset_ + idx];
	}

	size_t dimension() const;
	void to_stream(std::ostream& os) override;
};

#endif /* POINTACCESSOR_H_ */
