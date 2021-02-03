#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <list>
#include <omp.h>

#include "Cell.h"

struct AutomatonConfig {
	bool periodic;
	std::string neighbourMethod;
	int xSize;
	int ySize;
	int zSize;
	int randomSeeds;
	int monteCarloIterationsCount;
	double monteCarloKt;
};

class Automaton {
private:
	int size;
	int x_size, y_size, z_size;
	int seedCount;
	bool periodic;
	std::vector<Cell> currentStep;
	std::vector<int> previousStep;

	void (Automaton::*Neighbours)(int, int, int, bool);
	void Moore(int, int, int, bool);
	void VonNeumann(int, int, int, bool);

	int calculateNewState(int);

	int to1D(int, int, int);
	void copyToPreviousStep();

	void MonteCarlo(int, double);
	void MonteCarlo(double);
	int calculateEnergy(int, int);
public:
	Automaton(const AutomatonConfig&);

	void generateRandomSeeds(int);

	bool evolve();
	void generateStructure();

	const std::vector<Cell>& getCurrentBoard();
};