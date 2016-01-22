#include "../src/grid/Grid.h"
#include "../src/util/RandomPointGenerator.h"
#include "../src/model/PointContainer.h"
#include "../src/model/PointArrayAccessor.h"

#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <utility>
#include <chrono>
#include <vector>

typedef std::chrono::milliseconds mili_sec;

std::pair<long, std::vector<long>> testGrid(unsigned dimension,
		std::vector<unsigned>& ks, unsigned numberOfPoints,
		std::size_t cellFillOptimum, PointAccessor* query,
		PointContainer& points) {
	auto start_grid_build = std::chrono::system_clock::now();
	Grid grid(dimension, points.data(), numberOfPoints * dimension,
			cellFillOptimum);
	long grid_build_duration = static_cast<long>(std::chrono::duration_cast
			< mili_sec
			> (std::chrono::system_clock::now() - start_grid_build).count());

	std::vector<long> kRuntime;

	for (auto k : ks) {
		auto start_knn = std::chrono::system_clock::now();
		grid.kNearestNeighbors(k, query);
		long knn_duration = static_cast<long>(std::chrono::duration_cast
				< mili_sec
				> (std::chrono::system_clock::now() - start_knn).count());

		kRuntime.push_back(knn_duration);
	}

	return std::make_pair(grid_build_duration, kRuntime);
}

int main(int argc, char** argv) {

	long SEED = 12345l;
	const unsigned DIMENSION = 3;
	unsigned NUMBER_OF_TEST_POINTS = 100000;
	unsigned K = 1000;

	RandomPointGenerator rg(SEED);
	unsigned NUMBER_OF_MBR_COORDINATES = 2 * DIMENSION;
	double mbrCoords[] = { -100.0, -100.0, -100.0, 100.0, 100.0, 100.0 };

	MBR m = MBR(DIMENSION);
	m = m.createMBR(mbrCoords, NUMBER_OF_MBR_COORDINATES);
	PointContainer points = rg.generatePoints(NUMBER_OF_TEST_POINTS,
			RandomPointGenerator::UNIFORM, m);

	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };
	PointArrayAccessor query(queryCoords, 0, DIMENSION);

	std::cout << "#### Starting Grid Performance Test ###" << std::endl;

	std::vector<unsigned> ks;
	long sum = 0;
	for (unsigned k = 1; k < (K + 2)/4; k += 10) {
		ks.push_back(k);
	}

	for (std::size_t cellFillOpt = 10; cellFillOpt <= 1000; cellFillOpt +=
			100) {

		auto res_pair = testGrid(DIMENSION, ks, NUMBER_OF_TEST_POINTS,
				cellFillOpt, &query, points);

		std::cout << "build time: " << res_pair.first << std::endl;
		std::cout << "cell-fill-optimum: " << cellFillOpt << std::endl;
		sum = 0;

		for (auto kRuntime : res_pair.second) {
			sum += kRuntime;
		}

		std::cout << "sum of [" << ks.size() << "] kNN  rumtimes: " << sum
				<< std::endl;
	}
	std::cout << "#### Grid Performance Test Succeeded###" << std::endl;
	return EXIT_SUCCESS;
}
