#ifndef UTIL_RANDOMPOINTGENERATOR_H_
#define UTIL_RANDOMPOINTGENERATOR_H_

#include "../model/MBR.h"

#include <random>
#include <vector>

class RandomPointGenerator {
private:
	std::random_device randDevice_;
	std::default_random_engine randEngine_;
	std::vector<std::uniform_real_distribution<double>*> uniform_;
	std::vector<std::normal_distribution<double>*> gauss_;

	bool checkMBR_;
	std::size_t numberOfGeneratorThreads_ = 1;
	void initUniform(MBR& m, std::size_t dimension);
	void initGauss(double mean, double stddev);
	void initGaussCluster(std::size_t dimension, double stddev);
	void genUniformPts(std::vector<double>& randPts, std::size_t numberOfPoints,
			std::size_t dimension, MBR& m, unsigned threadOffset = 0);
	void genGaussPts(std::vector<double>& randPts, std::size_t numberOfPoints,
			std::size_t dimension, std::size_t numberOfClusters, MBR& m);
public:
	RandomPointGenerator() :
			randEngine_(randDevice_()), checkMBR_(false) {

	}

	RandomPointGenerator(std::size_t seed, bool checkMBR = false) :
			randEngine_(seed), checkMBR_(checkMBR) {
	}

	virtual ~RandomPointGenerator() {
		for (auto dist : uniform_) {
			delete (dist);
		}

		for (auto dist : gauss_) {
			delete (dist);
		}
	}

	enum DISTRIBUTION {
		UNIFORM, GAUSS, GAUSS_CLUSTER
	};

	PointContainer generatePoints(std::size_t numberOfPoints, DISTRIBUTION d,
			MBR& m, double mean = 0.0, double stddev = 1.0,
			int numberOfClusters = 1);

	void setCheckMBR(bool checkFlag) {
		checkMBR_ = checkFlag;
	}
	void setNumberOfGeneratorThreads(std::size_t numberOfGeneratorThreads) {
		numberOfGeneratorThreads_ = numberOfGeneratorThreads;
	}
	std::size_t getNumberOfGeneratorThreads() {
		return numberOfGeneratorThreads_;
	}
};

#endif
