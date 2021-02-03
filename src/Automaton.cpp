#include "Automaton.h"

Automaton::Automaton(int world_size, int my_rank, const AutomatonConfig& config) : 
					x_size(config.xSize), y_size(config.ySize), z_size(config.zSize), 
					seedCount(config.seedCounterStart), maxSeeds(config.overallSeeds), 
					world_size(world_size), my_rank(my_rank)
{
	periodic = config.periodic;

	x_begin = 0;
	x_end = x_size;

	if (periodic) {
		x_size += 2;
		x_begin = 1;
		x_end = x_size - 1;
	}
	else {
		if (my_rank == 0) {
			x_size += 1;
		}
		else if (my_rank == world_size - 1) {
			x_size += 1;
			x_begin = 1;
			x_end = x_size;
		}
		else {
			x_size += 2;
			x_begin = 1;
			x_end = x_size - 1;
		}
	}
	currentStep.resize(x_size, std::vector<std::vector<Cell>>(y_size, std::vector<Cell>(z_size)));
	previousStep.resize(x_size, std::vector<std::vector<int>>(y_size, std::vector<int>(z_size)));

	leftBorder.reserve(y_size * z_size);
	rightBorder.reserve(y_size * z_size);

	if (config.neighbourMethod == "Moore") {
		Neighbours = &Automaton::Moore;
	}
	else {
		Neighbours = &Automaton::VonNeumann;
	}

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
	std::uniform_real_distribution<double> x_dist(x_begin, x_end);
	std::uniform_real_distribution<double> y_dist(0, y_size);
	std::uniform_real_distribution<double> z_dist(0, z_size);

	for (int i = 0; i < count; i++) {
		currentStep[x_dist(mt)][y_dist(mt)][z_dist(mt)].set(++seedCount);
	}
}

bool Automaton::evolve()
{
	bool result = false;
	for (int i = x_begin; i < x_end; i++) {
		for(int j = 0; j < y_size; j++) {
			for (int k = 0; k < z_size; k++) {
				if (currentStep[i][j][k].get() == 0) {
					int newState = calculateNewState(i, j, k);
					if (newState > 0) {
						currentStep[i][j][k].set(newState);
						if (x_begin != 0 && i == 1) {
							leftBorder.emplace_back(j, k, newState);
						}
						else if (x_end != x_size && i == x_end - 1) {
							rightBorder.emplace_back(j, k, newState);
						}
					}
					result = true;
				}
			}
		}
	}
	return result;
}

void Automaton::generateStructure()
{
	bool rv = true;
	if (my_rank == 0) std::cout << "Iterations=" << std::flush;
	auto start_all = std::chrono::high_resolution_clock::now();
	while (rv) {
		auto start = std::chrono::high_resolution_clock::now();
		syncProcessesForStructureGenerating();
		copyToPreviousStep();
		rv = evolve();
		rv = notifyWorking(rv);
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
		if (my_rank == 0) std::cout << duration.count() << "," << std::flush;
	}
	if (my_rank == 0) std::cout << std::endl << std::flush;
	auto stop_all = std::chrono::high_resolution_clock::now();
	auto duration_all = std::chrono::duration_cast<std::chrono::milliseconds>(stop_all - start_all);
	if(my_rank == 0) std::cout << "Structure_generation=" << duration_all.count() << std::endl << std::flush;
}

int Automaton::calculateNewState(int x, int y, int z)
{
	std::vector<int> neighbourTypeCount(maxSeeds + 1);//0 will be dead
	const auto& neighbours = currentStep[x][y][z].getNeighbours();
	
	for (auto & [i, j, k] : neighbours) {
		if (previousStep[i][j][k] > 0) {
			neighbourTypeCount[previousStep[i][j][k]]++;
		}
	}

	int max_index = std::max_element(neighbourTypeCount.begin(), neighbourTypeCount.end()) - neighbourTypeCount.begin();
	if (neighbourTypeCount[max_index] == 0) {//prevent returning weird index when every surrounding cell is empty
		return 0;
	}
	return max_index;
}

