#ifndef __BOT_STARTER_H
#define __BOT_STARTER_H

#include <vector>
#include "botState.h"
#include "bitField.h"

class BotStarter {
	public:
		void makeMove(BotState state, bool fast = false);
};

int getMaxHeight(BitField &f);

struct GameState {
	BitField field_;
	int combo_;
	int depth_;
	float oneDeepScore_;
	int rootMaxHeight_;
	int linesThreatened_;

	std::vector<Move> moves_;
	std::vector<int> lineClearArr_;
	std::vector<bool> tSpinArr_;
	std::vector<bool> perfClearArr_;

	GameState() : field_(0, 0), combo_(0), depth_(0), oneDeepScore_(0), rootMaxHeight_(0), linesThreatened_(0) {}
	GameState(const BitField &f) : field_(f), combo_(0), depth_(0), oneDeepScore_(0), linesThreatened_(0) {
		rootMaxHeight_ = getMaxHeight(field_);
	}
	GameState(const BitField &f, int combo) : field_(f), combo_(combo), depth_(0), oneDeepScore_(0), linesThreatened_(0) {
		rootMaxHeight_ = getMaxHeight(field_);
	}

	BitField& getField() { return field_; }
};

struct CompareShape {
	const Shape &dest;
	CompareShape(const Shape &d) : dest(d) {}

	int getDistance(const Shape &input) const {
		return abs(input.row_ - dest.row_) + abs(input.col_ - dest.col_)
			+ abs(input.rotations_ - dest.rotations_);
	}

	bool operator()(const Shape &lhs, const Shape &rhs)	const {
		return getDistance(lhs) > getDistance(rhs);
	}
};

struct CompareBitField {
	bool operator()(const BitField &lhs, const BitField &rhs) {
		return lhs.grid_ < rhs.grid_;
	}
};

struct CompareGameStatePair {
	bool operator()(const std::pair<GameState, float> &lhs, const std::pair<GameState, float> &rhs) {
		return lhs.second > rhs.second;	
	}
};

std::vector<Move> moveToColumn(const BitField &field, Shape shape, int column, int clockwiseRotations);
std::vector<std::pair<GameState, float>> calculateBestMoves(GameState &gameState, const std::vector<Shape> &shapeList, int N, float (*scorer)(GameState &gs));
std::vector<GameState> moveSearch(std::vector<std::pair<GameState, float>> &startingStates, int depth, int bigN, int littleN, float(*scorer)(GameState &gs));
std::vector<GameState> moveSearch(std::vector<GameState> &startingStates, int depth, int bigN, int littleN, float(*scorer)(GameState &gs));

std::string moveSetToString(const std::vector<Move> &moves);
void pushHardDropMoves(std::vector<GameState> &stack, GameState &currState, const Shape &currShape);
void pushSoftDropMoves(std::vector<GameState> &stack, GameState &currState, const Shape &shape);
std::vector<Move> getMoveList(const BitField &field, Shape start, Shape end);

GameState calculateBest3PlyMove(GameState &start, const std::vector<Shape> &shapeList, float(*scorer)(GameState &gs));

float dummyScore(GameState &gs);

#endif