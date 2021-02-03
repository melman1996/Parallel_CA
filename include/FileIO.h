#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "Automaton.h"
#include "Cell.h"
#include "Utils.h"

namespace FileIO {
	int WriteBoardToFile(std::string, int, int, int, const std::vector<Cell>&);
	std::vector<std::vector<std::vector<Cell>>> ReadBoardFromFile(std::string);

	AutomatonConfig ReadConfig(std::string);
}