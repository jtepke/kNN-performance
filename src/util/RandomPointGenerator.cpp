#include "../util/RandomPointGenerator.h"
#include "../util/MemoryManagement.h"
#include <stdexcept>
#include <cassert>

void RandomPointGenerator::initUniform(MBR& m, std::size_t dimension) {
	uniform_.reserve(dimension);

	for (std::size_t i = 0; i < dimension; i++) {
		auto min = m.getLowerPoint()[i];
		auto max = m.getUpperPoint()[i];
		uniform_[i] = new std::uniform_real_distribution<double>(min, max);
	}
}

void RandomPointGenerator::initGauss(double mean, double stddev) {
	gauss_.reserve(1);
	gauss_[0] = new std::normal_distribution<double>(mean, stddev);
}

void RandomPointGenerator::initGaussCluster(std::size_t numberOfClusters,
		double stddev) {
	gauss_.reserve(numberOfClusters);

	for (std::size_t i = 0; i < numberOfClusters; i++) {
		auto mean = (*uniform_[i])(randEngine_);
		gauss_[i] = new std::normal_distribution<double>(mean, stddev);
	}
}

void RandomPointGenerator::genUniformPts(std::vector<double>& randPts,
		std::size_t numberOfPoints, std::size_t dimension, MBR& mbr) {

	for (std::size_t i = 0; i < numberOfPoints; i++) {
		for (std::size_t j = 0; j < dimension; j++) {
			randPts[(i * dimension) + j] = (*uniform_[j])(randEngine_);
		}
		assert(mbr.isWithin(&randPts[i * dimension]));
	}
}

void RandomPointGenerator::genGaussPts(std::vector<double>& randPts,
		std::size_t numberOfPoints, std::size_t dimension,
		std::size_t numberOfClusters, MBR& m) {
	std::size_t clusterID = 0;
	std::size_t coordIndex = 0;

	for (std::size_t point = 0; point < numberOfPoints; point++) {
		clusterID = point % numberOfClusters;
		for (std::size_t dim = 0; dim < dimension; dim++) {
			coordIndex = (point * dimension) + dim;
			randPts[coordIndex] = (*gauss_[clusterID])(randEngine_);
		}
	}
}

PointContainer RandomPointGenerator::generatePoints(std::size_t numberOfPoints,
		DISTRIBUTION distrib, MBR& mbr, double mean, double stddev,
		int numberOfClusters) {
	assert(!mbr.empty());
	assert(mbr.getLowerPoint().dimension() == mbr.getUpperPoint().dimension());

	std::size_t dimension = mbr.getLowerPoint().dimension();
	std::size_t numberOfCoordinates = numberOfPoints * dimension;

	std::vector<double> randPoints(numberOfCoordinates);

	switch (distrib) {
	case GAUSS_CLUSTER:
		initUniform(mbr, dimension);
		initGaussCluster(numberOfClusters, stddev);
		genGaussPts(randPoints, numberOfPoints, dimension, numberOfClusters,
				mbr);
		break;
	case GAUSS:
		initGauss(mean, stddev);
		genGaussPts(randPoints, numberOfPoints, dimension, 1, mbr);
		break;
	case UNIFORM:
		initUniform(mbr, dimension);
		genUniformPts(randPoints, numberOfPoints, dimension, mbr);
		break;
	default:
		//this should never happen...
		throw std::runtime_error("Cannot handle distribution: " + distrib);
	}
	return PointContainer(dimension, randPoints);

}

