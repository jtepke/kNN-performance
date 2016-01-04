#include "gtest/gtest.h"
#include "knn/Metrics.h"
#include "model/PointArrayAccessor.h"

#include "cmath"

class MetricsTest: public ::testing::Test {
protected:
	double two_point_coords_[6];
	PointArrayAccessor p1_;
	PointArrayAccessor p2_;

	MetricsTest() :
			two_point_coords_ { 0.0, 0.0, 0.0, 1.0, 1.0, 1.0 }, p1_(
					PointArrayAccessor(two_point_coords_, 0, 3)), p2_(
					PointArrayAccessor(two_point_coords_, 3, 3)) {

	}
	virtual void SetUp() {
	}
	virtual void TearDown() {
	}
};

TEST_F(MetricsTest, squared_euclidean) {
	ASSERT_DOUBLE_EQ(3.0, Metrics::squared_euclidean(p1_, p2_));
}

TEST_F(MetricsTest, euclidean) {
	ASSERT_DOUBLE_EQ(std::sqrt(3.0), Metrics::euclidean(p1_, p2_));
}
