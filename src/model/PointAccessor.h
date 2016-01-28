#ifndef MODEL_POINTACCESSOR_H_
#define MODEL_POINTACCESSOR_H_
#include "../util/Representable.h"
#include <cstddef>

class PointAccessor: Representable {
protected:
	std::size_t pIndexOffset_;
	 std::size_t dimension_;
public:
	PointAccessor(std::size_t pointIndexOffset, std::size_t dimension) :
			pIndexOffset_(pointIndexOffset), dimension_(dimension) {

	}
	virtual ~PointAccessor() {
	}

	std::size_t dimension() const;
	void setOffset(std::size_t new_offset);
	std::size_t getOffset();
	void to_stream(std::ostream& os) override;
	virtual double& operator[](std::size_t idx) = 0;
	virtual double* getData() = 0;
};

#endif
