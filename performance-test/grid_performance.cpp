#include "../src/knn/BPQ.h"
#include "../src/grid/Grid.h"
#include "../src/knn/NaiveKnn.h"
#include "../src/util/RandomPointGenerator.h"
#include "../src/model/PointContainer.h"
#include "../src/model/PointArrayAccessor.h"

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <utility>
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

std::pair<long, long> testNaive(unsigned dimension, unsigned k,
		unsigned numberOfPoints, PointContainer& queries,
		PointContainer& points) {

	auto start_grid_build = std::chrono::system_clock::now();
	NaiveKnn naive(points.data(), dimension, numberOfPoints);
	long naive_build_duration = static_cast<long>(std::chrono::duration_cast
			< mili_sec
			> (std::chrono::system_clock::now() - start_grid_build).count());

	auto start_knn = std::chrono::system_clock::now();
	for (std::size_t q_idx = 0; q_idx < queries.size(); ++q_idx) {
		auto&& q = queries[q_idx];
		naive.kNearestNeighbors(k, &q);
	}

	long knn_duration = static_cast<long>(std::chrono::duration_cast < mili_sec
			> (std::chrono::system_clock::now() - start_knn).count());

	return std::make_pair(naive_build_duration, knn_duration);
}

void determineGridCellFillSize(unsigned k, unsigned dimension,
		unsigned numberOfPoints, PointContainer& points,
		PointContainer& queries) {
	std::cout << "#### Determine Grid Cell Fill Optimum ###" << std::endl;

	for (std::size_t cellFillOpt = 10; cellFillOpt <= 1000; cellFillOpt +=
			100) {

		auto res_pair = testGrid(dimension, k, numberOfPoints, cellFillOpt,
				queries, points);

		std::cout << "build time: " << res_pair.first << std::endl;
		std::cout << "kNN lookup time: " << res_pair.second << std::endl;
		std::cout << "cell-fill-optimum: " << cellFillOpt << std::endl;
		std::cout << "k: " << k << std::endl;
	}
	std::cout << "#### Determine Grid Cell Fill Optimum Ended ###" << std::endl;
}

int main(int argc, char** argv) {
	long SEED = 62345l;
	const unsigned DIMENSION = 3;
	unsigned NUMBER_OF_TEST_POINTS = 400000000;
	unsigned NUMBER_OF_QUERIES = 1;
	unsigned K = 500000;

	RandomPointGenerator rg(SEED);
	unsigned NUMBER_OF_MBR_COORDINATES = 2 * DIMENSION;
	double mbrCoords[] = { -100.0, -100.0, -100.0, 100.0, 100.0, 100.0 };
	double query_mbrCoords[] = { -99.0, -99.0, -99.0, 99.0, 99.0, 99.0 };

	MBR mbr = MBR(DIMENSION);
	mbr = mbr.createMBR(mbrCoords, NUMBER_OF_MBR_COORDINATES);

	MBR query_mbr = MBR(DIMENSION);
	query_mbr = query_mbr.createMBR(query_mbrCoords, NUMBER_OF_MBR_COORDINATES);

	PointContainer points = rg.generatePoints(NUMBER_OF_TEST_POINTS,
			RandomPointGenerator::UNIFORM, mbr);

	PointContainer queries = rg.generatePoints(NUMBER_OF_QUERIES,
			RandomPointGenerator::UNIFORM, query_mbr);

	for (unsigned nrOfPts = 20000000; nrOfPts <= NUMBER_OF_TEST_POINTS;
			nrOfPts += 20000000) {
		std::cout << "Number Of Points: " << nrOfPts << std::endl;
		auto res_pair = testNaive(DIMENSION, K, nrOfPts, queries, points);
		std::cout << "build time: " << res_pair.first << std::endl;
		std::cout << "kNN lookup time: " << res_pair.second << std::endl;
	}

	return EXIT_SUCCESS;
}
