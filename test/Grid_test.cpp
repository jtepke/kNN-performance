#include "gtest/gtest.h"
#include "grid/Grid.h"
#include "knn/BPQ.h"
#include "knn/Metrics.h"
#include "model/PointArrayAccessor.h"
#include "util/RandomPointGenerator.h"
#include "knn/NaiveKnn.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <chrono>
#include <utility>
#include <vector>

typedef std::chrono::milliseconds mili_sec;

class GridTest: public ::testing::Test {
protected:
	static const unsigned THREE_DIMENSIONS = 3;
	static const unsigned SIZE_POINT_SET_1 = 5;
	static const unsigned NUMBER_OF_EXAMPLE_GRIDS = 3;
	static const unsigned NUMBER_OF_COORDINATES_SET_1 = SIZE_POINT_SET_1
			* THREE_DIMENSIONS;
	static const unsigned ONE_POINT_PER_CELL = 1;
	static const unsigned THREE_POINTS_PER_CELL = 3;
	static const unsigned FIVE_POINTS_PER_CELL = 5;

	std::vector<Grid*> testGrids_;
	std::array<double, NUMBER_OF_COORDINATES_SET_1> point_set_1 { { 0.0, 0.0,
			0.0, 1.0, 1.0, 1.0, 2.0, 0.0, -1.0, 3.5, 1.0, 0.0, 4.5, -2.5, 5.0 } };

	Grid* g1_;
	Grid* g2_;
	Grid* g3_;

	virtual void SetUp() {

		g1_ = new Grid(THREE_DIMENSIONS, point_set_1.data(),
				NUMBER_OF_COORDINATES_SET_1, ONE_POINT_PER_CELL);
		g2_ = new Grid(THREE_DIMENSIONS, point_set_1.data(),
				NUMBER_OF_COORDINATES_SET_1, THREE_POINTS_PER_CELL);
		g3_ = new Grid(THREE_DIMENSIONS, point_set_1.data(),
				NUMBER_OF_COORDINATES_SET_1, FIVE_POINTS_PER_CELL);

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
	EXPECT_GT(g1_->mbr_.getHighPoint()[0], 4.5);
	EXPECT_GT(g1_->mbr_.getHighPoint()[1], 1.0);
	EXPECT_GT(g1_->mbr_.getHighPoint()[2], 5.0);
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
	EXPECT_THROW(g1_->insert(point_outside_of_grid, false), std::runtime_error);
}

class GridKnnTest: public ::testing::Test {
protected:
	static const unsigned DIMENSION = 3;

	const unsigned NUMBER_OF_TEST_POINTS = 10000;
	const unsigned NUMBER_OF_QUERIES = 5;
	const unsigned MAX_K = NUMBER_OF_TEST_POINTS;
	const unsigned SEED = 12345;

	Grid* kNN_test_grid_;
	RandomPointGenerator queryGenerator { };
	PointContainer points_;

	MBR grid_mbr { DIMENSION };
	MBR query_mbr { DIMENSION };

	virtual void SetUp() {
		RandomPointGenerator rg(SEED);
		double gridMbrCoords[] = { -100.0, 0.0, -50.0, 100.0, 7.0, 42.1235896 };
		double queryMbrCoords[] = { -90.0, 0.5, -48.0, 100.0, 6.5, 40.0 };
		grid_mbr = grid_mbr.createMBR(gridMbrCoords, 6);
		query_mbr = query_mbr.createMBR(queryMbrCoords, 6);

		points_ = rg.generatePoints(NUMBER_OF_TEST_POINTS,
				RandomPointGenerator::UNIFORM, grid_mbr);
		kNN_test_grid_ = new Grid(DIMENSION, points_.data(),
				NUMBER_OF_TEST_POINTS * DIMENSION);
	}

	virtual void TearDown() {
	}

	PointContainer genQueries(std::size_t numberOfQueries) {
		return queryGenerator.generatePoints(numberOfQueries,
				RandomPointGenerator::UNIFORM, query_mbr);
	}

};

class GridInsertTest: public ::testing::Test {
protected:
	static const unsigned DIMENSION = 3;

	const unsigned NUMBER_OF_TEST_POINTS = 100000000;

	Grid* kNN_test_grid_;
	RandomPointGenerator pointGenerator { };
	PointContainer points_;

	MBR grid_mbr { DIMENSION };
	MBR query_mbr { DIMENSION };

