#include "Cell.h"

Cell::Cell() : id(0)
{
}

void Cell::addNeighbour(int i, int j, int k)
{
	neighbours.push_back(std::make_tuple(i, j, k));
}

int Cell::get() const
{
	return id;
}

void Cell::set(int newId)
{
	id = newId;
}

const std::vector<std::tuple<int, int, int>>& const Cell::getNeighbours()
{
	return neighbours;
}
