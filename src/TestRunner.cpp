#include "grid/MBR.h"
#include "grid/Grid.h"

#include <iostream>
#include <string>
#include <cstdio>

#define NUMBER_OF_POINTS 39999
#define MAX_COORD 1000.0
#define MIN_COORD -1000.0

double fRand(double fMin, double fMax) {
	double f = (double) rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);
}

int main(int argc, char** agrv) {
	std::cout << "Spatial Grid Test Runner...\t" << "running..." << std::endl;

//	double points[9] = { 1.2, 1.3, 1.4, 1.3, 3.2, 4.4, 5.2, 8.3, 10.4 };
//	Grid* g = new Grid(3, points, 9);
//	g->to_stream(std::cout);

	double* randomPtsOut = new double[NUMBER_OF_POINTS];
	double* randomPtsIn = new double[NUMBER_OF_POINTS];

	for (std::size_t i = 0; i < NUMBER_OF_POINTS; i++) {
		double randNr = fRand(MIN_COORD, MAX_COORD);
		randomPtsOut[i] = randNr;

//		std::cout << std::to_string(randNr) << std::endl;
	}

	std::FILE* fout = std::fopen("testPts.bin", "wb");
	std::fwrite(randomPtsOut, sizeof(double), NUMBER_OF_POINTS, fout);
	std::fclose(fout);

	delete(randomPtsOut);
	std::FILE* fin = std::fopen("testPts.bin", "rb");
	std::fread(randomPtsIn, sizeof(double), NUMBER_OF_POINTS, fin);
	std::fclose(fin);

	Grid* g = new Grid(3, randomPtsIn, NUMBER_OF_POINTS);
//	delete(randomPtsIn);
//	delete(g);
//	g->to_stream(std::cout);

	std::cout << "END" << std::endl;
	return 0;
}
