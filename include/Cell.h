#pragma once

#include <vector>

class Cell {
private:
	int id;
	std::vector<int> neighbours;
public:
	Cell();
	void addNeighbour(int);

	int get() const;
	void set(int);

	const std::vector<int>& const getNeighbours();
};