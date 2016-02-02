#ifndef MODEL_MBR_H_
#define MODEL_MBR_H_

#include "PointContainer.h"
#include <cstddef>

class MBR: public PointContainer {
protected:
	static const std::size_t LOW_INDEX = 0;
	static const std::size_t HIGH_INDEX = 1;
	void resizeContainerToMBRSize();
public:
	MBR(const std::size_t dimension) :
			PointContainer(dimension, dimension * 2) {

	}
	virtual ~MBR() {

	}

	PointVectorAccessor getLowPoint();
	PointVectorAccessor getHighPoint();
	virtual void addLowPoint(std::vector<double>& point);
	virtual void addHighPoint(std::vector<double>& point);
	bool isWithin(double * point);
	bool isWithin(PointAccessor * point);
	void to_stream(std::ostream& os) override;
	bool empty() override;
	MBR createMBR(double * pts);

};

#endif
