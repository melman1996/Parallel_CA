#include "Cell.h"

void Cell::addNeighbour(int index)
{
	neighbours.push_back(index);
}

Cell::Cell(const Cell& cell)
{
	id = cell.id;
	neighbours = cell.neighbours;
}

Cell& Cell::operator=(const Cell& cell)
{
	id = cell.id;
	neighbours = cell.neighbours;
	return *this;
}

int Cell::get() const
{
	return id;
}

void Cell::set(int newId)
{
	id = newId;
}

const std::vector<int>& Cell::getNeighbours() const
{
	return neighbours;
}
