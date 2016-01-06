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
	std::shared_ptr<double> points_;
	static const unsigned NUMBER_OF_TEST_POINTS = 1000000;
	static const unsigned DIMENSION = 3;
	static const unsigned K = 1000;
	static const unsigned SEED = 12345;
	std::string EXPECTED_RESULTS_FILE =
			"../resource/test/naive_1K_expected_results_distances.bin";

	virtual void SetUp() {
		RandomPointGenerator rg(SEED);
		double mbrCoords[] = { -100.0, 0.0, -50.0, 100.0, 7.0, 42.1235896 };

		MBR m = MBR(DIMENSION);
		m = m.createMBR(mbrCoords);
		points_ = rg.generatePoints(NUMBER_OF_TEST_POINTS,
				RandomPointGenerator::UNIFORM, m);
	}

	virtual void TearDown() {
	}
};

TEST_F(NaiveKnnTest, k_1000_contains_1000_results) {
	NaiveKnn naive(points_.get(), DIMENSION, NUMBER_OF_TEST_POINTS);
	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };
	PointArrayAccessor query(queryCoords, 0, DIMENSION);

	kNNResultQueue result = naive.kNearestNeighbors(K, query);

	ASSERT_EQ(static_cast<std::size_t>(K), result.size());
}

TEST_F(NaiveKnnTest, k_1000) {
	NaiveKnn naive(points_, DIMENSION, NUMBER_OF_TEST_POINTS);
	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };
	PointArrayAccessor query(queryCoords, 0, DIMENSION);
	kNNResultQueue result = naive.kNearestNeighbors(K, query);

	std::shared_ptr<double> expectedResults(new double[K]);

	FileHandler::readPointsFromFile(EXPECTED_RESULTS_FILE.c_str(),
			expectedResults, K, 1);

	double actual_dist;
	int i = 0;

	while (!(result.empty())) {
		actual_dist = Metrics::squared_euclidean(*(result.top()), query);
		result.pop();

		ASSERT_DOUBLE_EQ(expectedResults[i++], actual_dist);

	}

}
