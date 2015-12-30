#ifndef UTIL_FILEHANDLER_H_
#define UTIL_FILEHANDLER_H_

#include <cstddef>

class FileHandler {
public:
	static bool writePointsToFile(const char* fileName, double * points,
			std::size_t size);
	static bool readPointsFromFile(const char* fileName, double * points,
			std::size_t numberOfPoints, std::size_t dimension);

};

#endif /* UTIL_FILEHANDLER_H_ */
