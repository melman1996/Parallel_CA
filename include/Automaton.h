#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <list>
#include <mpi.h>

#include "MPI_defs.h"
#include "Cell.h"

struct AutomatonConfig {
	bool periodic;
	std::string neighbourMethod;
	int xSize;
	int ySize;
	int zSize;
	int randomSeeds;
	int seedCounterStart;
	int overallSeeds;
	int monteCarloIterationsCount;
	double monteCarloKt;
};

class Automaton {
private:
	//MPI
	int world_size, my_rank;
	//CA
	int x_size, y_size, z_size;
	int x_begin, x_end;
	int seedCount;
	int maxSeeds;
	bool periodic;
	std::vector<std::vector<std::vector<Cell>>> currentStep;
	std::vector<std::vector<std::vector<int>>> previousStep;

	std::vector<SingleCell> leftBorder, rightBorder;

	void (Automaton::*Neighbours)(int, int, int, bool);
	void Moore(int, int, int, bool);
	void VonNeumann(int, int, int, bool);

	int calculateNewState(int, int, int);

	void copyToPreviousStep();

	void MonteCarlo(int, double);
	void MonteCarlo(double);
	int calculateEnergy(int, int, int, int);

	//MPI
	void syncProcessesForStructureGenerating();
	bool notifyWorking(bool);
	void sendCell(int, int, int);
	void receiveCells();
public:
	Automaton(int, int, const AutomatonConfig&);

	void generateRandomSeeds(int);

	bool evolve();
	void generateStructure();

	const std::vector<std::vector<std::vector<int>>> getCurrentBoard();
};