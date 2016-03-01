#include "../src/grid/Grid.h"
#include "../src/knn/BPQ.h"
#include "../src/knn/NaiveKnn.h"
#include "../src/knn/KnnProcessor.h"
#include "../src/model/PointContainer.h"
#include "../src/model/PointArrayAccessor.h"
#include "../src/naive-map-reduce/NaiveMapReduce.h"
#include "../src/util/RandomPointGenerator.h"
#include "../src/util/FileHandler.h"
#include "../src/util/StopWatch.h"

#include <cstddef>
#include <cstring>
#include <iostream>
#include <istream>

// Global time

//------------------------------------------------------------------------
//	Global variables - Execution options
//------------------------------------------------------------------------
//Reference parameters
std::size_t dimension;					// dimension
std::size_t numberOfRefPoints = 0;			// reference size
unsigned seed;
bool customizedSeed = false;
double refMean = 0.0;					// mean
double refStddev = 1.0;				// standard deviation
unsigned numberOfRefClusters = 1;		// number of colors
MBR refMBR { 1 };						// reference points bounds
RandomPointGenerator::DISTRIBUTION refDistrib;

//Grid parameters
std::size_t gridCellSize = Grid::CELL_FILL_OPTIMUM_DEFAULT;	// grid bucket size
unsigned gridMaxNumberOfInsertThreads = Grid::MAX_NUMBER_OF_THREADS_DEFAULT;
unsigned gridInsertThreadLoad = Grid::THREAD_LOAD_DEFAULT;

//Naive MapReduce parameters
unsigned maxNumberOfThreads = NaiveMapReduce::MAX_NUMBER_OF_THREADS;
unsigned maxThreadLoad = NaiveMapReduce::MAX_THREAD_LOAD;
unsigned singleThreadedThreshold = NaiveMapReduce::SINGLE_THREADED_THRESHOLD;

//Query parameters
std::size_t numberOfQueryPoints = 0;			// query size
double queryMean = 0.0;				// mean
double queryStddev = 1.0;				// standard deviation
int numberOfQueryClusters = 1;		// number of colors
MBR queryMBR { 1 };						// reference points bounds
RandomPointGenerator::DISTRIBUTION queryDistrib;

//Run kNN Query:
unsigned k = 1;						// number of near neighbors
bool verboseStats = false;
//------------------------------------------------------------------------
// getDirective - skip comments and read next directive
//	Returns true if directive read, and false if EOF seen.
//------------------------------------------------------------------------

bool skipComment(				// skip any comments
		std::istream &in)				// input stream
		{
	char ch = 0;
	// skip whitespace
	do {
		in.get(ch);
	} while (isspace(ch) && !in.eof());
	while (ch == '#' && !in.eof()) {		// comment?
		// skip to end of line
		do {
			in.get(ch);
		} while (ch != '\n' && !in.eof());
		// skip whitespace
		do {
			in.get(ch);
		} while (isspace(ch) && !in.eof());
	}
	if (in.eof()) {
		return false; // end of file
	}
	in.putback(ch);				// put character back
	return true;
}

bool getDirective(std::istream &in,			// input stream
		char* directive)		// directive storage
		{
	if (!skipComment(in))			// skip comments
		return false;			// found EOF along the way?
	in >> directive;				// read directive
	return true;
}

MBR parseMbr(std::istream& in, std::size_t dimension) {
	std::size_t mbrSize = 2 * dimension;
	double coords[mbrSize];
	MBR m { dimension };
	for (std::size_t i = 0; i < mbrSize; ++i) {
		in >> coords[i];
	}

	return m.createMBR(coords, 2 * dimension);
}

template<class T>
StopWatch executeKnn(PointContainer& queries, unsigned k,
		KnnProcessor<T>* processor) {

	StopWatch watch;
	watch.start();

	for (size_t i = 0; i < queries.size(); ++i) {
		PointVectorAccessor query = queries[i];
		processor->kNearestNeighbors(k, &query);
	}
	watch.stop();
	return watch;
}

void printStats(const std::string & indexName, bool verbose, StopWatch& watch) {
	std::cout << "Finished kNN (k=" << k << ") lookup on " << indexName << "\n";
	std::cout << "Run queries: " << numberOfQueryPoints << "\n";
	std::cout << "Indexed points: " << numberOfRefPoints << "\n";

	auto sumRuntimes = watch.getLastSplit();
	auto avgRuntime = sumRuntimes / numberOfQueryPoints;

	std::cout << "Query avg. runtime (micro sec.): " << avgRuntime << "\n";
	std::cout << "Query whole runtime (micro sec.): " << sumRuntimes << "\n";
	if (verbose) {
		std::cout << "All query runtimes (micro sec.):\n";
		for (auto split : watch.getSplitTimes()) {
			std::cout << split << '\n';
		}
	}

	std::cout << std::endl;
}

