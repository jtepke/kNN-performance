#ifndef UTIL_FILEHANDLER_H_
#define UTIL_FILEHANDLER_H_

#include <cstddef>
#include <memory>

class FileHandler {
public:
	static bool writePointsToFile(const char* fileName,
			std::shared_ptr<double> points, std::size_t size);
	static bool readPointsFromFile(const char* fileName,
			std::shared_ptr<double> points, std::size_t numberOfPoints,
			std::size_t dimension);

};

#endif
