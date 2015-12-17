/*
 * Representable.h
 *
 *  Created on: Dec 9, 2015
 *      Author: d065325
 */

#ifndef REPRESENTABLE_H_
#define REPRESENTABLE_H_

#include <string>
#include <ostream>

class Representable {
public:
	Representable();
	virtual ~Representable();
	virtual std::string to_string() = 0;
	void to_stream(std::ostream &os);
};

#endif /* REPRESENTABLE_H_ */
