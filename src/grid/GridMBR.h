/*
 * MBR.h
 *
 *  Created on: Dec 16, 2015
 *      Author: d065325
 */

#ifndef MBR_H_
#define MBR_H_

#include "PointContainer.h"
#include "../util/Representable.h"
#include <cstddef>

class MBR: public PointContainer {
private:
	static const std::size_t LOWER_INDEX = 0;
	static const std::size_t UPPERD_INDEX = 1;
	static const double EPSILON;
	void resizeContainerToMBRSize();
	void addEpsilon(std::vector<double>& upper);

public:
	MBR(const std::size_t dimension) :
			PointContainer(dimension) {

	}

	virtual ~MBR() {

	}

	PointAccessor getLowerPoint();
	PointAccessor getUpperPoint();
	void addLower(const std::vector<double>& point);
	void addUpper(std::vector<double>& point);
	bool isWithin(double * point);
	void to_stream(std::ostream& os) override;
	bool empty() override;
	static MBR buildMBR(double * pts, std::size_t size, std::size_t dimension);
};

#endif /* MBR_H_ */
