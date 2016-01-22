#include "gtest/gtest.h"
#include "grid/Grid.h"
#include "knn/BPQ.h"
#include "knn/Metrics.h"
#include "model/PointArrayAccessor.h"
#include "util/RandomPointGenerator.h"
#include "knn/NaiveKnn.h"

#include "cmath"
#include <array>
#include <vector>
#include <utility>

class GridTest: public ::testing::Test {
protected:
	static const unsigned THREE_DIMENSIONS = 3;
	static const unsigned SIZE_POINT_SET_1 = 5;
	static const unsigned NUMBER_OF_EXAMPLE_GRIDS = 3;
	static const unsigned ONE_POINT_PER_CELL = 1;
	static const unsigned THREE_POINTS_PER_CELL = 3;
	static const unsigned FIVE_POINTS_PER_CELL = 5;

	std::vector<Grid*> testGrids_;
	std::array<double, SIZE_POINT_SET_1 * THREE_DIMENSIONS> point_set_1 { { 0.0,
			0.0, 0.0, 1.0, 1.0, 1.0, 2.0, 0.0, -1.0, 3.5, 1.0, 0.0, 4.5, -2.5,
			5.0 } };

	Grid* g1_;
	Grid* g2_;
	Grid* g3_;

	virtual void SetUp() {
		g1_ = new Grid(THREE_DIMENSIONS, point_set_1.data(),
				SIZE_POINT_SET_1 * THREE_DIMENSIONS, ONE_POINT_PER_CELL);
		g2_ = new Grid(THREE_DIMENSIONS, point_set_1.data(),
				SIZE_POINT_SET_1 * THREE_DIMENSIONS, THREE_POINTS_PER_CELL);
		g3_ = new Grid(THREE_DIMENSIONS, point_set_1.data(),
				SIZE_POINT_SET_1 * THREE_DIMENSIONS, FIVE_POINTS_PER_CELL);

		testGrids_.push_back(g1_);
		testGrids_.push_back(g2_);
		testGrids_.push_back(g3_);
	}

	virtual void TearDown() {
		delete (g1_);
		delete (g2_);
		delete (g3_);
	}
};

///////////////////////////////////
/////////// Grid Tests ////////////
///////////////////////////////////

TEST_F(GridTest, A_Grid_can_determine_its_width) {
	//Assert Grid width (including MBR offset for high point):
	EXPECT_DOUBLE_EQ(g1_->gridWidthPerDim_[0], 4.6);
	EXPECT_DOUBLE_EQ(g1_->gridWidthPerDim_[1], 3.6);
	EXPECT_DOUBLE_EQ(g1_->gridWidthPerDim_[2], 6.1);
}

TEST_F(GridTest, A_Grid_determines_the_exact_low_point_of_all_coordinates) {
	//Assert Grid MBR: lower point
	EXPECT_GT(g1_->mbr_.getUpperPoint()[0], 4.5);
	EXPECT_GT(g1_->mbr_.getUpperPoint()[1], 1.0);
	EXPECT_GT(g1_->mbr_.getUpperPoint()[2], 5.0);
}

TEST_F(GridTest, A_Grid_chooses_proper_partitioning_for_differing_space_fill_parameter) {
	//Assert Cell numbers per dimension for different cell-fill parameters
	EXPECT_EQ(g1_->cellsPerDimension_[0], 2);
	EXPECT_EQ(g1_->cellsPerDimension_[1], 2);
	EXPECT_EQ(g1_->cellsPerDimension_[2], 3);

	EXPECT_EQ(g2_->cellsPerDimension_[0], 2);
	EXPECT_EQ(g2_->cellsPerDimension_[1], 1);
	EXPECT_EQ(g2_->cellsPerDimension_[2], 2);

	EXPECT_EQ(g3_->cellsPerDimension_[0], 1);
	EXPECT_EQ(g3_->cellsPerDimension_[1], 1);
	EXPECT_EQ(g3_->cellsPerDimension_[2], 2);
}

TEST_F(GridTest, A_Grid_always_contains_the_number_points_that_have_been_inserted) {
	//Assert that all grids consist of five points
	for (auto grid : testGrids_) {
		int numberOfPoints = 0;
		for (const auto& pc : grid->grid_) {
			numberOfPoints += pc.size();
		}
		EXPECT_EQ(numberOfPoints, 5);
	}

}

