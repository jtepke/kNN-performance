/*
 * Grid.h
 *
 *  Created on: Dec 16, 2015
 *      Author: d065325
 */

#ifndef GRID_H_
#define GRID_H_
#include "MBR.h"
#include "../util/Representable.h"
#include <cstddef>
#include <vector>

class Grid: public Representable {
private:
	/** we assume this the optimal number points per cell */
	static const unsigned CELL_FILL_OPTIMUM = 32;
	/** dimension of the grid space */
	const std::size_t dimension_;
	/** minimum bounding hyperrectangle around the inserted point cloud */
	MBR mbr_;
	/** number of points in the grid */
	const std::size_t numberOfPoints_;
	/** width of the grid in each dimension */
	const std::vector<double> gridWidthPerDim_;
	/** number if cells per row in each dimension */
	const std::vector<std::size_t> cellsPerDimension_;
	/** the grid is a vector of buckets containing points */
	std::vector<PointContainer> grid_;

	/** Insert a set of points into the grid. */
	void insert(double * coordinates, std::size_t size);
	/** Insert single point into grid. */
	void insert(double * point);
	/** Calculates gridWith per dimension. */
	const std::vector<double> widthPerDimension();
	/** Returns vector containing number of cells per dimension. */
	const std::vector<std::size_t> calculateCellsPerDimension() const;
	/** Calculates volume of cells up to a particular dimension. */
	std::size_t productOfCellsUpToDimension(std::size_t dimension);
	/** Calculates the grid index (cell number) for a point. */
	std::size_t cellNumber(double * point);
	/** Assigns empty PointsContainer to grid_[containerIndex]. */
	void initPointContainers(std::size_t containerIndex);
	/** Allocates memory for grid_ vector. */
	void allocPointContainers();

public:
	Grid(const std::size_t dimension, double * coordinates, std::size_t size) :
			dimension_(dimension), mbr_(
					MBR::buildMBR(coordinates, size, dimension)), numberOfPoints_(
					size / dimension), gridWidthPerDim_(widthPerDimension()), cellsPerDimension_(
					calculateCellsPerDimension()) {
		allocPointContainers();
		insert(coordinates, size);
	}
	virtual ~Grid();

	/** Lookup the closest point for input query point. */
	const PointAccessor nearestNeighbor(const PointAccessor& query);
	/** Returns a vector of the k-nearest neighbors for a given query point. */
	const PointContainer kNearestNeighbors(unsigned k,
			const PointAccessor& query);
	/** Returns string representation of grid object. */
	void to_stream(std::ostream& os) override;
};

#endif /* GRID_H_ */
