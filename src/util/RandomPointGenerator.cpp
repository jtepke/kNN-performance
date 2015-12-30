/*
 * RandomGenerator.cpp
 *
 *  Created on: Dec 19, 2015
 *      Author: jan
 */

#include "../util/RandomPointGenerator.h"
#include <stdexcept>
#include <cassert>

void RandomPointGenerator::initUniform(GridMBR& m, std::size_t dimension) {
	uniform_ = new std::uniform_real_distribution<double> *[dimension];

	for (std::size_t i = 0; i < dimension; i++) {
		auto min = m.getLowerPoint()[i];
		auto max = m.getUpperPoint()[i];
		uniform_[i] = new std::uniform_real_distribution<double>(min, max);
	}
}

void RandomPointGenerator::initGauss(double mean, double stddev) {
	gauss_ = new std::normal_distribution<double>*[1];
	gauss_[0] = new std::normal_distribution<double>(mean, stddev);
}

void RandomPointGenerator::initGaussCluster(std::size_t numberOfClusters,
		double stddev) {
	gauss_ = new std::normal_distribution<double>*[numberOfClusters];

	for (std::size_t i = 0; i < numberOfClusters; i++) {
		auto mean = (*uniform_[i])(randEngine_);
		gauss_[i] = new std::normal_distribution<double>(mean, stddev);
	}
}

void RandomPointGenerator::genUniformPts(double * randPts,
		std::size_t numberOfPoints, std::size_t dimension) {

	for (std::size_t i = 0; i < numberOfPoints; i++) {
		for (std::size_t j = 0; j < dimension; j++) {
			randPts[(i * dimension) + j] = (*uniform_[j])(randEngine_);
		}
	}
}

void RandomPointGenerator::genGaussPts(double * randPts,
		std::size_t numberOfPoints, std::size_t dimension,
		std::size_t numberOfClusters, GridMBR& m) {
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

double * RandomPointGenerator::generatePoints(std::size_t numberOfPoints,
		DISTRIBUTION distrib, GridMBR& mbr, double mean, double stddev,
		int numberOfClusters) {
	assert(!mbr.empty());

	std::size_t dimension = mbr.getLowerPoint().dimension();
	std::size_t numberOfCoordinates = numberOfPoints * dimension;

	double * randPoints = new double[numberOfCoordinates];

	switch (distrib) {
	case GAUSS_CLUSTER:
		initUniform(mbr, dimension);
		initGaussCluster(numberOfClusters, stddev);
		genGaussPts(randPoints, numberOfPoints, dimension, numberOfClusters,
				mbr);
		break;
	case GAUSS:
		initGauss(mean, stddev);
		genGaussPts(randPoints, numberOfPoints, dimension, 1,
						mbr);
		break;
	case UNIFORM:
		initUniform(mbr, dimension);
		genUniformPts(randPoints, numberOfPoints, dimension);
		break;
	default:
		//this should never happen...
		throw std::runtime_error("Cannot handle distribution: " + distrib);
	}

	return randPoints;

}