	virtual void SetUp() {
		double gridMbrCoords[] = { -100.0, 0.0, -50.0, 100.0, 7.0, 42.1235896 };
		grid_mbr = grid_mbr.createMBR(gridMbrCoords, 6);

		points_ = pointGenerator.generatePoints(NUMBER_OF_TEST_POINTS,
				RandomPointGenerator::UNIFORM, grid_mbr);
		auto start_grid_build = std::chrono::system_clock::now();
		kNN_test_grid_ = new Grid(DIMENSION, points_.data(),
				NUMBER_OF_TEST_POINTS * DIMENSION);
		long grid_build_duration = static_cast<long>(std::chrono::duration_cast
				< mili_sec
				> (std::chrono::system_clock::now() - start_grid_build).count());
		std::cout << "Grid build up time: " << grid_build_duration << std::endl;
	}

	virtual void TearDown() {
	}
};

TEST_F(GridInsertTest, GridHandlesParallelInsertsProperly) {
	EXPECT_EQ(kNN_test_grid_->numberOfPoints_, NUMBER_OF_TEST_POINTS);
}

///////////////////////////////////
/////////// kNN Tests /////////////
///////////////////////////////////
TEST_F(GridKnnTest, kNN_grid_scenario_does_not_contain_duplicate_points) {
	unsigned actual_number_of_stored_points = 0;
	for (const auto& pc : kNN_test_grid_->grid_) {
		actual_number_of_stored_points += pc.size();
	}

	EXPECT_EQ(actual_number_of_stored_points, NUMBER_OF_TEST_POINTS);
}

TEST_F(GridKnnTest, getHyperSquareEnvironment_returns_all_cells_eventually) {
	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };
	PointArrayAccessor query(queryCoords, 0, DIMENSION);
	unsigned cellNumber = kNN_test_grid_->cellNumber(&query);
	std::vector<unsigned> cartesionQueryCoords = kNN_test_grid_->getCartesian(
			cellNumber);
	std::vector<unsigned> unexpected;

	//Iteration 0, same cell number should be returned:
	auto result_0 = kNN_test_grid_->getHyperSquareCellEnvironment(0, cellNumber,
			cartesionQueryCoords);
	EXPECT_EQ(result_0.size(), 1);
	EXPECT_EQ(result_0.front(), cellNumber);
	unexpected.push_back(result_0.front());

	//Iteration 1:
	auto result_1 = kNN_test_grid_->getHyperSquareCellEnvironment(1, cellNumber,
			cartesionQueryCoords);

	std::vector<unsigned> expected_1 { 12, 21, 30, 14, 23, 32, 13, 31 };
	EXPECT_EQ(result_1.size(), expected_1.size());

	for (unsigned actual : result_1) {
		EXPECT_TRUE(
				std::find(std::begin(expected_1), std::end(expected_1), actual)
						!= std::end(result_1));
	}

	//Iteration 2:
	auto result_2 = kNN_test_grid_->getHyperSquareCellEnvironment(2, cellNumber,
			cartesionQueryCoords);

	std::vector<unsigned> expected_2 { 2, 11, 20, 29, 6, 15, 24, 33, 3, 4, 5 };
	EXPECT_EQ(result_2.size(), expected_2.size());

	for (unsigned actual : result_2) {
		EXPECT_TRUE(
				std::find(std::begin(expected_2), std::end(expected_2), actual)
						!= std::end(result_2));
	}

	//Iteration 3:
	auto result_3 = kNN_test_grid_->getHyperSquareCellEnvironment(3, cellNumber,
			cartesionQueryCoords);

	std::vector<unsigned> expected_3 { 1, 10, 19, 28, 7, 16, 25, 34 };
	EXPECT_EQ(result_3.size(), expected_3.size());

	for (unsigned actual : result_3) {
		EXPECT_TRUE(
				std::find(std::begin(expected_3), std::end(expected_3), actual)
						!= std::end(result_3));
	}

	//Iteration 4:
	auto result_4 = kNN_test_grid_->getHyperSquareCellEnvironment(4, cellNumber,
			cartesionQueryCoords);

	std::vector<unsigned> expected_4 { 0, 9, 18, 27, 8, 17, 26, 35 };
	EXPECT_EQ(result_4.size(), expected_4.size());

	for (unsigned actual : result_4) {
		EXPECT_TRUE(
				std::find(std::begin(expected_4), std::end(expected_4), actual)
						!= std::end(result_4));
	}

