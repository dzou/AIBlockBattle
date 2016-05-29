#include <iostream>
#include <string>
#include <algorithm>

#include "bitField.h"
#include "util.h"
#include "shape.h"

using namespace std;


BitField::BitField(BitField &&other) 
	: width_(other.width_)
	, height_(other.height_)
	, grid_(std::move(other.grid_))
	{}

BitField& BitField::operator=(BitField &&other) {
	if (this != &other) {
		this->width_ = other.width_;
		this->height_ = other.height_;
		this->grid_ = std::move(other.grid_);
	}
	return *this;
}

BitField& BitField::operator=(const BitField &other) {
	if (this != &other) {
		this->width_ = other.width_;
		this->height_ = other.height_;
		this->grid_ = other.grid_;
	}
	return *this;
}

BitField::BitField(const BitField &otherField) {
	width_ = otherField.width_;
	height_ = otherField.height_;
	grid_ = otherField.grid_;
}

BitField::BitField(int width, int height) : width_(width), height_(height), 
	grid_(height + 2, EMPTY_ROW) { 
		// cout << width << "\n";
		grid_[0] = SOLID_ROW;
		grid_.push_back(SOLID_ROW);
		grid_.push_back(SOLID_ROW);
		grid_.push_back(SOLID_ROW);
	}

BitField::BitField(int width, int height, string fieldString) 
	: width_(width), 
		height_(height), 
		grid_(height + 2, 0)
{
		assert(width == 10);
		vector<string> gridLines = Split(fieldString, ';');
		assert(gridLines.size() == height); // input dimension checks

		grid_[0] = SOLID_ROW;
		grid_[1] = EMPTY_ROW;
		for (int i = 0; i < gridLines.size(); i++) {
			int line = 0xF << 2;
			vector<string> tokens = Split(gridLines[i], ',');
			assert(tokens.size() == width);
			for (int j = 0; j < tokens.size(); j++) {
				int change = stoi(tokens[j]);
				if (change == 1) {
					change = 0;
				}
				line |= change;
				line <<= 2;
			}
			line <<= 6;
			line |= 0xFF;

			grid_[i + 2] = line;
		}

		grid_.push_back(SOLID_ROW);
		grid_.push_back(SOLID_ROW);
		grid_.push_back(SOLID_ROW);
}

bool BitField::operator==(const BitField &rhs) const {
	if (grid_ == rhs.grid_) {
		return true;
	} else {
		return false;
	}
}

bool BitField::operator!=(const BitField &rhs) const {
	return !(*this == rhs);
}

string BitField::toString() const {
	string result = "";
	for (int i = 0; i < height_; i++) {
		for (int j = 0; j < width_; j++) {
			int cell = getCell(i, j);
			if (cell == BLOCK) {
				result += "+";
			} else {
				result += to_string(cell);
			}
		}
		result += "\n";
	}
	return result;
}

void BitField::printHex() const {
	for (const int &row : grid_) {
		cout << hex << row << "\n";
	}
	cout << "\n";
}



bool BitField::inField(const Shape &s) const {
	if (!isValid(s) || (s.row_ < 0 && s.getBlocks()[0] != 0)) {
		return false;
	} else {
		return true;
	}
}

int BitField::clearLines() {
	/*
	cout << widthMask << "\n";
	cout << FULL_LINE << "\n";
	cout << fullLineAdj << "\n";
	cout << width_ << "\n"; */
	int lineCount = 0;
	vector<int>::iterator it = grid_.begin();
	while (it != grid_.end()) {
		if (*it == FULL_ROW) {
			it = grid_.erase(it);
			++lineCount;
		} else {
			++it;
		}
	}	

	for (int i = 0; i < lineCount; i++) {
		grid_.insert(grid_.begin() + 1, EMPTY_ROW);
	}
	return lineCount;
}

