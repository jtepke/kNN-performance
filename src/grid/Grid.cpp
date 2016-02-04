#include "Grid.h"

#include "../model/PointArrayAccessor.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <chrono>
#include <iterator>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

MBR Grid::initGridMBR(double * coordinates, std::size_t dimension,
		std::size_t size) {
	GridMBR m = GridMBR(dimension);
	return m.createMBR(coordinates, size);
}

std::vector<std::size_t> Grid::initProductOfCellsUpToDimension(
		std::size_t dimension) const {
	assert(dimension <= dimension_);
	std::vector<std::size_t> pOfCellsUpToD(dimension + 1);
	pOfCellsUpToD[0] = 1;

	for (std::size_t i = 0; i < dimension; i++) {
		pOfCellsUpToD[i + 1] = pOfCellsUpToD[i] * cellsPerDimension_[i];
	}

	return pOfCellsUpToD;
}

void Grid::allocPointContainers() {
	std::size_t numberOfCells = productOfCellsUpToDimension_.at(dimension_);
	grid_.resize(numberOfCells, PointContainer(dimension_));
}

void Grid::insertMultiThreaded(double* coordinates, std::size_t size) {
	for (std::size_t i = 0; i < size; i += dimension_) {
		insert(&coordinates[i], true);
	}
}

void Grid::insert(double * coordinates, std::size_t size) {
	assert((size % dimension_) == 0);
	unsigned maxThreadLoad = 100000000;

	if (size > maxThreadLoad) {
		std::vector<std::thread> insertThreads;
		unsigned MAX_NUMBER_OF_THREADS = 20;
		unsigned numberOfThreads =
				(size / maxThreadLoad) > MAX_NUMBER_OF_THREADS ?
						MAX_NUMBER_OF_THREADS : (size / maxThreadLoad);

		//Init locks
		for (unsigned i = 0; i < grid_.size(); i++) {
			insertLocks_.push_back(new std::mutex());
		}

		//Make some offset calculations
		std::size_t thread_offset = (size / numberOfThreads);
		std::size_t step = thread_offset + dimension_
				- (thread_offset % dimension_);
		std::size_t lastFullStepOffset = (numberOfThreads - 1) * step;
		std::size_t endStep = size - lastFullStepOffset;

		//Start the first n-1 threads
		for (unsigned threadId = 0; threadId < numberOfThreads - 1;
				++threadId) {
			insertThreads.push_back(
					std::thread(&Grid::insertMultiThreaded, this,
							&coordinates[threadId * step], step));

		}

		//Handle last thread separately
		insertThreads.push_back(
				std::thread(&Grid::insertMultiThreaded, this,
						&coordinates[lastFullStepOffset], endStep));

		//Join threads
		for (unsigned threadId = 0; threadId < numberOfThreads; ++threadId) {
			insertThreads[threadId].join();
		}
	} else {
		for (std::size_t i = 0; i < size; i += dimension_) {
			insert(&coordinates[i], false);
		}
	}

}

unsigned Grid::cellNumber(double * point) {
	unsigned cellNr = 0;
	for (std::size_t i = 0; i < dimension_; i++) {

		cellNr +=
				productOfCellsUpToDimension_.at(i)
						* std::floor(
								(point[i] - mbr_.getLowPoint()[i])
										/ (gridWidthPerDim_[i]
												/ cellsPerDimension_[i]));

	}

	return cellNr;
}

unsigned Grid::cellNumber(PointAccessor* pa) {
	return cellNumber(pa->getData() + pa->getOffset());
}

void Grid::insert(double * point, bool isMultiThreaded) {
	if (!mbr_.isWithin(point)) {
		throw std::runtime_error("Point is not within MBR bounds.");
	} else {
		int cellNr = cellNumber(point);

		if (isMultiThreaded) {
			insertLocks_[cellNr]->lock();
			grid_[cellNr].addPoint(point);
			insertLocks_[cellNr]->unlock();
		} else {
			grid_[cellNr].addPoint(point);
		}

	}
}

