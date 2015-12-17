/*
 * Grid.cpp
 *
 *  Created on: Dec 16, 2015
 *      Author: d065325
 */

#include "Grid.h"
#include <vector>
#include <cassert>
#include <cmath>
#include <string>
#include <stdexcept>
#include <iostream>

Grid::~Grid() {
}

void Grid::initPointContainers() {
	std::size_t numberOfCells = productOfCellsUpToDimension(dimension_);
	grid_.resize(numberOfCells);
	for (std::size_t i = 0; i < numberOfCells; i++) {
		grid_[i] = PointContainer(dimension_);
	}
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
				"Point is not within MBR bounds: " + to_string());
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
			volume / (Grid::CELL_FILL_OPTIMUM * numberOfPoints_),
			1.0 / dimension_);

	for (std::size_t i = 0; i < dimension_; i++) {
		cellsPerDim[i] = std::ceil(gridWidthPerDim_[i] / cellWidth);
	}

	return cellsPerDim;
}

std::string Grid::to_string() {
	std::string gridStr = "Grid[\n";
	int bucketCounter = 0;
	gridStr += "dimension: " + std::to_string(dimension_) + "\n";
	gridStr += "cells in dimension: [\n";

	for (std::size_t i = 0; i < dimension_; i++) {
		gridStr += "dim " + std::to_string(i) + ": ";
		gridStr += std::to_string(cellsPerDimension_[i]) + "\n";
	}

	gridStr += "]\n";
	gridStr += "number of buckets: " + std::to_string(grid_.size()) + "\n";
	gridStr += mbr_.to_string() + "\n";

	for (PointContainer bucket : grid_) {
		if (!bucket.empty()) {
			gridStr += "bucket " + std::to_string(bucketCounter) + "[\n";
			gridStr += bucket.to_string();
			gridStr += "\n]\n";
		}
		bucketCounter++;
	}

	return gridStr + "\n]";
}
