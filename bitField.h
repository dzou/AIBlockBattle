#ifndef __BIT_FIELD_H
#define __BIT_FIELD_H

#include <string>
#include <vector>
#include "shape.h"

#define EMPTY_ROW 0xF00000FF
#define FULL_ROW 0xFAAAAAFF
#define SOLID_ROW 0xFFFFFFFF
#define CELL_SELECTOR 3

class BitField {

	public:
		int width_;
		int height_;
		std::vector<int> grid_;

		BitField(BitField &&other);
		BitField& operator=(BitField&& other);
		BitField& operator=(const BitField& other);
		BitField(const BitField &other);
		BitField(int width, int height);
		BitField(int width, int height, std::string fieldString);

		bool operator==(const BitField &field) const;
		bool operator!=(const BitField &field) const;

		std::string toString() const;

		
		bool isTSpin(const Shape &s) const;

		void printHex() const;
		void placePiece(const Shape &s);
		int clearLines();

		void dropPiece(Shape &s) const;
		int getHoleCount() const;
		bool inField(const Shape &s) const;		
		std::vector<int> getContourProfile() const;
		std::vector<std::pair<int, int>> getHoles() const;

		void setCell(int row, int col, Cell cell);
		bool isPerfectClear() const;
		inline int getCell(int row, int col) const { return (grid_[row + 2] >> 2 * (width_ - col + 3)) & CELL_SELECTOR; }
		inline const int& getRow(int row) const {	return grid_[row + 2]; }

		inline bool isValid(const Shape &s, const Move &dir) const {
			int rOffset = 0;
			int cOffset = 0;
			if (dir == Move::DOWN) { rOffset = 1; } 
			else if (dir == Move::UP) { rOffset = -1; } 
			else if (dir == Move::LEFT) { cOffset = -1; } 
			else if (dir == Move::RIGHT) { cOffset = 1; }

			int adjRow = s.row_ + rOffset + 2;
			int adjCol = s.col_ + cOffset;
			const std::vector<int> &blocks = s.getBlocks();
			int shapeWidth = blocks.size();

			for (int i = 0; i < shapeWidth; ++i) {
			if ((grid_[adjRow + i] & (blocks[i] << 2 * (14 - shapeWidth - adjCol))) != 0) { return false; } }
			return true;
		}

		inline bool isValid(const Shape &s) const {
			return isValid(s, Move::SKIP);
		}
};

namespace std {
	template<> struct hash<BitField> {
	    size_t operator()(const BitField &bf) const {
	    	size_t result = 0;
	    	for (int i = 0; i < bf.height_; i++) {
	    		result ^= bf.grid_[i];
	    		result <<= 1;
	    	}
	    	return result;
	    }
	};
}


#endif // __BIT_FIELD_H