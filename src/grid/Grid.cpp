#include "Grid.h"

#include "../model/PointArrayAccessor.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iterator>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

Grid::~Grid() {
}

MBR Grid::initGridMBR(double * coordinates, std::size_t size,
		std::size_t dimension) {
	GridMBR m = GridMBR(dimension);
	return m.createMBR(coordinates, size);
}

void Grid::allocPointContainers() {
	std::size_t numberOfCells = productOfCellsUpToDimension(dimension_);
	grid_.resize(numberOfCells, PointContainer(dimension_));
}

void Grid::insert(double * coordinates, std::size_t size) {
	assert((size % dimension_) == 0);

	for (std::size_t i = 0; i < size; i += dimension_) {
		insert(&coordinates[i]);
	}

}

std::size_t Grid::productOfCellsUpToDimension(std::size_t dimension) {
	assert(dimension <= dimension_);

	std::size_t cellProduct = 1;
	for (std::size_t i = 0; i < dimension; i++) {
		cellProduct *= cellsPerDimension_[i];
	}

	assert(cellProduct >= 1);
	return cellProduct;
}

unsigned Grid::cellNumber(double * point) {
	unsigned cellNr = 0;
	for (std::size_t i = 0; i < dimension_; i++) {

		cellNr +=
				productOfCellsUpToDimension(i)
						* std::floor(
								(point[i] - mbr_.getLowerPoint()[i])
										/ (gridWidthPerDim_[i]
												/ cellsPerDimension_[i]));

	}

	return cellNr;
}

unsigned Grid::cellNumber(PointAccessor* pa) {
	return cellNumber(pa->getData() + pa->getOffset());
}

void Grid::insert(double * point) {
	if (!mbr_.isWithin(point)) {
		throw std::runtime_error("Point is not within MBR bounds.");
	} else {
		int cellNr = cellNumber(point);
		grid_[cellNr].addPoint(point);
	}
}

const std::vector<double> Grid::widthPerDimension() {
	std::vector<double> widthPerDim;

	for (std::size_t i = 0; i < dimension_; i++) {
		double resInCurrentDim = mbr_.getUpperPoint()[i]
				- mbr_.getLowerPoint()[i];
		widthPerDim.push_back(resInCurrentDim);
	}

	return widthPerDim;
}

const std::vector<std::size_t> Grid::calculateCellsPerDimension(
		std::size_t cellFillOptimum) const {
	std::vector<std::size_t> cellsPerDim(dimension_);
	double volume = 1.0;

	for (auto dimWidth : gridWidthPerDim_) {
		volume *= dimWidth;
	}

	double cellWidth = std::pow(
			volume / ((double) numberOfPoints_ / cellFillOptimum),
			1.0 / dimension_);

	for (std::size_t i = 0; i < dimension_; i++) {
		cellsPerDim[i] = std::ceil(gridWidthPerDim_[i] / cellWidth);
	}

	return cellsPerDim;
}

std::pair<int, double> Grid::findClosestCellBorder(PointAccessor* query) {
	int closestDistInDim = 0;
	double closestDist = std::numeric_limits<double>::infinity();
	PointVectorAccessor lowerPoint = mbr_.getLowerPoint();

	for (std::size_t d = 0; d < dimension_; d++) {
		double cellWidth = gridWidthPerDim_[d] / cellsPerDimension_[d];
		double queryDistToLowPoint = (*query)[d] - lowerPoint[d];
		int cellNumberOfLeftBorder = std::floor(
				queryDistToLowPoint / cellWidth);
		double distToLeftBorder = queryDistToLowPoint
				- (cellNumberOfLeftBorder * cellWidth);
		double distToRightBorder = ((cellNumberOfLeftBorder + 1) * cellWidth)
				- queryDistToLowPoint;

		if (distToLeftBorder < distToRightBorder) {
			closestDist =
					distToLeftBorder < closestDist ?
							distToLeftBorder : closestDist;
		} else {
			closestDist =
					distToRightBorder < closestDist ?
							distToRightBorder : closestDist;
		}
	}

	assert(closestDist < std::numeric_limits<double>::infinity());

	return std::make_pair(closestDistInDim, closestDist * closestDist);
}

