#include "gtest/gtest.h"
#include "knn/NaiveKnn.h"
#include "knn/Metrics.h"
#include "model/PointArrayAccessor.h"
#include "naive-map-reduce/NaiveMapReduce.h"
#include "util/RandomPointGenerator.h"
#include "util/FileHandler.h"

#include "iostream"
#include "string"

class NaiveMapReduceKnnTest: public ::testing::Test {
protected:
	PointContainer points_;
	static const unsigned NUMBER_OF_TEST_POINTS = 1000000;
	static const unsigned DIMENSION = 3;
	static const unsigned K = 1000;
	static const unsigned SEED = 12345;
	static const unsigned MAX_THREADS = 10;
	static const unsigned MAX_THREAD_LOAD = 100000;
	static const unsigned MAX_SINGLE_THREADED = 999999;
	std::string EXPECTED_SPATIAL_GRID_TEST_RESULTS =
			"../resource/test/naive_1K_expected_results_distances.bin";

	virtual void SetUp() {
		RandomPointGenerator rg(SEED);
		double mbrCoords[] = { -100.0, 0.0, -50.0, 100.0, 7.0, 42.1235896 };

		MBR m = MBR(DIMENSION);
		m = m.createMBR(mbrCoords, 2 * DIMENSION);
		points_ = rg.generatePoints(NUMBER_OF_TEST_POINTS,
				RandomPointGenerator::UNIFORM, m);
	}

	virtual void TearDown() {
	}
};

TEST_F(NaiveMapReduceKnnTest, k_1000_contains_exacty_1000_results) {
	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };
	NaiveMapReduce naiveMapReduce(points_.data(), DIMENSION,
			NUMBER_OF_TEST_POINTS);
	PointArrayAccessor query(queryCoords, 0, DIMENSION);

	auto result = naiveMapReduce.kNearestNeighbors(K, &query);

	ASSERT_EQ(static_cast<std::size_t>(K), result.size());
}

TEST_F(NaiveMapReduceKnnTest, distributed_run_produces_same_results_as_single_threaded) {
	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };
	PointArrayAccessor query(queryCoords, 0, DIMENSION);

	NaiveKnn naive(points_.data(), DIMENSION, NUMBER_OF_TEST_POINTS);
	NaiveMapReduce naiveMapReduce(points_.data(), DIMENSION,
			NUMBER_OF_TEST_POINTS, MAX_THREADS, MAX_THREAD_LOAD, MAX_SINGLE_THREADED, KNN_STRATEGY::GRID);

	auto naiveResult = naive.kNearestNeighbors(K, &query);
	auto naiveMapReduceResult = naiveMapReduce.kNearestNeighbors(K, &query);

	double naive_top_dist = 0.0;
	double naive_map_reduce_top_dist = 0.0;

	while (!(naiveResult.empty())) {
		naive_top_dist = naiveResult.topDistance();
		naive_map_reduce_top_dist = naiveMapReduceResult.topDistance();

		ASSERT_DOUBLE_EQ(naive_top_dist, naive_map_reduce_top_dist);

		naiveResult.pop();
		naiveMapReduceResult.pop();
	}
}

TEST_F(NaiveMapReduceKnnTest, NaiveMR_using_Grid_vs_Naive_approach_end_with_same_results) {
	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };
	NaiveMapReduce naiveMapReduceNaive(points_.data(), DIMENSION,
			NUMBER_OF_TEST_POINTS, MAX_THREADS, MAX_THREAD_LOAD, MAX_SINGLE_THREADED, KNN_STRATEGY::NAIVE);
	NaiveMapReduce naiveMapReduceGrid(points_.data(), DIMENSION,
			NUMBER_OF_TEST_POINTS, MAX_THREADS, MAX_THREAD_LOAD, MAX_SINGLE_THREADED, KNN_STRATEGY::GRID);
	PointArrayAccessor query(queryCoords, 0, DIMENSION);

	auto resultNaive = naiveMapReduceNaive.kNearestNeighbors(K, &query);
	auto resultGrid = naiveMapReduceGrid.kNearestNeighbors(K, &query);

	ASSERT_EQ(static_cast<std::size_t>(K), resultNaive.size());
	ASSERT_EQ(static_cast<std::size_t>(K), resultGrid.size());

	double naive_top_dist = 0.0;
	double grid_top_dist = 0.0;

	while (!(resultNaive.empty())) {

		naive_top_dist = resultNaive.topDistance();
		grid_top_dist = resultGrid.topDistance();

		ASSERT_DOUBLE_EQ(naive_top_dist, grid_top_dist);
		resultNaive.pop();
		resultGrid.pop();
	}
}