bool BitField::isTSpin(const Shape &s) const {
	// assert(isValid(s));

	if (s.type_ == T && !isValid(s, UP)) {
		int offset = 2 * (14 - 3 - s.col_);
		int topCorners = (getRow(s.row_) & (0x33 << offset)) >> offset;
		int bottomCorners = (getRow(s.row_ + 2) & (0x33 << offset)) >> offset;
		
		int tSpinHead;
		if (s.rotations_ == 0) {
			tSpinHead = getCell(s.row_ + 2, s.col_ + 1);
		} else if (s.rotations_ == 1) {
			tSpinHead = getCell(s.row_ + 1, s.col_);
		} else if (s.rotations_ == 2) {
			tSpinHead = getCell(s.row_, s.col_ + 1);
		} else {
			tSpinHead = getCell(s.row_ + 1, s.col_ + 2);
		}

		/*
		bool isTspin = ((topCorners == 0x20) || (topCorners == 0x2)) && (bottomCorners == 0x22);
		int cornerBlocks = 0;
		cornerBlocks += getCell(s.row_, s.col_) == BLOCK ? 1 : 0;
		cornerBlocks += getCell(s.row_ + 2, s.col_) == BLOCK ? 1 : 0;
		cornerBlocks += getCell(s.row_, s.col_ + 2) == BLOCK ? 1 : 0;
		cornerBlocks += getCell(s.row_ + 2, s.col_ + 2) == BLOCK ? 1 : 0;

		if (cornerBlocks >= 3) {
			if (!isTspin) {
				cout << toString() << "\n";
				cout << s.toString() << "\n";
				cout << hex << topCorners << "\n";
				cout << hex << bottomCorners << "\n";
				// assert(false);
			}
		}
		*/

		return (bottomCorners == 0x22) && ((topCorners == 0x20) || (topCorners == 0x2))
					&& (tSpinHead == 0);
	} 
	
	return false;
}

vector<int> BitField::getContourProfile() const {
	vector<int> result(width_, 0);
	for (int i = 0; i < height_; i++) {
		for (int j = 0; j < width_; j++) {
			if (getCell(i, j) == BLOCK) {
				result[j] = max(height_ - i, result[j]);
			}
		}
	}
	return result;
}

vector<pair<int, int>> BitField::getHoles() const {
	vector<pair<int, int>> result;
	vector<int> contourProfile = getContourProfile();
	
	for (int i = 0; i < height_; i++) {
		for (int j = 0; j < width_; j++) {
			if (height_ - i < contourProfile[j] && getCell(i, j) == EMPTY) {
				result.push_back(make_pair(i, j));
			}
		}
	}
	
	return result;
}

int BitField::getHoleCount() const {
	vector<int> contourProfile = getContourProfile();
	int count = 0;
	for (int i = 0; i < height_; i++) {
		for (int j = 0; j < width_; j++) {
			if (height_ - i < contourProfile[j] && getCell(i, j) == EMPTY) {
				++count;
			}
		}
	}
	return count;
}

void BitField::placePiece(const Shape &shape) {
	assert(inField(shape));

	const vector<int> &blocks = shape.getBlocks();
	int shapeWidth = blocks.size();

	for (int i = 0; i < shapeWidth; ++i) {
		grid_[shape.row_ + i + 2] |= (blocks[i] << 2 * (14 - shapeWidth - shape.col_));
	}
}

void BitField::dropPiece(Shape &shape) const {
	// assert(isValid(shape));
	while (isValid(shape)) {
		shape.moveDown();
	}
	shape.moveUp();
}

bool BitField::isPerfectClear() const {
	for (const int &line : grid_) {
		if (line != SOLID_ROW && line != EMPTY_ROW) {
			return false;
		}
	}
	return true;
}

/*
int main() {
	string input = 
		"0,0,0,0,1,1,0,0,0,0;"
		"0,0,0,0,0,0,0,0,0,0;"
		"0,0,0,0,0,0,0,0,0,0;"
		"0,0,0,0,0,0,0,0,0,0;"
		"0,0,0,0,0,0,0,0,0,0;"
		"0,0,0,0,0,0,0,0,0,0;"
		"2,2,2,2,2,2,2,2,2,2;"
		"0,0,0,0,2,0,0,0,0,2;"
		"0,0,0,0,0,2,2,2,2,2;"
		"2,0,2,2,2,2,0,2,0,2";

	string input2 = 
		"0,0,0,0,0,0,0,0,0,0;"
		"0,0,0,0,0,0,0,0,0,0;"
		"0,0,0,0,0,0,0,0,0,0;"
		"0,0,0,0,0,0,0,0,0,0;"
		"0,0,0,0,0,0,0,0,0,0;"
		"0,0,0,0,0,0,0,0,0,0;"
		"0,0,0,0,0,0,0,0,0,0;"
		"2,2,2,2,0,0,2,2,2,2;"
		"2,2,2,2,0,0,2,2,2,2;"
		"0,2,2,2,2,2,2,2,2,2";

	BitField bf(10, 10, input);
	cout << bf.toString() << "\n";
	bf.printHex();

	Shape s(T);
	s.rotateClockwise();
	bf.dropPiece(s);
	while (bf.isValid(s, LEFT)) {
		s.moveLeft();
	}
	bf.placePiece(s);

	cout << bf.toString() << "\n";
	cout << s.toString() << "\n";

	return 1;
}
*/