	//Iteration > 5:
	for (unsigned k_iteration = 5; k_iteration < 100; ++k_iteration) {
		EXPECT_EQ(
				kNN_test_grid_->getHyperSquareCellEnvironment(k_iteration,
						cellNumber, cartesionQueryCoords).size(), 0);
	}

	std::size_t sum_of_points = 0;
	std::size_t sum_of_cells = 0;
	for (unsigned k_iteration = 0; k_iteration < 5; ++k_iteration) {
		auto results = kNN_test_grid_->getHyperSquareCellEnvironment(
				k_iteration, cellNumber, cartesionQueryCoords);
		for (auto cellId : results) {
			sum_of_points += kNN_test_grid_->grid_[cellId].size();
		}
		sum_of_cells += results.size();
	}

	EXPECT_EQ(sum_of_cells, kNN_test_grid_->grid_.size());
	EXPECT_EQ(sum_of_points, kNN_test_grid_->numberOfPoints_);
}

TEST_F(GridKnnTest, kNN_radius_of_valid_points_is_enlarged_correctly) {
	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };
	PointArrayAccessor query(queryCoords, 0, DIMENSION);
	double last_result = 0.0;
	double current_result;
	for (int k_it = 0; k_it < 6; k_it++) {
		current_result = kNN_test_grid_->findNextClosestCellBorder(&query,
				k_it);
		EXPECT_GT(current_result, last_result);
		last_result = current_result;
	}
	EXPECT_TRUE(last_result == std::numeric_limits<double>::infinity());
}

TEST_F(GridKnnTest, kNN_lookup_ends_with_k_results) {
	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };

	PointArrayAccessor query(queryCoords, 0, DIMENSION);

	auto result = kNN_test_grid_->kNearestNeighbors(MAX_K, &query);

	EXPECT_EQ(static_cast<std::size_t>(MAX_K), result.size());
}

TEST_F(GridKnnTest, grid_produces_same_results_as_naive_approach) {
	NaiveKnn naive(points_.data(), DIMENSION, NUMBER_OF_TEST_POINTS);

	double queryCoords[DIMENSION] = { 1.0, 1.0, 1.0 };
	PointArrayAccessor query(queryCoords, 0, DIMENSION);

	for (unsigned current_k = 1; current_k <= MAX_K; ++current_k) {
		auto pc = genQueries(NUMBER_OF_QUERIES);
		for (unsigned queryNumber = 0; queryNumber < NUMBER_OF_QUERIES;
				++queryNumber) {
			auto query = pc[queryNumber];
			auto results_naive = naive.kNearestNeighbors(current_k, &query);
			auto results_grid = kNN_test_grid_->kNearestNeighbors(current_k,
					&query);

			double naive_dist;
			double grid_dist;

			EXPECT_EQ(results_naive.size(), results_grid.size());
			while (!(results_naive.empty())) {
				auto&& pa = results_naive.top();
				naive_dist = Metrics::squared_euclidean(pa, &query);
				auto top = results_grid.top();
				grid_dist = Metrics::squared_euclidean(top, &query);

				EXPECT_DOUBLE_EQ(naive_dist, grid_dist);
				//Error log
				if (naive_dist != grid_dist) {
					std::cerr << "-----------------------\n";
					std::cerr << "found mismatch for k: " << current_k
							<< " of max. k: " << MAX_K << '\n';
					std::cerr << "the current number of queries: "
							<< queryNumber << '\n';
					std::cerr << "for query: ";
					query.to_stream(std::cerr);
					std::cerr << "the current size of BPQ is: "
							<< results_grid.size() << '\n';
					std::cerr << "naive top(): ";
					results_naive.top().to_stream(std::cerr);
//					std::cerr << "grid top(): ";
//					results_grid.top()->to_stream(std::cerr);
					std::cerr << "-----------------------\n";
					std::cerr << std::endl;
					goto error_abort;
				}
				results_grid.pop();
				results_naive.pop();
			}
		}

		//Hack, to speed up test without breaking others
		if (current_k >= 500) {
			current_k += 39;
		}

		switch (current_k) {
		case 2019:
			current_k += 2960;
			break;
		case 5539:
			current_k += 1860;
			break;
		case 7559:
			current_k += 2370;
			break;
		}
		//End hack
	}
	error_abort: ;
}
