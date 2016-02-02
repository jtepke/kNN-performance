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

MBR Grid::initGridMBR(double * coordinates, std::size_t dimension) {
	GridMBR m = GridMBR(dimension);
	return m.createMBR(coordinates);
}

void Grid::allocPointContainers() {
	std::size_t numberOfCells = productOfCellsUpToDimension(dimension_);
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
		std::cout << "parallel ";
		for (unsigned i = 0; i < grid_.size(); i++) {
			insertLocks_.push_back(new std::mutex());
		}

		std::size_t thread_offset = (size / 3);
		std::size_t end1 = thread_offset + dimension_
				- (thread_offset % dimension_);
		std::size_t end2 = 2 * end1;
		std::size_t end3 = size - end2;

//		std::cout << "size: " << size << std::endl;
//		std::cout << "end1: " << end1 << std::endl;
//		std::cout << "end2: " << end2 << std::endl;
//		std::cout << "end3: " << end3 << std::endl;

		std::thread t1(&Grid::insertMultiThreaded, this, &coordinates[0], end1);
//		std::cout << "Thread 1 ceated:" << std::endl;
		std::thread t2(&Grid::insertMultiThreaded, this, &coordinates[end1],
				end1);
//		std::cout << "Thread 2 ceated:" << std::endl;
		std::thread t3(&Grid::insertMultiThreaded, this, &coordinates[end2],
				end3);
//		std::cout << "Thread 3 ceated:" << std::endl;

		t1.join();
//		std::cout << "T1 finished" << std::endl;
		t2.join();
//		std::cout << "T2 finished" << std::endl;
		t3.join();
//		std::cout << "T3 finished" << std::endl;
	} else {
		for (std::size_t i = 0; i < size; i += dimension_) {
			insert(&coordinates[i], false);
		}
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
//			std::cout << "insert into: " << cellNr << std::endl;
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
	BPQ<PointVectorAccessor> candidates(k, query);

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
				//points from this part are not considerable since they reach
				//out of the radius of valid points.
			} else {
				if (current_dist < candidates.max_dist()) {
					candidates.push(candidate, current_dist);
				} else {
					//remove, since point was within radius of considerable
					//points. This is important! Otherwise kNN look-up will/may
					//not terminate.
//					delete (candidate);
//					candidate = NULL;
				}
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

//delete unconsidered points
//TODO: extract into method
//	for (auto it = unconsidered_pts.begin(); it != unconsidered_pts.end();
//			++it) {
//		auto canidate = it->second;
//		delete (canidate);
//		canidate = NULL;
//	}

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
