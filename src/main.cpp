#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <mpi.h>

#include "Automaton.h"
#include "FileIO.h"
#include "MPI_defs.h"

int main(int, char* []) {
	MPI_Init(NULL, NULL);

	int my_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	auto start = std::chrono::high_resolution_clock::now();
	AutomatonConfig config;
	if (my_rank == 0) {
		AutomatonConfig orgConfig = FileIO::ReadConfig("config.txt");
		std::vector<AutomatonConfig> configs(world_size, orgConfig);

		int newXSize = std::ceil(orgConfig.xSize / world_size);
		for (int i = 0; i < world_size; i++) { //new xSize
			configs[i].xSize = newXSize;
			if (i == world_size - 1) {
				configs[i].xSize = orgConfig.xSize - ((world_size - 1) * newXSize);
			}
			configs[i].randomSeeds = 0;
		}
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_real_distribution<double> rand(0, world_size);
		for (int i = 0; i < orgConfig.randomSeeds; i++) { //distribute seeds
			configs[rand(mt)].randomSeeds++;
		}
		for (int i = 1; i < world_size; i++) {
			configs[i].seedCounterStart = configs[i - 1].seedCounterStart + configs[i - 1].randomSeeds;
		}
		for (int i = 1; i < world_size; i++) { //send config
			MPI_Send(&configs[i], sizeof(AutomatonConfig), MPI_CHAR, i, Mpi::CONFIG, MPI_COMM_WORLD);
		}
		config = configs[0];
	}
	else {
		MPI_Recv(&config, sizeof(AutomatonConfig), MPI_CHAR, 0, Mpi::CONFIG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	if(my_rank == 0) std::cout << "ReadConfig=" << duration.count() << std::endl;

	auto automaton = std::make_unique<Automaton>(world_size, my_rank, config);

	start = std::chrono::high_resolution_clock::now();
	if (my_rank > 0) {
		std::vector<std::vector<std::vector<int>>> board = automaton->getCurrentBoard();
		std::vector<int> toSend;
		toSend.reserve(board.size() * board[0].size() * board[0][0].size());
		for (const auto& lvl1 : board) {
			for (const auto& lvl2 : lvl1) {
				for (int value : lvl2) {
					toSend.push_back(value);
				}
			}
		}
		int sizeToSend = toSend.size();
		MPI_Send(&sizeToSend, 1, MPI_INT, 0, Mpi::BOARD_SIZE, MPI_COMM_WORLD);
		MPI_Send(&toSend[0], sizeToSend, MPI_INT, 0, Mpi::BOARD, MPI_COMM_WORLD);
	}
	else {
		std::vector<std::vector<std::vector<int>>> board = automaton->getCurrentBoard();
		for (int i = 1; i < world_size; i++) {
			int boardSize;
			MPI_Recv(&boardSize, 1, MPI_INT, i, Mpi::BOARD_SIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			std::vector<int> toSync(boardSize);
			MPI_Recv(&toSync[0], toSync.size(), MPI_INT, i, Mpi::BOARD, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			for (int index = 0; index < toSync.size();) {
				std::vector<std::vector<int>> plane;
				for (int j = 0; j < config.ySize; j++) {
					std::vector<int> row;
					for (int k = 0; k < config.zSize; k++) {
						row.push_back(toSync[index++]);
					}
					plane.push_back(row);
				}
				board.push_back(plane);
			}
		}
		FileIO::WriteBoardToFile("board.txt", board);
	}
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	if (my_rank == 0) std::cout << "WriteToFile=" << duration.count() << std::endl;

	MPI_Finalize();
	return 0;
}