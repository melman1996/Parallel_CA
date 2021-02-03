#include "Cell.h"

Cell::Cell() : id(0)
{
}

void Cell::addNeighbour(int index)
{
	neighbours.push_back(index);
}

int Cell::get() const
{
	return id;
}

void Cell::set(int newId)
{
	id = newId;
}

const std::vector<int>& const Cell::getNeighbours()
{
	return neighbours;
}
