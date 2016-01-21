#include "../src/grid/Grid.h"
#include "../src/util/RandomPointGenerator.h"
#include "../src/model/PointContainer.h"
#include "../src/model/PointArrayAccessor.h"

#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <utility>
#include <chrono>

int main(int argc, char** argv) {

	long SEED = 12345l;
	const unsigned DIMENSION = 3;
	unsigned NUMBER_OF_TEST_POINTS = 300000000;
	unsigned K = 100000;

	RandomPointGenerator rg(SEED);
	unsigned NUMBER_OF_MBR_COORDINATES = 2 * DIMENSION;
	double mbrCoords[] = { -100.0, -100.0, -100.0, 100.0, 100.0, 100.0 };

	MBR m = MBR(DIMENSION);
	m = m.createMBR(mbrCoords, NUMBER_OF_MBR_COORDINATES);
	PointContainer points_ = rg.generatePoints(NUMBER_OF_TEST_POINTS,
			RandomPointGenerator::UNIFORM, m);
	Grid grid(DIMENSION, points_.data(), NUMBER_OF_TEST_POINTS * DIMENSION,100);

	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };
	PointArrayAccessor query(queryCoords, 0, DIMENSION);

	std::cout << "#### Starting Grid Performance Test ###" << std::endl;

	auto start = std::chrono::system_clock::now();
	BPQ result_grid = grid.kNearestNeighbors(K, &query);
	auto duration = std::chrono::duration_cast < std::chrono::milliseconds
			> (std::chrono::system_clock::now() - start);

	std::cout << "kNN execution time: " << duration.count() << std::endl;

	return EXIT_SUCCESS;
}
