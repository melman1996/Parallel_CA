#include "Automaton.h"

Automaton::Automaton(const AutomatonConfig& config) : x_size(config.xSize), y_size(config.ySize), z_size(config.zSize), seedCount(0)
{
	size = x_size * y_size * z_size;
	currentStep.resize(size);
	previousStep.resize(size);
	if (config.neighbourMethod == "Moore") {
		Neighbours = &Automaton::Moore;
	}
	else {
		Neighbours = &Automaton::VonNeumann;
	}

	periodic = config.periodic;

	for (int i = 0; i < x_size; i++) {
		for (int j = 0; j < y_size; j++) {
			for (int k = 0; k < z_size; k++) {
				(this->*Neighbours)(i, j, k, periodic);
			}
		}
	}


	generateRandomSeeds(config.randomSeeds);
	generateStructure();
	MonteCarlo(config.monteCarloIterationsCount, config.monteCarloKt);
}

void Automaton::generateRandomSeeds(int count)
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<double> dist(0, size);

	for (int i = 0; i < count; i++) {
		currentStep[dist(mt)].set(++seedCount);
	}
}

bool Automaton::evolve()
{
	bool result = false;
	copyToPreviousStep();
	std::for_each(
		std::execution::par,
		currentStep.begin(),
		currentStep.end(),
		[this, &result](auto&& item) {
			if (item.get() == 0) {
				item.set(calculateNewState(item));
				result = true;
			}
		}
	);
	return result;
}

void Automaton::generateStructure()
{
	bool rv = true;
	std::cout << "Iterations=";
	auto start_all = std::chrono::high_resolution_clock::now();
	while (rv) {
		auto start = std::chrono::high_resolution_clock::now();
		rv = evolve();
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
		std::cout << duration.count() << ",";
	}
	std::cout << std::endl;
	auto stop_all = std::chrono::high_resolution_clock::now();
	auto duration_all = std::chrono::duration_cast<std::chrono::milliseconds>(stop_all - start_all);
	std::cout << "Structure_generation=" << duration_all.count() << std::endl;
}

int Automaton::calculateNewState(int index)
{
	std::vector<int> neighbourTypeCount(seedCount + 1);//0 will be dead
	const auto& neighbours = currentStep[index].getNeighbours();
	
	for (auto & i : neighbours) {
		if (previousStep[i] > 0) {
			neighbourTypeCount[previousStep[i]]++;
		}
	}

	int max_index = std::max_element(neighbourTypeCount.begin(), neighbourTypeCount.end()) - neighbourTypeCount.begin();
	if (neighbourTypeCount[max_index] == 0) {//prevent returning weird index when every surrounding cell is empty
		return 0;
	}
	return max_index;
}

int Automaton::calculateNewState(Cell& cell)
{
	std::vector<int> neighbourTypeCount(seedCount + 1);//0 will be dead
	const auto& neighbours = cell.getNeighbours();

	for (auto& i : neighbours) {
		if (previousStep[i] > 0) {
			neighbourTypeCount[previousStep[i]]++;
		}
	}

	int max_index = std::max_element(neighbourTypeCount.begin(), neighbourTypeCount.end()) - neighbourTypeCount.begin();
	if (neighbourTypeCount[max_index] == 0) {//prevent returning weird index when every surrounding cell is empty
		return 0;
	}
	return max_index;
}

int Automaton::to1D(int x, int y, int z)
{
	return (z * x_size * y_size) + (y * x_size) + x;
}

void Automaton::copyToPreviousStep()
{
	for (int i = 0; i < size; i++) {
		previousStep[i] = currentStep[i].get();
	}
}

void Automaton::Moore(int x, int y, int z, bool periodic)
{
	for (int i = x - 1; i <= x + 1; i++) {
		int real_i = i;
		if (periodic) {
			real_i = (real_i + x_size) % x_size;
		}
		else if (real_i < 0 || real_i >= x_size) continue;

		for (int j = y - 1; j <= y + 1; j++) {
			int real_j = j;
			if (periodic) {
				real_j = (real_j + y_size) % y_size;
			}
			else if (real_j < 0 || real_j >= y_size) continue;

			for (int k = z - 1; k <= z + 1; k++) {
				int real_k = k;
				if (periodic) {
					real_k = (real_k + z_size) % z_size;
				}
				else if (real_k < 0 || real_k >= z_size) continue;

				if (i == x && j == y && k == z) continue;

				currentStep[to1D(x, y, z)].addNeighbour(to1D(real_i, real_j, real_k));
			}
		}
	}
}

void Automaton::VonNeumann(int x, int y, int z, bool periodic)
{
	for (int i = x - 1; i <= x + 1; i++) {
		int real_i = i;
		if (periodic) {
			real_i = (real_i + x_size) % x_size;
		}
		else if (real_i < 0 || real_i >= x_size) continue;

		for (int j = y - 1; j <= y + 1; j++) {
			int real_j = j;
			if (periodic) {
				real_j = (real_j + y_size) % y_size;
			}
			else if (real_j < 0 || real_j >= y_size) continue;

			for (int k = z - 1; k <= z + 1; k++) {
				int real_k = k;
				if (periodic) {
					real_k = (real_k + z_size) % z_size;
				}
				else if (real_k < 0 || real_k >= z_size) continue;

				int distance = std::abs(x - i) + std::abs(y - j) + std::abs(z - k);
				if (distance == 1) {
					currentStep[to1D(x, y, z)].addNeighbour(to1D(real_i, real_j, real_k));
				}
			}
		}
	}
}

void Automaton::MonteCarlo(int iterations, double kt)
{
	std::cout << "MCiterations=";
	auto start_all = std::chrono::high_resolution_clock::now();
	for(int i = 0; i < iterations; i++) {
		auto start = std::chrono::high_resolution_clock::now();
		MonteCarlo(kt);
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
		std::cout << duration.count() << ",";
	}
	std::cout << std::endl;
	auto stop_all = std::chrono::high_resolution_clock::now();
	auto duration_all = std::chrono::duration_cast<std::chrono::milliseconds>(stop_all - start_all);
	std::cout << "MonteCarlo=" << duration_all.count() << std::endl;
}

void Automaton::MonteCarlo(double kt)
{
	std::vector<int> cells;

	for (int i = 0; i < size; i++) {
		cells.push_back(i);
	}
	std::shuffle(cells.begin(), cells.end(), std::mt19937{ std::random_device{}() });

	std::for_each(
		std::execution::par,
		cells.begin(),
		cells.end(),
		[this](auto&& index) {
			const auto& neighbours = currentStep[index].getNeighbours();

			int currentEnergy = calculateEnergy(index, currentStep[index].get());

			std::vector<int> neighbourStates;

			for (auto& i : neighbours) {
				neighbourStates.push_back(currentStep[i].get());
			}
			std::unique(neighbourStates.begin(), neighbourStates.end());

			for (const auto& newState : neighbourStates) {
				int alternateEnergy = calculateEnergy(index, newState);
				if (currentEnergy >= alternateEnergy) {
					currentStep[index].set(newState);
					break;
				}
				else {

				}
			}
		});
}

int Automaton::calculateEnergy(int index, int state)
{
	int energy = 0;

	const auto& neighbours = currentStep[index].getNeighbours();

	for (const auto& i : neighbours) {
		if (currentStep[i].get() != state) {
			energy++;
		}
	}

	return energy;
}

const std::vector<Cell>& Automaton::getCurrentBoard()
{
	return currentStep;
}
