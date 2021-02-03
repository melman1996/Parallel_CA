#pragma once
enum Mpi {
	IS_WORKING,
	CONFIG,
	LEFT_BORDER_SIZE,
	LEFT_BORDER,
	RIGHT_BORDER_SIZE,
	RIGHT_BORDER,
	SINGLE_CELL,
	BOARD,
	BOARD_SIZE
};

struct SingleCell {
	int y;
	int z;
	int state;

	SingleCell() = default;
	SingleCell(int y, int z, int state) : y(y), z(z), state(state) {}
};