TEST_F(GridTest, A_Grid_computes_cell_numbers_for_coordinates_correctly) {
	//Make sure every cell can be reached by some (valid) input point:

	//Dim: 0
	std::array<double, 3> lower_left_front { { 0.0, -2.5, -1.0 } };
	std::array<double, 3> lower_right_front { { 4.5, -2.5, -1.0 } };

	EXPECT_EQ(g1_->cellNumber(lower_left_front.data()), 0);
	EXPECT_EQ(g1_->cellNumber(lower_right_front.data()), 1);

	//Dim: 1

	std::array<double, 3> higher_left_front { { 0.0, 1, -1.0 } };
	std::array<double, 3> higher_right_front { { 4.5, 1, -1.0 } };

	EXPECT_EQ(g1_->cellNumber(higher_left_front.data()), 2);
	EXPECT_EQ(g1_->cellNumber(higher_right_front.data()), 3);

	//Dim: 2 middle

	std::array<double, 3> lower_left_middle { { 0.0, -2.5, 2.0 } };
	std::array<double, 3> lower_right_middle { { 4.5, -2.5, 2.0 } };
	std::array<double, 3> upper_left_middle { { 0.0, 1, 2.0 } };
	std::array<double, 3> upper_right_middle { { 4.5, 1, 2.0 } };

	EXPECT_EQ(g1_->cellNumber(lower_left_middle.data()), 4);
	EXPECT_EQ(g1_->cellNumber(lower_right_middle.data()), 5);
	EXPECT_EQ(g1_->cellNumber(upper_left_middle.data()), 6);
	EXPECT_EQ(g1_->cellNumber(upper_right_middle.data()), 7);

	//Dim: 2 back
	std::array<double, 3> lower_left_back { { 0.0, -2.5, 5.0 } };
	std::array<double, 3> lower_right_back { { 4.5, -2.5, 5.0 } };
	std::array<double, 3> upper_left_back { { 0.0, 1, 5.0 } };
	std::array<double, 3> upper_right_back { { 4.5, 1, 5.0 } };

	EXPECT_EQ(g1_->cellNumber(lower_left_back.data()), 8);
	EXPECT_EQ(g1_->cellNumber(lower_right_back.data()), 9);
	EXPECT_EQ(g1_->cellNumber(upper_left_back.data()), 10);
	EXPECT_EQ(g1_->cellNumber(upper_right_back.data()), 11);
}

TEST_F(GridTest, A_Grid_throws_expections_if_points_outside_of_the_grid_are_inserted) {
	double point_outside_of_grid[] = { -1.0, -2.0, -3.0 };
	EXPECT_THROW(g1_->insert(point_outside_of_grid), std::runtime_error);
}

class GridKnnTest: public ::testing::Test {
protected:
	PointContainer points_;
	static const unsigned DIMENSION = 3;
	const unsigned NUMBER_OF_TEST_POINTS = 1000000;
	const unsigned K = 1000;
	const unsigned ANOTHER_K = 15401;
	const unsigned SEED = 12345;
	Grid* kNN_test_grid_;

	virtual void SetUp() {
		RandomPointGenerator rg(SEED);
		unsigned NUMBER_OF_MBR_COORDINATES = 2 * DIMENSION;
		double mbrCoords[] = { -100.0, 0.0, -50.0, 100.0, 7.0, 42.1235896 };

		MBR m = MBR(DIMENSION);
		m = m.createMBR(mbrCoords, NUMBER_OF_MBR_COORDINATES);
		points_ = rg.generatePoints(NUMBER_OF_TEST_POINTS,
				RandomPointGenerator::UNIFORM, m);
		kNN_test_grid_ = new Grid(DIMENSION, points_.data(),
				NUMBER_OF_TEST_POINTS * DIMENSION);
	}
	virtual void TearDown() {
	}
};

///////////////////////////////////
/////////// kNN Tests /////////////
///////////////////////////////////
TEST_F(GridKnnTest, kNN_grid_scenario_has_no_duplicate_points) {
	unsigned actual_number_of_stored_points = 0;
	for (const auto& pc : kNN_test_grid_->grid_) {
		actual_number_of_stored_points += pc.size();
	}

	EXPECT_EQ(actual_number_of_stored_points, NUMBER_OF_TEST_POINTS);
}

TEST_F(GridKnnTest, kNN_lookup_ends_with_k_results) {
	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };

	PointArrayAccessor query(queryCoords, 0, DIMENSION);

	BPQ result = kNN_test_grid_->kNearestNeighbors(K, &query);

	ASSERT_EQ(static_cast<std::size_t>(K), result.size());
	std::cout << Metrics::squared_euclidean(result.top(), &query);
}

TEST_F(GridKnnTest, grid_produces_same_results_as_naive_approach) {
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

TEST_F(GridKnnTest, grid_returns_corrects_results_for_another_k) {
	Grid grid(DIMENSION, points_.data(), NUMBER_OF_TEST_POINTS * DIMENSION);
	NaiveKnn naive(points_.data(), DIMENSION, NUMBER_OF_TEST_POINTS);

	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };
	PointArrayAccessor query(queryCoords, 0, DIMENSION);

	BPQ results_naive = naive.kNearestNeighbors(ANOTHER_K, &query);
	BPQ results_grid = grid.kNearestNeighbors(ANOTHER_K, &query);

	double naive_dist;
	double grid_dist;

	while (!(results_naive.empty())) {
		naive_dist = Metrics::squared_euclidean(results_naive.top(), &query);
		grid_dist = Metrics::squared_euclidean(results_grid.top(), &query);

		results_grid.pop();
		results_naive.pop();

		ASSERT_DOUBLE_EQ(naive_dist, grid_dist);
	}
}
