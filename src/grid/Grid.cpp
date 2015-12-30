#include "Grid.h"
#include <vector>
#include <cassert>
#include <cmath>
#include <string>
#include <stdexcept>
#include <iostream>

Grid::~Grid() {
}

void Grid::initPointContainers(std::size_t containerIndex) {
	if (grid_[containerIndex].empty()) {
		grid_[containerIndex] = PointContainer(dimension_);
	}
}

void Grid::allocPointContainers() {
	std::size_t numberOfCells = productOfCellsUpToDimension(dimension_);
	grid_.resize(numberOfCells);
}

void Grid::insert(double * coordinates, std::size_t size) {
	assert((size % dimension_) == 0);

	for (std::size_t i = 0; i < size; i += dimension_) {
		insert(&coordinates[i]);
	}

}

std::size_t Grid::productOfCellsUpToDimension(std::size_t dimension) {
	assert(dimension <= dimension_);

	int cellProduct = 1;
	for (std::size_t i = 0; i < dimension; i++) {
		cellProduct *= cellsPerDimension_[i];
	}

	return cellProduct;
}

std::size_t Grid::cellNumber(double * point) {
	int cellNr = 0;
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

void Grid::insert(double * point) {
	if (!mbr_.isWithin(point)) {
		throw std::runtime_error(
				"Point is not within MBR bounds.");
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

const std::vector<std::size_t> Grid::calculateCellsPerDimension() const {
	std::vector<std::size_t> cellsPerDim(dimension_);
	double volume = 1.0;

	for (auto dimWidth : gridWidthPerDim_) {
		volume *= dimWidth;
	}

	double cellWidth = std::pow(
			volume / (numberOfPoints_ / Grid::CELL_FILL_OPTIMUM),
			1.0 / dimension_);

	for (std::size_t i = 0; i < dimension_; i++) {
		cellsPerDim[i] = std::ceil(gridWidthPerDim_[i] / cellWidth);
	}

	return cellsPerDim;
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