void Grid::initMinAndMax(std::vector<int>& min, std::vector<int>& max,
		int kNN_iteration,
		const std::vector<unsigned>& cartesionQueryCoordinates) {

	for (std::size_t d = 0; d < dimension_; d++) {
		int queryCoord = cartesionQueryCoordinates[d];
		int diff = (queryCoord - kNN_iteration);
		int sum = (queryCoord + kNN_iteration);

		if (sum > kNN_iteration) {
			max[d] = kNN_iteration;
		} else {
			max[d] = sum < cellsPerDimension_[d] ? sum : max[d];
		}

		if (diff > 0) {
			min[d] = -kNN_iteration;
		} else {
			min[d] = -(diff + kNN_iteration);
		}

		std::cout << "cells per dim: " << cellsPerDimension_[d] << std::endl;
	}
}
unsigned Grid::calculateCellNumber(
		const std::vector<int>& gridCartesianCoords) {
	unsigned cellNumber = 0;
	for (unsigned d = 0; d < dimension_; d++) {
		cellNumber += gridCartesianCoords[d] * productOfCellsUpToDimension(d);
	}

	assert(cellNumber >= 0);
	assert(cellNumber < grid_.size());

	return cellNumber;
}
void Grid::addToResult(const std::vector<int>& shifts,
		const std::vector<unsigned>& query,
		std::vector<unsigned>& cellNumbers) {
	std::vector<int> query_cp(std::begin(query), std::end(query));

	for (std::size_t d = 0; d < dimension_; d++) {
		query_cp[d] += shifts[d];
	}

	cellNumbers.push_back(calculateCellNumber(query_cp));
}
std::vector<unsigned> Grid::getHyperSquareCellEnvironment(int kNN_iteration,
		unsigned queryCellNumber, std::vector<unsigned>& cartesianQueryCoords) {
	std::vector<unsigned> cellNumbers;
	std::vector<int> min(dimension_);
	std::vector<int> max(std::begin(cellsPerDimension_),
			std::end(cellsPerDimension_));
	if (kNN_iteration == 0) {
		cellNumbers.push_back(queryCellNumber);
	} else {
		//determine cell environment for non-trivial cases:
		initMinAndMax(min, max, kNN_iteration, cartesianQueryCoords);
		std::vector<int> fix_bounds;

		for (auto& m : min) {
			std::cout << "min: " << m << std::endl;
		}
		for (auto& m : max) {
			std::cout << "max: " << m << std::endl;
		}
		for (std::size_t fix_dim = 0; fix_dim < dimension_; fix_dim++) {
			std::vector<int> coordinateShifts(std::begin(min), std::end(min));
			if (max[fix_dim] != kNN_iteration
					&& min[fix_dim] != -kNN_iteration) {
				continue;
			}
			fix_bounds.clear();
			if (min[fix_dim] == -kNN_iteration) {
				fix_bounds.push_back(-kNN_iteration);
			}
			if (max[fix_dim] == kNN_iteration) {
				fix_bounds.push_back(kNN_iteration);
			}
			std::cout << "fix_bounds: " << std::endl;
			for (auto& f : fix_bounds) {
				std::cout << "\t" << f;
			}
			std::cout << std::endl;
			//Iterate through possible fixed states
			for (int fix_coord : fix_bounds) {
				coordinateShifts[fix_dim] = fix_coord;
				for (std::size_t before_fixed_dim = 0;
						before_fixed_dim < fix_dim; before_fixed_dim++) {
					//Add [min-ish,...,min-ish,fix-val,min,...,min]
					addToResult(coordinateShifts, cartesianQueryCoords,
							cellNumbers);
					int start_val =
							min[before_fixed_dim] == -kNN_iteration ?
									-kNN_iteration + 1 : min[before_fixed_dim];

					int end_val =
							max[before_fixed_dim] == kNN_iteration ?
									kNN_iteration - 1 : max[before_fixed_dim];

					//Add everything before fixed dimension in range: min-ish ... max-ish
					while (start_val <= end_val) {
						coordinateShifts[before_fixed_dim] = start_val;
						addToResult(coordinateShifts, cartesianQueryCoords,
								cellNumbers);
						start_val++;
					}
				}
				//Add everything after fixed dimension in range min ... max
				for (std::size_t after_fixed_dim = fix_dim + 1;
						after_fixed_dim < dimension_; after_fixed_dim++) {
					int start_val = min[after_fixed_dim];
					int end_val = max[after_fixed_dim];
					while (start_val < end_val) {
						coordinateShifts[after_fixed_dim] = start_val;
						addToResult(coordinateShifts, cartesianQueryCoords,
								cellNumbers);
						start_val++;
					}
				}
			}
		}

	}

	return cellNumbers;
}

