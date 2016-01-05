#ifndef MODEL_POINTACCESSOR_H_
#define MODEL_POINTACCESSOR_H_
#include "../util/Representable.h"
#include <cstddef>

class PointAccessor: Representable {
protected:
	std::size_t pIndexOffset_;
	const std::size_t dimension_;
public:
	PointAccessor(std::size_t pointIndexOffset, std::size_t dimension) :
			pIndexOffset_(pointIndexOffset), dimension_(dimension) {

	}
	virtual ~PointAccessor() {
	}

	std::size_t dimension() const;
	void set_new_offset(std::size_t new_offset);
	void to_stream(std::ostream& os) override;
	virtual double& operator[](std::size_t idx) = 0;
};

#endif