const std::vector<double> Grid::widthPerDimension() {
	std::vector<double> widthPerDim;

	for (std::size_t i = 0; i < dimension_; i++) {
		double resInCurrentDim = mbr_.getHighPoint()[i] - mbr_.getLowPoint()[i];
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

double Grid::findNextClosestCellBorder(PointAccessor* query, int kNNiteration) {
	assert(mbr_.isWithin(query));
	assert(kNNiteration >= 0);

	double infinity = std::numeric_limits<double>::infinity();
	double closestDist = infinity;
	PointVectorAccessor lowPoint = mbr_.getLowPoint();
	PointVectorAccessor highPoint = mbr_.getHighPoint();

	for (std::size_t d = 0; d < dimension_; d++) {
		double cellWidth = gridWidthPerDim_[d] / cellsPerDimension_[d];
		double queryCoordInDim_d = (*query)[d];
		double queryDistToLowPoint = queryCoordInDim_d - lowPoint[d];
		double queryDistToHighPoint = highPoint[d] - queryCoordInDim_d;

		//row number of cell containing query point
		unsigned numberOfCellsToLeftBorder = std::floor(
				(queryDistToLowPoint / gridWidthPerDim_[d])
						* cellsPerDimension_[d]);

		double kNNIterationShift = kNNiteration * cellWidth;

		assert(queryDistToLowPoint >= 0.0);
		assert(queryDistToHighPoint >= 0.0);
		assert(numberOfCellsToLeftBorder >= 0);
		assert(numberOfCellsToLeftBorder < cellsPerDimension_[d]);
		assert(kNNIterationShift >= 0.0);

		double distToLeftBorder =
				(numberOfCellsToLeftBorder == 0) ?
						queryDistToLowPoint :
						queryDistToLowPoint
								- ((numberOfCellsToLeftBorder) * cellWidth);

		double distToRightBorder = cellWidth - distToLeftBorder;

		assert(distToLeftBorder <= cellWidth);
		assert(distToLeftBorder >= 0.0);
		assert(distToRightBorder <= cellWidth);
		assert(distToRightBorder >= 0.0);

		//shift left, depending on iteration count
		distToLeftBorder += kNNIterationShift;
		//shift right, depending on iteration count
		distToRightBorder += kNNIterationShift;

		if (distToLeftBorder > queryDistToLowPoint) {
			distToLeftBorder = infinity;
		}
		if (distToRightBorder > queryDistToHighPoint) {
			distToRightBorder = infinity;

			if (distToLeftBorder == infinity) {
				continue;
			}
		}

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

	if (closestDist == infinity) {
		//This should only happen in last iteration
		return infinity;
	} else {
		return closestDist * closestDist;
	}
}

void Grid::initMinAndMax(std::vector<int>& min, std::vector<int>& max,
		int kNN_iteration,
		const std::vector<unsigned>& cartesionQueryCoordinates) {

	for (std::size_t d = 0; d < dimension_; d++) {
		int queryCoord = cartesionQueryCoordinates[d];
		int diff = (queryCoord - kNN_iteration);
		unsigned sum = (queryCoord + kNN_iteration);

		if (sum >= cellsPerDimension_[d]) {
			max[d] = (cellsPerDimension_[d] - 1) - queryCoord;
		} else {
			max[d] = kNN_iteration;
		}

		if (diff > 0) {
			min[d] = -kNN_iteration;
		} else {
			min[d] = -(diff + kNN_iteration);
		}

		assert(max[d] < static_cast<int>(cellsPerDimension_[d]));
		assert(min[d] + kNN_iteration >= 0);
	}
}

unsigned Grid::calculateCellNumber(
		const std::vector<int>& gridCartesianCoords) {
	unsigned cellNumber = 0;

	for (unsigned d = 0; d < dimension_; d++) {
		cellNumber += gridCartesianCoords[d]
				* productOfCellsUpToDimension_.at(d);
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

		unsigned last_idx = dimension_ - 1;
		for (std::size_t fix_dim = 0; fix_dim < dimension_; ++fix_dim) {
			std::vector<int> coordinateShifts;
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

			unsigned last_it_idx =
					(last_idx == fix_dim) ? last_idx - 1 : last_idx;
			unsigned overflow_idx = (fix_dim == 0) ? 1 : 0;

			//Iterate through possible fixed states
			for (int fix_coord : fix_bounds) {
				coordinateShifts.assign(std::begin(min), std::end(min));
				coordinateShifts[fix_dim] = fix_coord;

				while (coordinateShifts[overflow_idx] <= max[overflow_idx]) {

					addToResult(coordinateShifts, cartesianQueryCoords,
							cellNumbers);

					++coordinateShifts[last_it_idx];

					unsigned i = last_it_idx;
					while (i > overflow_idx && coordinateShifts[i] > max[i]) {
						coordinateShifts[i] = min[i];
						//no need to return this result since it would be a dupe.
						if (i == fix_dim + 1) {
							assert(fix_dim != 0);
							i -= 2;
						} else {
							--i;
						}
						++coordinateShifts[i];
					}
				}
			}
			max[fix_dim] =
					(max[fix_dim] == kNN_iteration) ?
							kNN_iteration - 1 : max[fix_dim];
			min[fix_dim] =
					(min[fix_dim] == -kNN_iteration) ?
							-kNN_iteration + 1 : min[fix_dim];
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

BPQ<PointVectorAccessor> Grid::kNearestNeighbors(unsigned k,
		PointAccessor* query) {
	BPQ<PointVectorAccessor> candidates(k);

	int kNN_iteration = 0;
	double closestDistToCellBorder;
	std::multimap<double, PointVectorAccessor> unconsidered_pts;
	unsigned queryCellNo = cellNumber(query);
	std::vector<unsigned> cartesianQueryCoords = getCartesian(queryCellNo);
	while (candidates.notFull()) {
		closestDistToCellBorder = findNextClosestCellBorder(query,
				kNN_iteration);
		for (auto it = unconsidered_pts.begin(); it != unconsidered_pts.end();
				it = unconsidered_pts.erase(it)) {
			double current_dist = it->first;
			PointVectorAccessor candidate = it->second;

			if (!(current_dist < closestDistToCellBorder)) {
				break;
				//points from this part are not considerable since they not
				//in the radius of valid points.
			} else if (current_dist < candidates.max_dist()) {
				candidates.push(candidate, current_dist);
			}

		}

		for (unsigned cNumber : getHyperSquareCellEnvironment(kNN_iteration,
				queryCellNo, cartesianQueryCoords)) {
			PointContainer& pc = grid_[cNumber];

			for (std::size_t p_idx = 0; p_idx < pc.size(); ++p_idx) {
				auto point = pc[p_idx];

				double current_dist = Metrics::squared_euclidean(point, query);
				if (current_dist < closestDistToCellBorder) {
					if (current_dist < candidates.max_dist()) {
						candidates.push(point, current_dist);
					}
				} else {
					unconsidered_pts.insert(
							std::make_pair(current_dist, point));
				}
			}
		}
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
