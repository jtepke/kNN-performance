#include "../util/FileHandler.h"

#include <cstdio>
#include <string>
#include <iostream>

bool FileHandler::writePointsToFile(const char* fileName, std::shared_ptr<double> points,
		std::size_t size) {

	std::FILE* fout = std::fopen(fileName, "wb");
	if (!fout) {
		std::cerr << "Failed to open '" << fileName << "'" << std::endl;
		return false;
	}

	auto writtenItems = std::fwrite(points.get(), sizeof(double), size, fout);

	if (writtenItems != size) {
		std::cerr << "Reading file '" << fileName
				<< "' did not succeed\nfread() returned: "
				<< writtenItems << std::endl;
		return false;
	}

	auto closeVal = std::fclose(fout);

	if (closeVal) {
		std::cerr << "Failed to close '" << fileName << "'" << "\terrno: "
				<< errno << std::endl;
		return false;
	}

	return true;
}

bool FileHandler::readPointsFromFile(const char * fileName, std::shared_ptr<double> points,
		std::size_t numberOfPoints, std::size_t dimension) {

	auto numberOfCoordinates = numberOfPoints * dimension;

	std::FILE* fin = std::fopen(fileName, "rb");
	if (!fin) {
		std::cerr << "Failed to open '" << fileName << "'" << std::endl;
		return false;
	}

	auto readItems = std::fread(points.get(), sizeof(double), numberOfCoordinates,
			fin);

	if (readItems != numberOfCoordinates) {
		std::cerr << "Reading file '" << fileName
				<< "' did not succeed\nfread() returned: "
				<< readItems << std::endl;
		return false;
	}

	auto closeVal = std::fclose(fin);

	if (closeVal) {
		std::cerr << "Failed to close '" << fileName << "'" << "\terrno: "
				<< errno << std::endl;
		return false;
	}

	return true;
}
