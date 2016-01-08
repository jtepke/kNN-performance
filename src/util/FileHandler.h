#ifndef UTIL_FILEHANDLER_H_
#define UTIL_FILEHANDLER_H_

#include "../model/PointContainer.h"

#include <cstddef>
#include <memory>
#include <string>

class FileHandler {
public:
	static void writePointsToFile(const std::string& fileName, double* points,
			std::size_t size);
	static PointContainer readPointsFromFile(const std::string& fileName,
			std::size_t numberOfPoints, std::size_t dimension);

};

#endif