std::vector<unsigned> Grid::getCartesian(unsigned cellNumber) {
	std::vector<unsigned> cartesianCoordinates(dimension_);
	for (std::size_t i = 0; i < dimension_; i++) {
		cartesianCoordinates[i] = cellNumber % cellsPerDimension_[i];
		cellNumber = cellNumber / cellsPerDimension_[i];
		if (cellNumber == 0) {
			break;
		}
	}

	return cartesianCoordinates;
}

BPQ Grid::kNearestNeighbors(unsigned k, PointAccessor* query) {
	BPQ candidates(k, query);
	auto squared_cell_border_dist = findClosestCellBorder(query);
	//Dimension of closest border;
	int closestDistDim = squared_cell_border_dist.first;
	//Euclidian squared dist to closest border
	double closestDistToCellBorder = squared_cell_border_dist.second;

	int kNN_iteration = 0;
	std::multimap<double, PointArrayAccessor*> unconsidered_pts;
	unsigned queryCellNo = cellNumber(query);
	std::vector<unsigned> cartesianQueryCoords = getCartesian(queryCellNo);
	while (candidates.notFull()) {
		std::cout << "closest distance to border: " << closestDistToCellBorder
				<< std::endl;
		std::cout << "unconsidered size: " << unconsidered_pts.size()
				<< std::endl;

		for (auto it = unconsidered_pts.begin(); it != unconsidered_pts.end();
				) {
			auto current_dist = it->first;
			auto candidate = it->second;
			if (current_dist < closestDistToCellBorder) {
				if (current_dist < candidates.max_dist()) {
					candidates.push(candidate, current_dist);
				}
				//remove, since point was within radius of considerable
				//points.
				it = unconsidered_pts.erase(it);
			} else {
				//points from this part are not considerable since they reach
				//out of the radius of valid points.
				break;
			}
		}
		std::cout << "considered unconsidered points here." << std::endl;
		for (unsigned cNumber : getHyperSquareCellEnvironment(kNN_iteration,
				queryCellNo, cartesianQueryCoords)) {
			std::cout << "grid size: " << grid_.size() << std::endl;
			PointContainer& pc = grid_[cNumber];
			std::cout << "size of pointContainer[";
			for (auto c : getCartesian(cNumber)) {
				std::cout << c << ' ';
			}
			std::cout << "]: " << pc.size() << std::endl;
			for (std::size_t p_idx = 0; p_idx < pc.size(); p_idx++) {
				auto point = pc[p_idx];
				auto candidate = new PointArrayAccessor(point.getData(),
						point.getOffset(), point.dimension());
				double current_dist = Metrics::squared_euclidean(candidate,
						query);
				if (current_dist < closestDistToCellBorder) {
					if (current_dist < candidates.max_dist()) {
						candidates.push(candidate, current_dist);
					}
				} else {
					unconsidered_pts.insert(
							std::make_pair(current_dist, candidate));
				}
			}
		}
		std::cout << "processed all cells in iteration: " << kNN_iteration
				<< std::endl;
		closestDistToCellBorder += (gridWidthPerDim_[closestDistDim]
				/ cellsPerDimension_[closestDistDim]);
		kNN_iteration++;
	}
	return candidates;
}

void Grid::to_stream(std::ostream& os) {
	os << "Grid[\n";
	int bucketCounter = 0;
	os << "dimension: " << dimension_ << +'\n';
	os << "cells in dimension: [\n";

	for (std::size_t i = 0; i < dimension_; i++) {
		os << "dim " << i << ": " << cellsPerDimension_[i] << '\n';
	}

	os << "]\n";
	os << "number of buckets: " << grid_.size() << '\n';
	os << "points / bucket: " << (numberOfPoints_ / grid_.size()) << '\n';
	mbr_.to_stream(os);

	for (PointContainer bucket : grid_) {
		if (!bucket.empty()) {
			os << "bucket " << bucketCounter << "[\n";
			bucket.to_stream(os);
			os << "\n]\n";
		}
		bucketCounter++;
	}

	os << "\n]";
}