Grid* buildUpGrid(Grid* grid, StopWatch& watch, double* refPtsArray,
		std::size_t cellSize, bool printCSV = false) {
	if (!printCSV) {
		std::cout << "Building grid index (cell size = " << cellSize;
		if (numberOfRefPoints > gridInsertThreadLoad) {
			std::cout << ", max. number of threads = " << maxNumberOfThreads;
		}
		std::cout << ") ... this may take a while ..." << std::endl;
	}
	if (grid) {
		delete (grid);
	}
	watch.start();
	grid = new Grid { dimension, refPtsArray, numberOfRefPoints * dimension,
			cellSize, gridMaxNumberOfInsertThreads, gridInsertThreadLoad };
	watch.stop();
	if (!printCSV) {
		std::cout << "Finished grid construction! (" << watch.getLastSplit()
				<< " micro sec.)\n" << std::endl;
	} else {
		std::cout << cellSize << ',' << watch.getLastSplit() << std::endl;
	}
	return grid;
}

int main(int argc, char** argv) {
	char directive[50];		// input directive
	char arg[50];			// all-purpose argument
	RandomPointGenerator* rpg = nullptr;
	Grid* grid = nullptr;
	NaiveKnn* naive = nullptr;
	NaiveMapReduce* naiveMR = nullptr;

	StopWatch watch { };
	PointContainer refPoints;
	PointContainer queryPoints;

	std::cout
			<< "###############################################################\n"
			<< "kNN Tester: Please specify input \n"
			<< "###############################################################\n\n";

//--------------------------------------------------------------------
//	Main input loop
//--------------------------------------------------------------------
// read input directive
	while (getDirective(std::cin, directive)) {
		if (!strcmp(directive, "k")) {
			std::cin >> k;
		} else if (!strcmp(directive, "dimension")) {
			std::cin >> dimension;
		} else if (!strcmp(directive, "numberOfRefPoints")) {
			std::cin >> numberOfRefPoints;
		} else if (!strcmp(directive, "refMBR")) {
			refMBR = parseMbr(std::cin, dimension);
		} else if (!strcmp(directive, "refMean")) {
			std::cin >> refMean;
		} else if (!strcmp(directive, "refStddev")) {
			std::cin >> refStddev;
		} else if (!strcmp(directive, "numberOfRefClusters")) {
			std::cin >> numberOfRefClusters;
		} else if (!strcmp(directive, "seed")) {
			customizedSeed = true;
			std::cin >> seed;
		} else if (!strcmp(directive, "refDistribution")) {
			std::cin >> arg;
			if (!strcmp(arg, "uniform")) {
				refDistrib = RandomPointGenerator::UNIFORM;
			} else if (!strcmp(arg, "gauss")) {
				refDistrib = RandomPointGenerator::GAUSS;
			} else if (!strcmp(arg, "gauss_cluster")) {
				refDistrib = RandomPointGenerator::GAUSS_CLUSTER;
			}
		} else if (!strcmp(directive, "genReferencePoints")) {
			// generate reference points here:
			if (customizedSeed) {
				rpg = new RandomPointGenerator { seed };
			} else {
				rpg = new RandomPointGenerator { };
			}
			std::cout
					<< "Generating reference points ... this may take a while..."
					<< std::endl;
			watch.start();
			refPoints = rpg->generatePoints(numberOfRefPoints, refDistrib,
					refMBR, refMean, refStddev, numberOfRefClusters);
			watch.stop();
			std::cout << "Finished reference point generation! ("
					<< watch.getLastSplit() << " micro sec.)\n" << std::endl;
		} else if (!strcmp(directive, "writeReferencePointsToFile")) {
			std::string fileName;
			std::cin >> fileName;
			FileHandler::writePointsToFile(fileName, refPoints.data(),
					refPoints.size() * dimension);
			std::cout << "Reference points have been written to file: '"
					<< fileName << "'" << std::endl;
		} else if (!strcmp(directive, "writeQueryPointsToFile")) {
			std::string fileName;
			std::cin >> fileName;
			FileHandler::writePointsToFile(fileName, queryPoints.data(),
					queryPoints.size() * dimension);
			std::cout << "Query points have been written to file: '" << fileName
					<< "'" << std::endl;
		} else if (!strcmp(directive, "readReferencePointsFromFile")) {
			std::string fileName;
			std::size_t nOfPts;
			std::cin >> fileName;
			std::cin >> nOfPts;
			refPoints = FileHandler::readPointsFromFile(fileName, nOfPts,
					dimension);
			std::cout << refPoints.size()
					<< " reference points have been read from '" << fileName
					<< "'" << std::endl;
		} else if (!strcmp(directive, "readQueryPointsFromFile")) {
			std::string fileName;
			std::size_t nOfPts;
			std::cin >> fileName;
			std::cin >> nOfPts;
			queryPoints = FileHandler::readPointsFromFile(fileName, nOfPts,
					dimension);
			std::cout << queryPoints.size()
					<< " query points have been read from '" << fileName << "'"
					<< std::endl;
		} else if (!strcmp(directive, "numberOfQueryPoints")) {
			std::cin >> numberOfQueryPoints;
		} else if (!strcmp(directive, "queryMBR")) {
			queryMBR = parseMbr(std::cin, dimension);
		} else if (!strcmp(directive, "queryMean")) {
			std::cin >> queryMean;
		} else if (!strcmp(directive, "queryStddev")) {
			std::cin >> queryStddev;
		} else if (!strcmp(directive, "numberOfQueryClusters")) {
			std::cin >> numberOfQueryClusters;
		} else if (!strcmp(directive, "queryDistribution")) {
			std::cin >> arg;
			if (!strcmp(arg, "uniform")) {
				queryDistrib = RandomPointGenerator::UNIFORM;
			} else if (!strcmp(arg, "gauss")) {
				queryDistrib = RandomPointGenerator::GAUSS;
			} else if (!strcmp(arg, "gauss_cluster")) {
				queryDistrib = RandomPointGenerator::GAUSS_CLUSTER;
			}
		} else if (!strcmp(directive, "genQueryPoints")) {
			// generate query points here:
			if (customizedSeed) {
				rpg = new RandomPointGenerator { seed };
			} else {
				rpg = new RandomPointGenerator { };
			}
			std::cout << "Generating query points ... this may take a while..."
					<< std::endl;
			watch.start();
			queryPoints = rpg->generatePoints(numberOfQueryPoints, queryDistrib,
					queryMBR, queryMean, queryStddev, numberOfQueryClusters);
			watch.stop();
			std::cout << "Finished query point generation! ("
					<< watch.getLastSplit() << " micro sec.)\n" << std::endl;
		} else if (!strcmp(directive, "gridCellSize")) {
			std::cin >> gridCellSize;
		} else if (!strcmp(directive, "kOptimizedGridCells")) {
			gridCellSize = Grid::determineCellSize(k);
		} else if (!strcmp(directive, "gridMaxNumberOfThreads")) {
			std::cin >> gridMaxNumberOfInsertThreads;
		} else if (!strcmp(directive, "gridThreadLoad")) {
			std::cin >> gridInsertThreadLoad;
		} else if (!strcmp(directive, "maxNumberOfThreads")) {
			std::cin >> maxNumberOfThreads;
		} else if (!strcmp(directive, "maxThreadLoad")) {
			std::cin >> maxThreadLoad;
		} else if (!strcmp(directive, "singleThreadedThreshold")) {
			std::cin >> singleThreadedThreshold;
		} else if (!strcmp(directive, "buildGrid")) {
			grid = buildUpGrid(grid, watch, refPoints.data(), gridCellSize);
		} else if (!strcmp(directive, "buildNaive")) {
			if (naive) {
				delete (naive);
			}
			naive = new NaiveKnn { refPoints.data(), dimension,
					numberOfRefPoints };
		} else if (!strcmp(directive, "buildNaiveMapReduce")) {
			if (naiveMR) {
				delete (naiveMR);
			}
			naiveMR = new NaiveMapReduce { refPoints.data(), dimension,
					numberOfRefPoints, maxNumberOfThreads, maxThreadLoad,
					singleThreadedThreshold };
		} else if (!strcmp(directive, "runGridKnn")) {
			auto gridKnnTime = executeKnn<PointVectorAccessor>(queryPoints, k,
					grid);
			printStats("Spatial Grid", verboseStats, gridKnnTime);
		} else if (!strcmp(directive, "runGridCellSizeTest")) {
			//format: runGridCellSizeTest <start> <end> <step size>
			unsigned start = 0;
			unsigned end = 0;
			unsigned stepSize = 0;
			std::cin >> start;
			std::cin >> end;
			std::cin >> stepSize;

			while (start < end) {
				grid = buildUpGrid(grid, watch, refPoints.data(), start);

				auto gridKnnTime = executeKnn<PointVectorAccessor>(queryPoints,
						k, grid);
				printStats("Spatial Grid", verboseStats, gridKnnTime);
				start += stepSize;
			}
			auto gridKnnTime = executeKnn<PointVectorAccessor>(queryPoints, k,
					grid);
			printStats("Spatial Grid", verboseStats, gridKnnTime);
		} else if (!strcmp(directive, "runGridBuildUpTest")) {
			//test various cell sizes:
			//format: runGridBuildUpTest <start> <end> <step size> <output csv>
			unsigned start = 0;
			unsigned end = 0;
			unsigned stepSize = 0;
			bool outputCSV;
			std::cin >> start;
			std::cin >> end;
			std::cin >> stepSize;
			std::cin >> outputCSV;

			while (start <= end) {
				grid = buildUpGrid(grid, watch, refPoints.data(), start,
						outputCSV);
				start += stepSize;
			}
		} else if (!strcmp(directive, "runNaiveKnn")) {
			auto naiveKnntime = executeKnn<PointArrayAccessor>(queryPoints, k,
					naive);
			printStats("Naive Approach", verboseStats, naiveKnntime);
		} else if (!strcmp(directive, "runNaiveMapReduceKnn")) {
			auto naiveMRtime = executeKnn<PointArrayAccessor>(queryPoints, k,
					naiveMR);
			printStats("Naive Map Reduce Approach", verboseStats, naiveMRtime);
		} else if (!strcmp(directive, "verboseStats")) {
			std::cin >> verboseStats;
		} else {
			std::cerr << "Did not recognize: " << directive << std::endl;
		}
	}
}
