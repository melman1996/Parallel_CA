#include <iostream>
#include <memory>
#include <vector>
#include <chrono>

#include "Automaton.h"
#include "FileIO.h"

int main(int, char* []) {
	auto start = std::chrono::high_resolution_clock::now();
	AutomatonConfig config = FileIO::ReadConfig("config.txt");
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "ReadConfig=" << duration.count() << std::endl;

	start = std::chrono::high_resolution_clock::now();
	auto automaton = std::make_unique<Automaton>(config);
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "AllBoard=" << duration.count() << std::endl;

	start = std::chrono::high_resolution_clock::now();
	FileIO::WriteBoardToFile("board.txt", config.xSize, config.ySize, config.zSize, automaton->getCurrentBoard());
	stop = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	std::cout << "WriteToFile=" << duration.count() << std::endl;

	return 0;
}