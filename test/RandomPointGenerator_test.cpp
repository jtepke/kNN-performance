#include "gtest/gtest.h"
#include "model/PointContainer.h"
#include "util/RandomPointGenerator.h"

class RandomPointGeneratorTest: public ::testing::Test {
protected:
	RandomPointGenerator* rpg = nullptr;
	const std::size_t ONE_MIO_TEST_PTS = 1000000;
	const std::size_t DIMENSION = 3;
	const std::size_t TEST_SEED = 12345;
	MBR testMBR;
	RandomPointGeneratorTest() :
			testMBR { DIMENSION } {

	}
	virtual void SetUp() {
		double mbrCoords[] = { -100.0, 0.0, -50.0, 100.0, 7.0, 42.1235896 };
		testMBR = testMBR.createMBR(mbrCoords, 2 * DIMENSION);

	}

	virtual void TearDown() {
		if (rpg) {
			delete (rpg);
		}
	}

	void initRPGWithSeed(std::size_t seed) {
		rpg = new RandomPointGenerator { seed };
	}

	void initRPGWithoutSeed() {
		rpg = new RandomPointGenerator { };
	}
};

TEST_F(RandomPointGeneratorTest, can_generate_1MioPts_unfiormly_disitributed) {
	initRPGWithoutSeed();
	auto result = rpg->generatePoints(ONE_MIO_TEST_PTS,
			RandomPointGenerator::UNIFORM, testMBR);

	ASSERT_EQ(result.size(), ONE_MIO_TEST_PTS);
}

TEST_F(RandomPointGeneratorTest, can_generate_1MioPts_gauss_disitributed) {
	initRPGWithoutSeed();
	auto result = rpg->generatePoints(ONE_MIO_TEST_PTS,
			RandomPointGenerator::GAUSS, testMBR);

	ASSERT_EQ(result.size(), ONE_MIO_TEST_PTS);
}

TEST_F(RandomPointGeneratorTest, can_generate_1MioPts_gauss_cluster_disitributed_10_Clusters) {
	initRPGWithoutSeed();
	auto result = rpg->generatePoints(ONE_MIO_TEST_PTS,
			RandomPointGenerator::GAUSS_CLUSTER, testMBR, 2.0, 1.0, 10);

	ASSERT_EQ(result.size(), ONE_MIO_TEST_PTS);
}

TEST_F(RandomPointGeneratorTest, can_generate_1MioPts__twice_with_same_seeded) {
	initRPGWithSeed(TEST_SEED);
	auto result1 = rpg->generatePoints(ONE_MIO_TEST_PTS,
			RandomPointGenerator::UNIFORM, testMBR);
	initRPGWithSeed(TEST_SEED);
	auto result2 = rpg->generatePoints(ONE_MIO_TEST_PTS,
			RandomPointGenerator::UNIFORM, testMBR);

	for (unsigned i = 0; i < ONE_MIO_TEST_PTS; ++i) {
		for (unsigned j = 0; j < DIMENSION; ++j) {
			ASSERT_EQ(result1[i][j], result2[i][j]);
		}
	}
}
