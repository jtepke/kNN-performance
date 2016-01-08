#include "../util/FileHandler.h"

#include <cstdio>
#include <string>
#include <iostream>
#include <stdexcept>

void FileHandler::writePointsToFile(const std::string& fileName, double* points,
		std::size_t size) {

	std::FILE* fout = std::fopen(fileName.c_str(), "wb");
	if (!fout) {
		std::cerr << "Failed to open '" << fileName << "'" << std::endl;
		throw std::runtime_error("Failed to open '" + fileName + "'");
	}

	auto writtenItems = std::fwrite(points, sizeof(double), size, fout);

	if (writtenItems != size) {
		std::cerr << "Reading file '" << fileName
				<< "' did not succeed\nfwrite() returned: " << writtenItems
				<< std::endl;
		throw std::runtime_error(
				"Reading file '" + fileName
						+ "' did not succeed\nfwrite() returned: "
						+ std::to_string(writtenItems));
	}

	auto closeVal = std::fclose(fout);

	if (closeVal) {
		std::cerr << "Failed to close '" << fileName << "'" << "\terrno: "
				<< errno << std::endl;
		throw std::runtime_error(
				"Failed to close '" + fileName + "'" + "\terrno: "
						+ std::to_string(errno));
	}

}

PointContainer FileHandler::readPointsFromFile(const std::string& fileName,
		std::size_t numberOfPoints, std::size_t dimension) {

	auto numberOfCoordinates = numberOfPoints * dimension;
	double * points = new double[numberOfCoordinates];

	std::FILE* fin = std::fopen(fileName.c_str(), "rb");
	if (!fin) {
		std::cerr << "Failed to open '" << fileName << "'" << std::endl;
		delete[] (points);
		throw std::runtime_error("Failed to open '" + fileName + "'");
	}

	auto readItems = std::fread(points, sizeof(double), numberOfCoordinates,
			fin);

	if (readItems != numberOfCoordinates) {
		std::cerr << "Reading file '" << fileName
				<< "' did not succeed\nfread() returned: " << readItems
				<< std::endl;
		delete[] (points);
		throw std::runtime_error(
				"Reading file '" + fileName
						+ "' did not succeed\nfread() returned: "
						+ std::to_string(readItems));
	}

	auto closeVal = std::fclose(fin);

	if (closeVal) {
		std::cerr << "Failed to close '" << fileName << "'" << "\terrno: "
				<< errno << std::endl;
		delete[] (points);
		throw std::runtime_error(
				"Failed to close '" + fileName + "'" + "\terrno: "
						+ std::to_string(errno));
	}

	return PointContainer(dimension, points, numberOfPoints);
}
