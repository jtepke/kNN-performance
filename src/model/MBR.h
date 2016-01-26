#ifndef MODEL_MBR_H_
#define MODEL_MBR_H_

#include "PointContainer.h"
#include <cstddef>

class MBR: public PointContainer {
protected:
	static const std::size_t LOWER_INDEX = 0;
	static const std::size_t UPPERD_INDEX = 1;
	void resizeContainerToMBRSize();
public:
	MBR(const std::size_t dimension) :
			PointContainer(dimension) {

	}
	virtual ~MBR() {

	}

	PointVectorAccessor getLowerPoint();
	PointVectorAccessor getUpperPoint();
	virtual void addLower(std::vector<double>& point);
	virtual void addUpper(std::vector<double>& point);
	bool isWithin(double * point);
	bool isWithin(PointAccessor * point);
	void to_stream(std::ostream& os) override;
	bool empty() override;
	MBR createMBR(double * pts, std::size_t size);

};

#endif
