#include "gtest/gtest.h"
#include "knn/NaiveKnn.h"
#include "knn/Metrics.h"
#include "model/PointArrayAccessor.h"
#include "util/RandomPointGenerator.h"
#include "util/FileHandler.h"

#include "iostream"
#include "string"

class NaiveKnnTest: public ::testing::Test {
protected:
	PointContainer points_;
	static const unsigned NUMBER_OF_TEST_POINTS = 1000000;
	static const unsigned DIMENSION = 3;
	static const unsigned K = 1000;
	static const unsigned SEED = 12345;
	std::string EXPECTED_SPATIAL_GRID_TEST_RESULTS =
			"../resource/test/naive_1K_expected_results_distances.bin";

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

TEST_F(NaiveKnnTest, k_1000_contains_1000_results) {
	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };
	NaiveKnn naive(points_.data(), DIMENSION, NUMBER_OF_TEST_POINTS);
	PointArrayAccessor query(queryCoords, 0, DIMENSION);

	auto result = naive.kNearestNeighbors(K, &query);

	ASSERT_EQ(static_cast<std::size_t>(K), result.size());
}

TEST_F(NaiveKnnTest, k_1000) {
	NaiveKnn naive(points_.data(), DIMENSION, NUMBER_OF_TEST_POINTS);
	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };
	PointArrayAccessor query(queryCoords, 0, DIMENSION);
	auto actualResult = naive.kNearestNeighbors(K, &query);

	PointContainer expectedResults = FileHandler::readPointsFromFile(
			EXPECTED_SPATIAL_GRID_TEST_RESULTS, K, 1);

	double actual_dist;
	int i = 0;

	while (!(actualResult.empty())) {
		actual_dist = Metrics::squared_euclidean(actualResult.top(), &query);
		actualResult.pop();

		ASSERT_DOUBLE_EQ(expectedResults[i++][0], actual_dist);

	}
}
