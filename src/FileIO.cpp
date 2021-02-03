#include "FileIO.h"

int FileIO::WriteBoardToFile(std::string fileName, const std::vector<std::vector<std::vector<int>>>& board) {//TODO: maybe use templates	int FileIO::WriteBoardToFile(std::string fileName, int x, int y, int z, const std::vector<Cell> &board) {//TODO: maybe use templates
	int x = board.size(); //TODO: wrong asssumption that vectors are not empty	
	int y = board[0].size();
	int z = board[0][0].size();

	std::fstream file;
	file.open(fileName, std::ios::out);
	file << x << "x" << y << "x" << z << std::endl;
	for (int i = 0; i < x; i++) {
		for (int j = 0; j < y; j++) {
			for (int k = 0; k < z; k++) {
				file << board[i][j][k] << ",";
			}
		}
	}
	file.close();
	return 0;
}

std::vector<std::vector<std::vector<Cell>>> FileIO::ReadBoardFromFile(std::string fileName) {//TODO: fix that
	std::fstream file;
	file.open(fileName, std::ios::in);

	std::string line;

	std::getline(file, line);
	std::vector<std::string> size = Utils::SplitString(line, "x");//TODO: wrong assumption that the file is correct
	std::vector<std::vector<std::vector<Cell>>> board(std::stoi(size[0]), std::vector<std::vector<Cell>>(std::stoi(size[1]), std::vector<Cell>(std::stoi(size[2]))));

	std::getline(file, line);
	std::vector<std::string> values = Utils::SplitString(line, ",");
	for (int i = 0; i < board.size(); i++) {
		for (int j = 0; j < board[i].size(); j++) {
			for (int k = 0; k < board[i][j].size(); k++) {
				board[i][j][k].set(std::stoi(values[(i * board.size() + j) * board[i].size() + k]));
			}
		}
	}
	file.close();

	return board;
}

AutomatonConfig FileIO::ReadConfig(std::string fileName) {
	AutomatonConfig config = {false, "Moore", 10, 10, 10, 10, 0, 0.6};

	std::fstream file;
	file.open(fileName, std::ios::in);

	std::string line;

	while (std::getline(file, line)) {
		auto splitLine = Utils::SplitString(line, "=");
		if (splitLine[0] == "periodic") {
			if (splitLine[1] == "yes") {
				config.periodic = true;
			}
			else if (splitLine[1] == "no") {
				config.periodic = false;
			}
			else {
				std::cout << "config value '" << splitLine[1] << "' for 'periodic' not supported\n";
			}
		}
		else if (splitLine[0] == "method") {
			config.neighbourMethod = splitLine[1];
		}
		else if (splitLine[0] == "x_size") {
			config.xSize = std::stoi(splitLine[1]);
		}
		else if (splitLine[0] == "y_size") {
			config.ySize = std::stoi(splitLine[1]);
		}
		else if (splitLine[0] == "z_size") {
			config.zSize = std::stoi(splitLine[1]);
		}
		else if (splitLine[0] == "random_seeds") {
			config.randomSeeds = std::stoi(splitLine[1]);
			config.overallSeeds = std::stoi(splitLine[1]);
		}
		else if (splitLine[0] == "MC_iterations") {
			config.monteCarloIterationsCount = std::stoi(splitLine[1]);
		}
		else if (splitLine[0] == "MC_kt") {
			config.monteCarloKt = std::stoi(splitLine[1]);
		}
		else {
			std::cout << "config '" << line << "' not supported\n";
		}
	}

	return config;
}