#include <iostream>
#include <vector>
#include <utility>

#include "shape.h"

using namespace std;

vector<int> I_TOPS = {1, 0, 2, 0};
vector<int> GEN_TOPS = {0, 0, 1, 0};

vector<vector<int>> I_SHAPE = {
	{0x0,  0xAA, 0x0,  0x0 },
	{0x8,	 0x8,  0x8,  0x8 },
	{0x0,  0x0,  0xAA, 0x0 },
	{0x20, 0x20, 0x20, 0x20}
};

vector<vector<int>> O_SHAPE = {
	{0xA, 0xA}
};

vector<vector<int>> J_SHAPE = {
	{0x20, 0x2A, 0x0},
	{0xA,  0x8,  0x8},
	{0x0,  0x2A, 0x2},
	{0x8,  0x8,  0x28}
};

vector<vector<int>> L_SHAPE = {
	{0x2, 0x2A, 0x0},
	{0x8, 0x8, 0xA},
	{0x0, 0x2A, 0x20},
	{0x28, 0x8, 0x8}
};

vector<vector<int>> S_SHAPE = {
	{0xA, 0x28, 0x0},
	{0x8, 0xA,  0x2},
	{0x0, 0xA, 0x28},
	{0x20, 0x28, 0x8}
};

vector<vector<int>> Z_SHAPE = {
	{0x28, 0xA, 0x0},
	{0x2, 0xA, 0x8},
	{0x0, 0x28, 0xA},
	{0x8, 0x28, 0x20},
};

vector<vector<int>> T_SHAPE = {
	{0x8, 0x2A, 0x0},
	{0x8, 0xA, 0x8},
	{0x0, 0x2A, 0x8},
	{0x8, 0x28, 0x8}
};

Shape::Shape(ShapeType type) {
	type_ = type;

	if (type == ShapeType::O) {
		row_ = -1;
		col_ = 4;
	} else {
		row_ = -1;
		col_ = 3;
	}
	rotations_ = 0;
}

Shape::Shape(ShapeType type, int row, int col) {
	type_ = type;
	row_ = row;
	col_ = col;
	rotations_ = 0;
}

Shape::Shape(ShapeType type, int row, int col, int rotations) {
	type_ = type;
	row_ = row;
	col_ = col;
	rotations_ = rotations % 4;
}

bool Shape::operator==(const Shape &rhs) const {
	if (rhs.row_ == row_ && rhs.col_ == col_ && rhs.rotations_ == rotations_) {
		return true;
	} else {
		return false;
	}
}

bool Shape::operator!=(const Shape &rhs) const {
	return !(*this == rhs);
}

Shape Shape::createNewShape(Move dir) {
	if (dir == Move::UP) {
		return Shape(type_, row_ - 1, col_, rotations_);
	} else if (dir == Move::DOWN) {
		return Shape(type_, row_ + 1, col_, rotations_);
	} else if (dir == Move::LEFT) {
		return Shape(type_, row_, col_ - 1, rotations_);
	} else if (dir == Move::RIGHT) {
		return Shape(type_, row_, col_ + 1, rotations_);
	} else if (dir == Move::TURNRIGHT) {
		return Shape(type_, row_, col_, rotations_ + 1);
	} else if (dir == Move::TURNLEFT) {
		return Shape(type_, row_, col_, rotations_ + 3);
	} else {
		return Shape(type_, row_, col_, rotations_);
	}
}

void Shape::moveDir(Move m) {
	if (m == Move::DOWN) {
		moveDown();
	} else if (m == Move::LEFT) {
		moveLeft();
	} else if (m == Move::RIGHT) {
		moveRight();
	} else if (m == Move::UP) {
		moveUp();
	} else if (m == Move::TURNRIGHT) {
		rotateClockwise();
	} else if (m == Move::TURNLEFT) {
		rotateCounterClockwise();
	}
}

void Shape::setCorner(int row, int col) {
	row_ = row;
	col_ = col;
}

string Shape::toString() const {
	string result;
	vector<vector<int>> grid(4, vector<int>(4, 0));
	const vector<int> &blocks = getBlocks();
	for (int i = 0; i < blocks.size(); i++) {
		for (int j = 0; j < blocks.size(); j++) {
			int cell = (blocks[i] >> 2 * (blocks.size() - j - 1)) & 3;
			if (cell == 2) {
				grid[i][j] = 1;
			}
		}
	}

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result += to_string(grid.at(i).at(j));
		}
		result += "\n";
	}
	result += "Corner: (" + to_string(row_) + ", " + to_string(col_) + ") r: " + to_string(rotations_);
	return result;
}

ShapeType getShapeType(std::string shapeString) {
	if (shapeString == "I") {
		return ShapeType::I;
	}
	else if (shapeString == "J") {
		return ShapeType::J;
	}
	else if (shapeString == "L") {
		return ShapeType::L;
	}
	else if (shapeString == "O") {
		return ShapeType::O;
	}
	else if (shapeString == "S") {
		return ShapeType::S;
	}
	else if (shapeString == "T") {
		return ShapeType::T;
	}
	else if (shapeString == "Z") {
		return ShapeType::Z;
	}

	assert(false); // Should never reach here; invalid shapeString
	return ShapeType::O;
}

int getSymmetries(ShapeType type) {
	if (type == ShapeType::O) {
		return 1;
	}
	else if (type == ShapeType::T || type == ShapeType::L || type == ShapeType::J) {
		return 4;
	}
	else {
		return 2;
	}
}

Move getOppositeMove(Move m) {
	switch (m) {
		case UP: return DOWN;
		case DOWN: return UP;
		case LEFT: return RIGHT;
		case RIGHT: return LEFT;
		case TURNRIGHT: return TURNLEFT;
		case TURNLEFT: return TURNRIGHT;
		case SKIP: return SKIP;
		case DROP: return DROP;
	}
	assert(false);
	return SKIP;
}

/*
int main() {
	vector<ShapeType> types = {I, O, J, L, S, Z, T};
	for (ShapeType t : types) {
		for (int i = 0; i < 4; i++) {
			Shape s(t, -1, -1, i);
			cout << s.toString() << "\n";
		}
	}
	return 1;
} 
*/