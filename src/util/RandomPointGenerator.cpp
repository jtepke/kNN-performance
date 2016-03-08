#include "../util/RandomPointGenerator.h"
#include "../util/MemoryManagement.h"

#include <cassert>
#include <stdexcept>
#include <thread>

void RandomPointGenerator::initUniform(MBR& m, std::size_t dimension) {
	uniform_.reserve(dimension);

	for (std::size_t i = 0; i < dimension; i++) {
		auto min = m.getLowPoint()[i];
		auto max = m.getHighPoint()[i];
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
		auto mean = (*uniform_[0])(randEngine_);
		gauss_[i] = new std::normal_distribution<double>(mean, stddev);
	}
}

void RandomPointGenerator::genUniformPts(std::vector<double>& randPts,
		std::size_t numberOfPoints, std::size_t dimension, MBR& mbr,
		unsigned threadOffset) {

	for (std::size_t i = 0; i < numberOfPoints; i++) {
		for (std::size_t j = 0; j < dimension; j++) {
			randPts[(i * dimension) + j + threadOffset] = (*uniform_[j])(
					randEngine_);
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
		for (std::size_t c_dim = 0; c_dim < dimension; c_dim++) {
			coordIndex = (point * dimension) + c_dim;
			randPts[coordIndex] = (*gauss_[clusterID])(randEngine_);
		}
		if (checkMBR_ && !m.isWithin(&randPts[point * dimension])) {
			--point;
		}
	}
}

PointContainer RandomPointGenerator::generatePoints(std::size_t numberOfPoints,
		DISTRIBUTION distrib, MBR& mbr, double mean, double stddev,
		int numberOfClusters) {
	assert(!mbr.empty());
	assert(mbr.getLowPoint().dimension() == mbr.getHighPoint().dimension());

	std::size_t dimension = mbr.getLowPoint().dimension();
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
		if (numberOfGeneratorThreads_ == 1) {
			genUniformPts(randPoints, numberOfPoints, dimension, mbr);
		} else {
			std::vector<std::thread> mapThreads;

			std::size_t step = numberOfPoints / numberOfGeneratorThreads_;
			std::size_t lastFullStepOffset = (numberOfGeneratorThreads_ - 1)
					* step;
			std::size_t endStep = numberOfPoints - lastFullStepOffset;

			for (unsigned threadId = 0;
					threadId < numberOfGeneratorThreads_ - 1; ++threadId) {
				mapThreads.push_back(
						std::thread(&RandomPointGenerator::genUniformPts, this,
								std::ref(randPoints), step, dimension,
								std::ref(mbr), step * dimension * threadId));
			}

			mapThreads.push_back(
					std::thread(&RandomPointGenerator::genUniformPts, this,
							std::ref(randPoints), endStep, dimension,
							std::ref(mbr),
							step * dimension
									* (numberOfGeneratorThreads_ - 1)));

			for (unsigned threadId = 0; threadId < numberOfGeneratorThreads_;
					++threadId) {
				mapThreads[threadId].join();
			}

		}
		break;
	default:
		//this should never happen...
		throw std::runtime_error("Cannot handle distribution: " + distrib);
	}
	return PointContainer(dimension, randPoints);

}

