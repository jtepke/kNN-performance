#ifndef MBR_H_
#define MBR_H_

#include "../model/PointContainer.h"
#include "../util/Representable.h"
#include <cstddef>

class GridMBR: public PointContainer {
private:
	static const std::size_t LOWER_INDEX = 0;
	static const std::size_t UPPERD_INDEX = 1;
	static const double EPSILON;
	void resizeContainerToMBRSize();
	void addEpsilon(std::vector<double>& upper);

public:
	GridMBR(const std::size_t dimension) :
			PointContainer(dimension) {

	}

	virtual ~GridMBR() {

	}

	PointVectorAccessor getLowerPoint();
	PointVectorAccessor getUpperPoint();
	void addLower(const std::vector<double>& point);
	void addUpper(std::vector<double>& point);
	bool isWithin(double * point);
	void to_stream(std::ostream& os) override;
	bool empty() override;
	static GridMBR buildMBR(double * pts, std::size_t size,
			std::size_t dimension);
};

#endif
