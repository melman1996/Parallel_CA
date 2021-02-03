#pragma once

#include <vector>

class Cell {
private:
	int id = 0;
	std::vector<int> neighbours;
public:
	Cell() = default;
	Cell(const Cell& cell);

	Cell& operator=(const Cell& cell);

	void addNeighbour(int);

	int get() const;
	void set(int);

	const std::vector<int>& getNeighbours() const;
};