void Automaton::copyToPreviousStep()
{
	for (int i = 0; i < x_size; i++) {
		for (int j = 0; j < y_size; j++) {
			for (int k = 0; k < z_size; k++) {
				previousStep[i][j][k] = currentStep[i][j][k].get();
			}
		}
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

				currentStep[x][y][z].addNeighbour(real_i, real_j, real_k);
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
					currentStep[x][y][z].addNeighbour(real_i, real_j, real_k);
				}
			}
		}
	}
}

void Automaton::MonteCarlo(int iterations, double kt)
{
	if(my_rank == 0) std::cout << "MCiterations=";
	auto start_all = std::chrono::high_resolution_clock::now();
	for(int i = 0; i < iterations; i++) {
		auto start = std::chrono::high_resolution_clock::now();
		MonteCarlo(kt);
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
		if (my_rank == 0) std::cout << duration.count() << ",";
	}
	if (my_rank == 0) std::cout << std::endl;
	auto stop_all = std::chrono::high_resolution_clock::now();
	auto duration_all = std::chrono::duration_cast<std::chrono::milliseconds>(stop_all - start_all);
	if (my_rank == 0) std::cout << "MonteCarlo=" << duration_all.count() << std::endl;
}

void Automaton::MonteCarlo(double kt)
{
	std::vector<std::tuple<int, int, int>> cells;

	for (int i = x_begin; i < x_end; i++) {
		for (int j = 0; j < y_size; j++) {
			for (int k = 0; k < z_size; k++) {
				cells.push_back(std::make_tuple(i, j, k));
			}
		}
	}
	std::shuffle(cells.begin(), cells.end(), std::mt19937{ std::random_device{}() });

	for (const auto& [i, j, k]: cells) {
		receiveCells();
		const auto& neighbours = currentStep[i][j][k].getNeighbours();

		int currentEnergy = calculateEnergy(i, j, k, currentStep[i][j][k].get());

		std::vector<int> neighbourStates;

		for (auto & [i, j, k]: neighbours) {
			neighbourStates.push_back(currentStep[i][j][k].get());
		}
		std::unique(neighbourStates.begin(), neighbourStates.end());

		for (const auto& newState : neighbourStates) {
			int alternateEnergy = calculateEnergy(i, j, k, newState);
			if (currentEnergy >= alternateEnergy) {
				currentStep[i][j][k].set(newState);
				sendCell(i, j, k);
				break;
			}
			else {

			}
		}
	}
}

int Automaton::calculateEnergy(int x, int y, int z, int state)
{
	int energy = 0;

	const auto& neighbours = currentStep[x][y][z].getNeighbours();

	for (const auto& [i, j, k] : neighbours) {
		if (currentStep[i][j][k].get() != state) {
			energy++;
		}
	}

	return energy;
}

