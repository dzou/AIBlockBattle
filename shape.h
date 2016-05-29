#ifndef __SHAPE_H
#define __SHAPE_H

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <utility>

enum Cell { EMPTY = 0, SHAPE = 1, BLOCK = 2, SOLID = 3 };
enum Move {UP, DOWN, LEFT, RIGHT, TURNLEFT, TURNRIGHT, DROP, SKIP};
enum ShapeType {I, J, L, O, S, T, Z, NONE};

extern std::vector<std::vector<int>> I_SHAPE;
extern std::vector<std::vector<int>> O_SHAPE;
extern std::vector<std::vector<int>> T_SHAPE;
extern std::vector<std::vector<int>> L_SHAPE;
extern std::vector<std::vector<int>> S_SHAPE;
extern std::vector<std::vector<int>> Z_SHAPE;
extern std::vector<std::vector<int>> J_SHAPE;
extern std::vector<int> GEN_TOPS;
extern std::vector<int> I_TOPS;

class Shape {
public:
	int row_;
	int col_;
	int rotations_;
	ShapeType type_;
	// std::vector<std::pair<int, int>> blocks_;	

	Shape(ShapeType shapeType);
	Shape(ShapeType shapeType, int row, int col);
	Shape(ShapeType shapeType, int row, int col, int rotations);
	std::string toString() const;
	void setCorner(int row, int col);

	void moveDir(Move move);

	Shape createNewShape(Move dir);	
	bool operator==(const Shape &rhs) const;
	bool operator!=(const Shape &rhs) const;
	
	inline void rotateClockwise() { rotations_ = (rotations_ + 1) % 4; }
	inline void rotateCounterClockwise() { rotations_ = (rotations_ + 3) % 4; }
	inline void moveUp() { row_ -= 1; }
	inline void moveRight() { col_ += 1; }
	inline void moveLeft() { col_ -= 1; }
	inline void moveDown() { row_ += 1; } 
	inline void setParams(int row, int col, int rotations) { row_ = row; col_ = col; rotations_ = rotations % 4; }

	const std::vector<int>& getBlocks() const {
		switch(type_) {
			case I: return I_SHAPE[rotations_];
			case O: return O_SHAPE[0];
			case T: return T_SHAPE[rotations_];
			case L: return L_SHAPE[rotations_];
			case S: return S_SHAPE[rotations_];
			case Z: return Z_SHAPE[rotations_];
			case J: return J_SHAPE[rotations_];
			case NONE: return O_SHAPE[0];
		}
		assert(false);
	}

	const int getTopRow(int rotations) const {
		assert(rotations >= 0 && rotations < 4);
		switch(type_) {
			case I: return I_TOPS[rotations_];			
			case O: return GEN_TOPS[0];
			default: return GEN_TOPS[rotations_];
		}
		assert(false);
	}

private:
	

	void setBlocks();
};

ShapeType getShapeType(std::string shapeString);
int getSymmetries(ShapeType type);

Move getOppositeMove(Move m);

namespace std {
	template<> struct hash<Shape> {
	    size_t operator()(const Shape &shape) const {
			return shape.row_ ^ (shape.col_ << 5) ^ (shape.rotations_ << 10);
	    }
	};
}

#endif // __SHAPE_H