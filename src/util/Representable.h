#ifndef REPRESENTABLE_H_
#define REPRESENTABLE_H_

#include <string>
#include <ostream>
#include <iostream>
#include <vector>

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& container) {
	os << '[';
	for (std::size_t i = 0; i < container.size(); i++) {

		if (i == container.size() - 1) {
			os << container[i] << "]" << std::endl;
		} else {
			os << container[i] << ", ";
		}

	}

	return os;
}

class Representable {
public:
	Representable() {
	}
	virtual ~Representable() {

	}
	virtual void to_stream(std::ostream& os) = 0;

};

#endif