void Automaton::syncProcessesForStructureGenerating()
{
	MPI_Barrier(MPI_COMM_WORLD);
	if (x_begin != 0) {// send left border
		int leftBorderSize = leftBorder.size();

		int recipient = my_rank - 1;
		if (recipient < 0) recipient = world_size - 1;

		MPI_Send(&leftBorderSize, 1, MPI_INT, recipient, Mpi::LEFT_BORDER_SIZE, MPI_COMM_WORLD);
		MPI_Send(&leftBorder[0], leftBorder.size() * sizeof(SingleCell), MPI_CHAR, recipient, Mpi::LEFT_BORDER, MPI_COMM_WORLD);
	}
	if (x_end != x_size) {// receive right border
		int toSyncSize;
		std::vector<SingleCell> toSync;

		int sender = my_rank + 1;
		if (sender >= world_size) sender = 0;

		MPI_Recv(&toSyncSize, 1, MPI_INT, sender, Mpi::LEFT_BORDER_SIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		toSync.resize(toSyncSize);
		MPI_Recv(&toSync[0], toSync.size() * sizeof(SingleCell), MPI_CHAR, sender, Mpi::LEFT_BORDER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		for (const auto& cell : toSync) {
			currentStep[x_size - 1][cell.y][cell.z].set(cell.state);
		}
	}
	if (x_end != x_size) {// send right border
		int rightBorderSize = rightBorder.size();

		int recipient = my_rank + 1;
		if (recipient >= world_size) recipient = 0;

		MPI_Send(&rightBorderSize, 1, MPI_INT, recipient, Mpi::RIGHT_BORDER_SIZE, MPI_COMM_WORLD);
		MPI_Send(&rightBorder[0], rightBorder.size() * sizeof(SingleCell), MPI_CHAR, recipient, Mpi::RIGHT_BORDER, MPI_COMM_WORLD);
	}
	if (x_begin != 0) {// receive left border
		int toSyncSize;
		std::vector<SingleCell> toSync;

		int sender = my_rank - 1;
		if (sender < 0) sender = world_size - 1;

		MPI_Recv(&toSyncSize, 1, MPI_INT, sender, Mpi::RIGHT_BORDER_SIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		toSync.resize(toSyncSize);
		MPI_Recv(&toSync[0], toSync.size() * sizeof(SingleCell), MPI_CHAR, sender, Mpi::RIGHT_BORDER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		for (const auto& cell : toSync) {
			currentStep[0][cell.y][cell.z].set(cell.state);
		}
	}
	leftBorder.clear();
	rightBorder.clear();
}

bool Automaton::notifyWorking(bool working) {
	bool rv = working;
	if (my_rank != 0) {
		MPI_Send(&working, 1, MPI_C_BOOL, 0, Mpi::IS_WORKING, MPI_COMM_WORLD);
	}
	else {
		for (int i = 1; i < world_size; i++) {
			bool isWorking;
			MPI_Recv(&isWorking, 1, MPI_C_BOOL, i, Mpi::IS_WORKING, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
			if (isWorking) rv = true;
		}
	}
	MPI_Bcast(&rv, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
	return rv;
}

void Automaton::sendCell(int x, int y, int z) {
	SingleCell cell = { y, z, currentStep[x][y][z].get() };
	if (x == x_begin && x_begin != 0) {
		int recipient = my_rank - 1;
		if (recipient < 0) recipient = world_size - 1;
		MPI_Send(&cell, sizeof(SingleCell), MPI_CHAR, recipient, Mpi::SINGLE_CELL, MPI_COMM_WORLD);
	}
	if (x == x_end - 1 && x_end != x_size) {
		int recipient = my_rank + 1;
		if (recipient >= world_size) recipient = 0;
		MPI_Send(&cell, sizeof(SingleCell), MPI_CHAR, recipient, Mpi::SINGLE_CELL, MPI_COMM_WORLD);
	}
}

void Automaton::receiveCells()
{
	while (true) {
		MPI_Status status;
		int flag = 0;
		MPI_Iprobe(MPI_ANY_SOURCE, Mpi::SINGLE_CELL, MPI_COMM_WORLD, &flag, &status);
		if (!flag) break; //no messages - exit
		int sender = status.MPI_SOURCE;
		SingleCell cell;
		MPI_Recv(&cell, sizeof(SingleCell), MPI_CHAR, sender, Mpi::SINGLE_CELL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		int left = my_rank - 1;
		if (left < 0) left = world_size - 1;
		int right = my_rank + 1;
		if (my_rank >= world_size - 1) right = 0;
		if (sender == left) {
			currentStep[0][cell.y][cell.z].set(cell.state);
		}
		else if (sender == right) {
			currentStep[x_size - 1][cell.y][cell.z].set(cell.state);
		}
		else {
			std::cout << "ERROR " << std::endl << std::flush;
		}
	}
}


const std::vector<std::vector<std::vector<int>>> Automaton::getCurrentBoard()
{
	std::vector<std::vector<std::vector<int>>> toReturn(x_end - x_begin, std::vector<std::vector<int>>(y_size, std::vector<int>(z_size)));
	for (int i = x_begin; i < x_end; i++) {
		for (int j = 0; j < y_size; j++) {
			for (int k = 0; k < z_size; k++) {
				toReturn[i - x_begin][j][k] = currentStep[i][j][k].get();
			}
		}
	}
	return std::move(toReturn);
}
