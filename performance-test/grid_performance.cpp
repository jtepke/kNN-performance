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

std::pair<long, long> testGrid(unsigned dimension, unsigned k,
		unsigned numberOfPoints, std::size_t cellFillOptimum,
		PointContainer& queries, PointContainer& points) {

	auto start_grid_build = std::chrono::system_clock::now();
	Grid grid(dimension, points.data(), numberOfPoints * dimension,
			cellFillOptimum);
	long grid_build_duration = static_cast<long>(std::chrono::duration_cast
			< mili_sec
			> (std::chrono::system_clock::now() - start_grid_build).count());

	auto start_knn = std::chrono::system_clock::now();
	for (std::size_t q_idx = 0; q_idx < queries.size(); ++q_idx) {
		auto&& q = queries[q_idx];
		grid.kNearestNeighbors(k, &q);
	}

	long knn_duration = static_cast<long>(std::chrono::duration_cast < mili_sec
			> (std::chrono::system_clock::now() - start_knn).count());

	return std::make_pair(grid_build_duration, knn_duration);
}

int main(int argc, char** argv) {

	long SEED = 62345l;
	const unsigned DIMENSION = 3;
	unsigned NUMBER_OF_TEST_POINTS = 100000000;
	unsigned NUMBER_OF_QUERIES = 1000;
	unsigned K = 50;

	RandomPointGenerator rg(SEED);
	unsigned NUMBER_OF_MBR_COORDINATES = 2 * DIMENSION;
	double mbrCoords[] = { -100.0, -100.0, -100.0, 100.0, 100.0, 100.0 };
	double query_mbrCoords[] = { -99.0, -99.0, -99.0, 99.0, 99.0, 99.0 };

	MBR mbr = MBR(DIMENSION);
	MBR query_mbr = MBR(DIMENSION);
	mbr = mbr.createMBR(mbrCoords, NUMBER_OF_MBR_COORDINATES);
	query_mbr = mbr.createMBR(query_mbrCoords, NUMBER_OF_MBR_COORDINATES);

	PointContainer points = rg.generatePoints(NUMBER_OF_TEST_POINTS,
			RandomPointGenerator::UNIFORM, mbr);

	PointContainer queries = rg.generatePoints(NUMBER_OF_QUERIES,
			RandomPointGenerator::UNIFORM, query_mbr);

	std::cout << "#### Starting Grid Performance Test ###" << std::endl;

	for (std::size_t cellFillOpt = 10; cellFillOpt <= 1000; cellFillOpt +=
			100) {

		auto res_pair = testGrid(DIMENSION, K, NUMBER_OF_TEST_POINTS,
				cellFillOpt, queries, points);

		std::cout << "build time: " << res_pair.first << std::endl;
		std::cout << "kNN lookup time: " << res_pair.second << std::endl;
		std::cout << "cell-fill-optimum: " << cellFillOpt << std::endl;
		std::cout << "k: " << K << std::endl;
	}
	std::cout << "#### Grid Performance Test Succeeded###" << std::endl;
	return EXIT_SUCCESS;
}
