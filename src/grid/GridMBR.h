#ifndef MBR_H_
#define MBR_H_

#include "../model/MBR.h"
#include <cstddef>

class GridMBR: public MBR {
private:
	static const double EPSILON;
	void addEpsilon(std::vector<double>& upper);

public:
	GridMBR(const std::size_t dimension) :
			MBR(dimension) {

	}
	virtual ~GridMBR() {

	}

	virtual void addHighPoint(std::vector<double>& point) override;
};

#endif
