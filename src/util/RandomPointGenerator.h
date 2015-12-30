#ifndef UTIL_RANDOMPOINTGENERATOR_H_
#define UTIL_RANDOMPOINTGENERATOR_H_

#include <random>
//TODO: use new MBR!
#include "../grid/GridMBR.h"

class RandomPointGenerator {
private:
	std::random_device randDevice_;
	std::default_random_engine randEngine_;
	std::uniform_real_distribution<double>** uniform_;
	std::normal_distribution<double>** gauss_;

	void initUniform(GridMBR& m, std::size_t dimension);
	void initGauss(double mean, double stddev);
	void initGaussCluster(std::size_t dimension, double stddev);
	void genUniformPts(double * randPts, std::size_t numberOfPoints,
			std::size_t dimension);
	void genGaussPts(double * randPts, std::size_t numberOfPoints,
			std::size_t dimension, std::size_t numberOfClusters, GridMBR& m);
public:
	RandomPointGenerator() :
			randEngine_(randDevice_()) {

	}

	RandomPointGenerator(unsigned seed) :
			randEngine_(seed) {
	}

	enum DISTRIBUTION {
		UNIFORM, GAUSS, GAUSS_CLUSTER
	};

	double * generatePoints(std::size_t numberOfPoints, DISTRIBUTION d,
			GridMBR& m, double mean = 0.0, double stddev = 1.0,
			int numberOfClusters = 1);
};

#endif /* UTIL_RANDOMPOINTGENERATOR_H_ */
