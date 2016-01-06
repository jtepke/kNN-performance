#ifndef REPRESENTABLE_H_
#define REPRESENTABLE_H_

#include <string>
#include <ostream>
#include <iostream>

class Representable {
public:
	Representable() {
	}
	virtual ~Representable() {

	}
	virtual void to_stream(std::ostream& os) = 0;
};

#endif
