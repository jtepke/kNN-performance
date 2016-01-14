#include "gtest/gtest.h"
#include "grid/Grid.h"
#include "knn/BPQ.h"
#include "knn/Metrics.h"
#include "model/PointArrayAccessor.h"
#include "util/RandomPointGenerator.h"
#include "knn/NaiveKnn.h"

#include "cmath"

class GridTest: public ::testing::Test {
protected:
	PointContainer points_;
	static const unsigned NUMBER_OF_TEST_POINTS = 1000000;
	static const unsigned DIMENSION = 3;
	static const unsigned K = 1000;
	static const unsigned SEED = 12345;

	virtual void SetUp() {
		RandomPointGenerator rg(SEED);
		double mbrCoords[] = { -100.0, 0.0, -50.0, 100.0, 7.0, 42.1235896 };

		MBR m = MBR(DIMENSION);
		m = m.createMBR(mbrCoords, 6);
		points_ = rg.generatePoints(NUMBER_OF_TEST_POINTS,
				RandomPointGenerator::UNIFORM, m);
	}
	virtual void TearDown() {
	}
};

TEST_F(GridTest, produces_k_results) {
	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };
	Grid grid(DIMENSION, points_.data(), NUMBER_OF_TEST_POINTS * DIMENSION);
	PointArrayAccessor query(queryCoords, 0, DIMENSION);

	BPQ result = grid.kNearestNeighbors(K, &query);

	ASSERT_EQ(static_cast<std::size_t>(K), result.size());
}

TEST_F(GridTest, grid_produces_same_results_as_naive_approach) {
	Grid grid(DIMENSION, points_.data(), NUMBER_OF_TEST_POINTS * DIMENSION);
	NaiveKnn naive(points_.data(), DIMENSION, NUMBER_OF_TEST_POINTS);

	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };
	PointArrayAccessor query(queryCoords, 0, DIMENSION);

	BPQ results_naive = naive.kNearestNeighbors(K, &query);
	BPQ result_grid = grid.kNearestNeighbors(K, &query);

	double naive_dist;
	double grid_dist;
	while (!(results_naive.empty())) {
		naive_dist = Metrics::squared_euclidean(results_naive.top(), &query);
		grid_dist = Metrics::squared_euclidean(result_grid.top(), &query);

		result_grid.pop();
		results_naive.pop();

		ASSERT_DOUBLE_EQ(naive_dist, grid_dist);

	}
}
