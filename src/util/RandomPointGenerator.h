#ifndef UTIL_RANDOMPOINTGENERATOR_H_
#define UTIL_RANDOMPOINTGENERATOR_H_

#include "../model/MBR.h"
#include <random>
#include <vector>
#include <memory>

class RandomPointGenerator {
private:
	std::random_device randDevice_;
	std::default_random_engine randEngine_;
	std::vector<std::unique_ptr<std::uniform_real_distribution<double>>>uniform_;
	std::vector<std::unique_ptr<std::normal_distribution<double>>> gauss_;

	void initUniform(MBR& m, std::size_t dimension);
	void initGauss(double mean, double stddev);
	void initGaussCluster(std::size_t dimension, double stddev);
	void genUniformPts(double * randPts, std::size_t numberOfPoints,
			std::size_t dimension);
	void genGaussPts(double * randPts, std::size_t numberOfPoints,
			std::size_t dimension, std::size_t numberOfClusters, MBR& m);
public:
	RandomPointGenerator() :
	randEngine_(randDevice_()) {

	}

	RandomPointGenerator(unsigned seed) :
	randEngine_(seed) {
	}

	virtual ~RandomPointGenerator() {
	}

	enum DISTRIBUTION {
		UNIFORM, GAUSS, GAUSS_CLUSTER
	};

	std::shared_ptr<double> generatePoints(std::size_t numberOfPoints, DISTRIBUTION d,
			MBR& m, double mean = 0.0, double stddev = 1.0,
			int numberOfClusters = 1);
};

#endif
