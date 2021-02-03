#pragma once

#include <vector>

class Cell {
private:
	int id;
	std::vector<std::tuple<int, int, int>> neighbours;
public:
	Cell();
	void addNeighbour(int, int, int);

	int get() const;
	void set(int);

	const std::vector<std::tuple<int, int, int>>& const getNeighbours();
};