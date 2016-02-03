#ifndef GRID_H_
#define GRID_H_

#include "../util/Representable.h"
#include "../model/PointAccessor.h"
#include "../knn/KnnProcessor.h"
#include "GridMBR.h"

#include <cstddef>
#include <mutex>
#include <vector>
#include <utility>

class Grid: public Representable, public KnnProcessor<PointVectorAccessor> {
public:
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
	/** product of cells up to dimension, necessary for several numerical calculations*/
	const std::vector<std::size_t> productOfCellsUpToDimension_;
	/** the grid is a vector of buckets containing points */
	std::vector<PointContainer> grid_;
	/** locks for multi-threaded insert operation*/
	std::vector<std::mutex*> insertLocks_;
	/** we assume this the optimal number points per cell */
	static const std::size_t CELL_FILL_OPTIMUM_DEFAULT = 1024;

	/** Create an MBR around the grid points. */
	static MBR initGridMBR(double * coordinates, std::size_t dimension,
			std::size_t size);
	/** Insert a set of points into the grid. */
	void insert(double * coordinates, std::size_t size);
	/** Insert single point into grid. */
	void insert(double * point, bool isMultiThreaded);
	/** insert, using locks*/
	void insertMultiThreaded(double* coordinates, std::size_t size);
	/** Calculates gridWith per dimension. */
	const std::vector<double> widthPerDimension();
	/** Returns vector containing number of cells per dimension. */
	//TODO: make static
	const std::vector<std::size_t> calculateCellsPerDimension(
			std::size_t cellFillOptimum) const;
	/** Calculates volume of cells up to a particular dimension. */
	std::vector<std::size_t> initProductOfCellsUpToDimension(
			std::size_t dimension) const;
	/** Calculates the grid index (cell number) for a point. */
	unsigned cellNumber(double * point);
	/** Calculates the grid index (cell number) for a point. */
	unsigned cellNumber(PointAccessor * point);
	/** Allocates memory for grid_ vector. */
	void allocPointContainers();

	/** kNN utility methods: */
	/** Returns squared distance to query point.*/
	double findNextClosestCellBorder(PointAccessor* query, int kNNiteration);
	/** Return a list of cell numbers for certain kNN iteration.*/
	std::vector<unsigned> getHyperSquareCellEnvironment(int kNN_iteration,
			unsigned queryCell, std::vector<unsigned>& cartesianQueryCoords);
	std::vector<unsigned> getCartesian(unsigned cellNumber);
	void initMinAndMax(std::vector<int>& min, std::vector<int>& max,
			int kNN_iteration,
			const std::vector<unsigned>& cartesionQueryCoordinates);
	void addToResult(const std::vector<int>& shifts,
			const std::vector<unsigned>& query,
			std::vector<unsigned>& cellNumbers);
	unsigned calculateCellNumber(const std::vector<int>& gridCartesianCoords);

//public:
	Grid(const std::size_t dimension, double * coordinates, std::size_t size,
			std::size_t cellFillOptimum = Grid::CELL_FILL_OPTIMUM_DEFAULT) :
			dimension_(dimension), mbr_(
					Grid::initGridMBR(coordinates, dimension, size)), numberOfPoints_(
					size / dimension), gridWidthPerDim_(widthPerDimension()), cellsPerDimension_(
					calculateCellsPerDimension(cellFillOptimum)), productOfCellsUpToDimension_(
					initProductOfCellsUpToDimension(dimension)) {

		allocPointContainers();
		insert(coordinates, size);
	}

	/** Returns a vector of the k-nearest neighbors for a given query point. */
	BPQ<PointVectorAccessor> kNearestNeighbors(unsigned k, PointAccessor* query)
			override;
	/** Returns string representation of grid object. */
	void to_stream(std::ostream& os) override;
};

#endif
