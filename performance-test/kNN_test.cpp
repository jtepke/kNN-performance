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

//------------------------------------------------------------------------
//	Global variables - Execution options
//------------------------------------------------------------------------
//Reference parameters
std::size_t dimension;					// dimension
std::size_t numberOfRefPoints = 0;			// reference size
std::size_t numberOfGeneratorThreads = 1; //number of generator threads
unsigned seed;
bool customizedSeed = false;
double refMean = 0.0;					// mean
double refStddev = 1.0;				// standard deviation
unsigned numberOfRefClusters = 1;		// number of clusters
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

enum KNN_APPROACH {
	NAIVE_KNN, GRID_KNN, NAIVE_MAP_REDUCE_KNN, GRID_MAP_REDUCE_KNN
};

bool removeWhiteSpace(				// skip whitespace and comments
		std::istream &in) {
	char ch = 0;
	// skip whitespace
	do {
		in.get(ch);
	} while (isspace(ch) && !in.eof());
	while (ch == '#' && !in.eof()) {		// is comment?
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

//------------------------------------------------------------------------
// nextToken: get next keyword, skip whitespace and comments
// Returns true if token could be read and false if input stream is empty.
//------------------------------------------------------------------------
bool nextToken(std::istream &in,			// input stream
		char* token)		// token storage
		{

	if (!removeWhiteSpace(in)) {		// skip comments
		return false;			// found EOF?
	}

	in >> token;			// read token
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
			std::cout << ", max. number of threads = "
					<< gridMaxNumberOfInsertThreads;
		}
		std::cout << ") ... this may take a while ..." << std::endl;
	}

	if (grid) {
		delete (grid);
		grid = nullptr;
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

//------------------------------------------------------------------------
// binary search thresholds for amount of reference points processable
// within given real-time criterion.
//------------------------------------------------------------------------
std::size_t binarySearchThreshold(unsigned realTimeCrit,
		PointContainer& refPoints, PointContainer& queries, unsigned k,
		KNN_APPROACH strategy, StopWatch& watch, RandomPointGenerator* rpg) {
//TODO: refactor boilerplate.

	std::size_t pointThreshold = refPoints.size();
	long step = pointThreshold / 2;
	//micro seconds
	long epsilon = 10000;
	bool binarySearch;

	NaiveKnn* naiveProcessor = nullptr;
	Grid* gridProcessor = nullptr;
	NaiveMapReduce* naiveMR = nullptr;

	switch (strategy) {
	case NAIVE_KNN:
		binarySearch = false;
		naiveProcessor = new NaiveKnn { refPoints.data(), dimension,
				pointThreshold };
		while (true) {
			StopWatch timeResults = executeKnn(queries, k, naiveProcessor);
			long result = timeResults.averageSplit();
			long timeDiff = realTimeCrit - result;

			std::cout << "time diff: " << timeDiff << std::endl;
			if (std::abs(timeDiff) < epsilon) {
				return pointThreshold;
			} else if (timeDiff > 0 && !binarySearch) {
				//ref point set size not sufficient
				PointContainer newPts = rpg->generatePoints(
						1.5 * refPoints.size(), refDistrib, refMBR, refMean,
						refStddev, numberOfRefClusters);
				refPoints.append(newPts);
				assert(refPoints.size() > pointThreshold);

				pointThreshold = refPoints.size();
				step = pointThreshold / 2;
				delete (naiveProcessor);
				naiveProcessor = new NaiveKnn { refPoints.data(), dimension,
						pointThreshold };
				std::cout << "generating more points: " << pointThreshold
						<< std::endl;
			} else {
				//enough points generated, drop into binary search mode
				binarySearch = true;
				if (timeDiff < 0) {
					pointThreshold -= step;
				} else {
					pointThreshold += step;
				}
				delete (naiveProcessor);
				naiveProcessor = new NaiveKnn { refPoints.data(), dimension,
						pointThreshold };
				step = step / 2;

				if (step == 1) {
					return pointThreshold;
				}
			}
		}
	case GRID_KNN:
		binarySearch = false;
		numberOfRefPoints = pointThreshold;
		gridProcessor = buildUpGrid(gridProcessor, watch, refPoints.data(),
				gridCellSize);
		while (true) {
			StopWatch timeResults = executeKnn(queries, k, gridProcessor);

			long result = timeResults.averageSplit();
			result += watch.getLastSplit();
			long timeDiff = realTimeCrit - result;

			std::cout << "time diff: " << timeDiff << std::endl;
			if (std::abs(timeDiff) < epsilon) {
				return pointThreshold;
			} else if (timeDiff > 0 && !binarySearch) {
				//ref point set size not sufficient
				PointContainer newPts = rpg->generatePoints(
						1.5 * refPoints.size(), refDistrib, refMBR, refMean,
						refStddev, numberOfRefClusters);
				refPoints.append(newPts);
				assert(refPoints.size() > pointThreshold);

				pointThreshold = refPoints.size();
				step = pointThreshold / 2;
				numberOfRefPoints = pointThreshold;
				gridProcessor = buildUpGrid(gridProcessor, watch,
						refPoints.data(), gridCellSize);
				std::cout << "generating more points: " << pointThreshold
						<< std::endl;
			} else {
				//enough points generated, drop into binary search mode
				binarySearch = true;
				if (timeDiff < 0) {
					pointThreshold -= step;
				} else {
					pointThreshold += step;
				}

				numberOfRefPoints = pointThreshold;
				gridProcessor = buildUpGrid(gridProcessor, watch,
						refPoints.data(), gridCellSize);
				step = step / 2;

				if (step == 1) {
					return pointThreshold;
				}
			}
		}
	case NAIVE_MAP_REDUCE_KNN:
		binarySearch = false;

		naiveMR = new NaiveMapReduce { refPoints.data(), dimension,
				pointThreshold, maxNumberOfThreads, maxThreadLoad,
				singleThreadedThreshold, KNN_STRATEGY::NAIVE };
		while (true) {
			std::cout << "start" << std::endl;
			StopWatch timeResults = executeKnn(queries, k, naiveMR);
			std::cout << "stop" << std::endl;

			long result = timeResults.averageSplit();
			long timeDiff = realTimeCrit - result;

			std::cout << "time diff: " << timeDiff << std::endl;
			if (std::abs(timeDiff) < epsilon) {
				return pointThreshold;
			} else if (timeDiff > 0 && !binarySearch) {
				//ref point set size not sufficient
				PointContainer newPts = rpg->generatePoints(
						1.5 * refPoints.size(), refDistrib, refMBR, refMean,
						refStddev, numberOfRefClusters);
				refPoints.append(newPts);
				assert(refPoints.size() > pointThreshold);

				pointThreshold = refPoints.size();
				step = pointThreshold / 2;
				delete (naiveMR);
				naiveMR = new NaiveMapReduce { refPoints.data(), dimension,
						pointThreshold, maxNumberOfThreads, maxThreadLoad,
						singleThreadedThreshold, KNN_STRATEGY::NAIVE };
				std::cout << "generating more points: " << pointThreshold
						<< std::endl;
			} else {
				//enough points generated, drop into binary search mode
				binarySearch = true;
				if (timeDiff < 0) {
					pointThreshold -= step;
				} else {
					pointThreshold += step;
				}

				delete (naiveMR);
				naiveMR = new NaiveMapReduce { refPoints.data(), dimension,
						pointThreshold, maxNumberOfThreads, maxThreadLoad,
						singleThreadedThreshold, KNN_STRATEGY::NAIVE };

				step = step / 2;

				if (step == 1) {
					return pointThreshold;
				}
			}
		}

	case GRID_MAP_REDUCE_KNN:
		binarySearch = false;

		naiveMR = new NaiveMapReduce { refPoints.data(), dimension,
				pointThreshold, maxNumberOfThreads, maxThreadLoad,
				singleThreadedThreshold, KNN_STRATEGY::GRID };
		while (true) {
			StopWatch timeResults = executeKnn(queries, k, naiveMR);

			long result = timeResults.averageSplit();
			long timeDiff = realTimeCrit - result;

			std::cout << "time diff: " << timeDiff << std::endl;
			if (std::abs(timeDiff) < epsilon) {
				return pointThreshold;
			} else if (timeDiff > 0 && !binarySearch) {
				//ref point set size not sufficient
				PointContainer newPts = rpg->generatePoints(
						1.5 * refPoints.size(), refDistrib, refMBR, refMean,
						refStddev, numberOfRefClusters);
				refPoints.append(newPts);
				assert(refPoints.size() > pointThreshold);

				pointThreshold = refPoints.size();
				step = pointThreshold / 2;

				naiveMR = new NaiveMapReduce { refPoints.data(), dimension,
						pointThreshold, maxNumberOfThreads, maxThreadLoad,
						singleThreadedThreshold, KNN_STRATEGY::GRID };
				std::cout << "generating more points: " << pointThreshold
						<< std::endl;
			} else {
				//enough points generated, drop into binary search mode
				binarySearch = true;
				if (timeDiff < 0) {
					pointThreshold -= step;
				} else {
					pointThreshold += step;
				}

				naiveMR = new NaiveMapReduce { refPoints.data(), dimension,
						pointThreshold, maxNumberOfThreads, maxThreadLoad,
						singleThreadedThreshold, KNN_STRATEGY::GRID };

				step = step / 2;

				if (step == 1) {
					return pointThreshold;
				}
			}
		}
	default:
		std::cerr << "Did not recognize kNN strategy. Enum value was: "
				<< strategy << std::endl;
		//ERROR
		return -1;

	}

}

int main(int argc, char** argv) {
	char token[50];			// input token
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
// read input token
	while (nextToken(std::cin, token)) {
		if (!strcmp(token, "k")) {
			std::cin >> k;
		} else if (!strcmp(token, "dimension")) {
			std::cin >> dimension;
		} else if (!strcmp(token, "numberOfRefPoints")) {
			std::cin >> numberOfRefPoints;
		} else if (!strcmp(token, "numberOfGeneratorThreads")) {
			std::cin >> numberOfGeneratorThreads;
		} else if (!strcmp(token, "refMBR")) {
			refMBR = parseMbr(std::cin, dimension);
		} else if (!strcmp(token, "refMean")) {
			std::cin >> refMean;
		} else if (!strcmp(token, "refStddev")) {
			std::cin >> refStddev;
		} else if (!strcmp(token, "numberOfRefClusters")) {
			std::cin >> numberOfRefClusters;
		} else if (!strcmp(token, "seed")) {
			customizedSeed = true;
			std::cin >> seed;
		} else if (!strcmp(token, "refDistribution")) {
			std::cin >> arg;
			if (!strcmp(arg, "uniform")) {
				refDistrib = RandomPointGenerator::UNIFORM;
			} else if (!strcmp(arg, "gauss")) {
				refDistrib = RandomPointGenerator::GAUSS;
			} else if (!strcmp(arg, "gauss_cluster")) {
				refDistrib = RandomPointGenerator::GAUSS_CLUSTER;
			}
		} else if (!strcmp(token, "genReferencePoints")) {
			// generate reference points here:
			if (customizedSeed) {
				rpg = new RandomPointGenerator { seed };
			} else {
				rpg = new RandomPointGenerator { };
			}
			rpg->setNumberOfGeneratorThreads(numberOfGeneratorThreads);
			std::cout
					<< "Generating reference points ... this may take a while..."
					<< std::endl;
			watch.start();
			refPoints = rpg->generatePoints(numberOfRefPoints, refDistrib,
					refMBR, refMean, refStddev, numberOfRefClusters);
			watch.stop();
			std::cout << "Finished reference point generation! ("
					<< watch.getLastSplit() << " micro sec.)\n" << std::endl;
		} else if (!strcmp(token, "writeReferencePointsToFile")) {
			std::string fileName;
			std::cin >> fileName;
			FileHandler::writePointsToFile(fileName, refPoints.data(),
					refPoints.size() * dimension);
			std::cout << "Reference points have been written to file: '"
					<< fileName << "'" << std::endl;
		} else if (!strcmp(token, "writeQueryPointsToFile")) {
			std::string fileName;
			std::cin >> fileName;
			FileHandler::writePointsToFile(fileName, queryPoints.data(),
					queryPoints.size() * dimension);
			std::cout << "Query points have been written to file: '" << fileName
					<< "'" << std::endl;
		} else if (!strcmp(token, "readReferencePointsFromFile")) {
			std::string fileName;
			std::size_t nOfPts;
			std::cin >> fileName;
			std::cin >> nOfPts;
			refPoints = FileHandler::readPointsFromFile(fileName, nOfPts,
					dimension);
			std::cout << refPoints.size()
					<< " reference points have been read from '" << fileName
					<< "'" << std::endl;
		} else if (!strcmp(token, "readQueryPointsFromFile")) {
			std::string fileName;
			std::size_t nOfPts;
			std::cin >> fileName;
			std::cin >> nOfPts;
			queryPoints = FileHandler::readPointsFromFile(fileName, nOfPts,
					dimension);
			std::cout << queryPoints.size()
					<< " query points have been read from '" << fileName << "'"
					<< std::endl;
		} else if (!strcmp(token, "numberOfQueryPoints")) {
			std::cin >> numberOfQueryPoints;
		} else if (!strcmp(token, "queryMBR")) {
			queryMBR = parseMbr(std::cin, dimension);
		} else if (!strcmp(token, "queryMean")) {
			std::cin >> queryMean;
		} else if (!strcmp(token, "queryStddev")) {
			std::cin >> queryStddev;
		} else if (!strcmp(token, "numberOfQueryClusters")) {
			std::cin >> numberOfQueryClusters;
		} else if (!strcmp(token, "queryDistribution")) {
			std::cin >> arg;
			if (!strcmp(arg, "uniform")) {
				queryDistrib = RandomPointGenerator::UNIFORM;
			} else if (!strcmp(arg, "gauss")) {
				queryDistrib = RandomPointGenerator::GAUSS;
			} else if (!strcmp(arg, "gauss_cluster")) {
				queryDistrib = RandomPointGenerator::GAUSS_CLUSTER;
			}
		} else if (!strcmp(token, "genQueryPoints")) {
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
		} else if (!strcmp(token, "gridCellSize")) {
			std::cin >> gridCellSize;
		} else if (!strcmp(token, "kOptimizedGridCells")) {
			gridCellSize = Grid::determineCellSize(k);
		} else if (!strcmp(token, "gridMaxNumberOfThreads")) {
			std::cin >> gridMaxNumberOfInsertThreads;
		} else if (!strcmp(token, "gridThreadLoad")) {
			std::cin >> gridInsertThreadLoad;
		} else if (!strcmp(token, "maxNumberOfThreads")) {
			std::cin >> maxNumberOfThreads;
		} else if (!strcmp(token, "maxThreadLoad")) {
			std::cin >> maxThreadLoad;
		} else if (!strcmp(token, "singleThreadedThreshold")) {
			std::cin >> singleThreadedThreshold;
		} else if (!strcmp(token, "buildGrid")) {
			grid = buildUpGrid(grid, watch, refPoints.data(), gridCellSize);
		} else if (!strcmp(token, "buildNaive")) {
			if (naive) {
				delete (naive);
			}
			naive = new NaiveKnn { refPoints.data(), dimension,
					numberOfRefPoints };
		} else if (!strcmp(token, "buildNaiveMapReduce")) {
			if (naiveMR) {
				delete (naiveMR);
			}
			bool useGrid;
			std::cin >> useGrid;
			if (useGrid) {
				naiveMR = new NaiveMapReduce { refPoints.data(), dimension,
						numberOfRefPoints, maxNumberOfThreads, maxThreadLoad,
						singleThreadedThreshold, KNN_STRATEGY::GRID };
			} else {
				naiveMR = new NaiveMapReduce { refPoints.data(), dimension,
						numberOfRefPoints, maxNumberOfThreads, maxThreadLoad,
						singleThreadedThreshold, KNN_STRATEGY::NAIVE };
			}
		} else if (!strcmp(token, "runGridKnn")) {
			auto gridKnnTime = executeKnn<PointVectorAccessor>(queryPoints, k,
					grid);
			printStats("Spatial Grid", verboseStats, gridKnnTime);
		} else if (!strcmp(token, "runGridCellSizeTest")) {
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
		} else if (!strcmp(token, "runGridBuildUpTest")) {
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
		} else if (!strcmp(token, "runRealTimeTest")) {
			//test various cell sizes:
			//format: runRealTimeTest <real-time-criterion (ms)> <(grid|naive|naiveMR|gridMR)>
			unsigned realTimeCrit = 0;
			unsigned resultPointSetSize = 0;
			std::string kNNApproach;
			std::cin >> realTimeCrit;
			std::cin >> arg;

			//represent time as micro seconds internally
			realTimeCrit *= 1000;
			if (!strcmp(arg, "grid")) {
				kNNApproach = "grid";
				resultPointSetSize = binarySearchThreshold(realTimeCrit,
						refPoints, queryPoints, k, KNN_APPROACH::GRID_KNN,
						watch, rpg);
			} else if (!strcmp(arg, "naive")) {
				kNNApproach = "naive";
				resultPointSetSize = binarySearchThreshold(realTimeCrit,
						refPoints, queryPoints, k, KNN_APPROACH::NAIVE_KNN,
						watch, rpg);
			} else if (!strcmp(arg, "naiveMR")) {
				kNNApproach = "naiveMR";
				resultPointSetSize = binarySearchThreshold(realTimeCrit,
						refPoints, queryPoints, k,
						KNN_APPROACH::NAIVE_MAP_REDUCE_KNN, watch, rpg);
			} else if (!strcmp(arg, "gridMR")) {
				kNNApproach = "gridMR";
				resultPointSetSize = binarySearchThreshold(realTimeCrit,
						refPoints, queryPoints, k,
						KNN_APPROACH::GRID_MAP_REDUCE_KNN, watch, rpg);
			}

			std::cout << "Approach: " << kNNApproach << std::endl;
			std::cout << "Number of points feasible within " << realTimeCrit
					<< " (ms): " << resultPointSetSize << std::endl;

		} else if (!strcmp(token, "runNaiveKnn")) {
			auto naiveKnntime = executeKnn<PointArrayAccessor>(queryPoints, k,
					naive);
			printStats("Naive Approach", verboseStats, naiveKnntime);
		} else if (!strcmp(token, "runNaiveMapReduceKnn")) {
			auto naiveMRtime = executeKnn<PointArrayAccessor>(queryPoints, k,
					naiveMR);
			printStats("Naive Map Reduce Approach", verboseStats, naiveMRtime);
		} else if (!strcmp(token, "verboseStats")) {
			std::cin >> verboseStats;
		} else {
			std::cerr << "Did not recognize: " << token << std::endl;
		}
	}
}
