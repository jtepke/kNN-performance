#include "grid/GridMBR.h"
#include "grid/Grid.h"
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>

#include "util/FileHandler.h"
#include "util/RandomPointGenerator.h"

#define MAX_COORD 1000.0
#define MIN_COORD -1000.0

double fRand(double fMin, double fMax) {
	double f = (double) rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);
}

int main(int argc, char** argv) {
	std::cout << "Spatial Grid Test Runner...\t" << "running..." << std::endl;

	int NUMBER_OF_POINTS = std::stoi(argv[1]);
	int DIMENSION = std::stoi(argv[2]);
	int NUMBER_OF_COORDS = NUMBER_OF_POINTS * DIMENSION;

	double mbrCoords[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 4.0, 4.0, 4.0,
			4.0, 4.0 };

	GridMBR m = GridMBR::buildMBR(mbrCoords, 12, 6);

	m.to_stream(std::cout);

	RandomPointGenerator g(12345);

	double* randomPtsOut = g.generatePoints(NUMBER_OF_POINTS,
			RandomPointGenerator::GAUSS_CLUSTER, m, 5.0, 2.0, 4);
//	double* randomPtsIn = new double[NUMBER_OF_COORDS];

//	for (int i = 0; i < NUMBER_OF_COORDS; i++) {
//		double randNr = fRand(MIN_COORD, MAX_COORD);
//		randomPtsOut[i] = randNr;
//	}

	PointContainer p(DIMENSION);
	p.add(randomPtsOut, NUMBER_OF_COORDS);
	p.to_stream(std::cout);

//	FileHandler::writePointsToFile("testPts.bin", randomPtsOut,
//			NUMBER_OF_COORDS);
//
//	delete[] (randomPtsOut);
//	FileHandler::readPointsFromFile("testPts.bin", randomPtsIn,
//			NUMBER_OF_POINTS, DIMENSION);
//	Grid* g = new Grid(DIMENSION, randomPtsIn, NUMBER_OF_COORDS);
//	delete[] (randomPtsIn);
//
//	g->to_stream(std::cout);
//	delete (g);

	std::cout << "END" << std::endl;
	return 0;
}
