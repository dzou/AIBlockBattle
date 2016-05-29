#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include <cfloat>
#include <queue>
#include <unordered_set>
#include <unordered_map>

#include "heuristic.h"
#include "bitField.h"
#include "shape.h"
#include "botStarter.h"
#include "botState.h"

using namespace std;

extern float COEFF_ARR[];

// static const float &SAMPLE_PORTION = 175.0;
static const float &DOWNSTACK_THRESH = 17.0;


static const float &DEEP_HEIGHT = COEFF_ARR[8];
static const float &DEEP_SIZE = 250.0;
static const float &SHALLOW_SIZE = 200.0;
static const float &SKIP_THRESH = 2.0;

#define VISUAL

/*
static const float &GREEDY_SAMPLE_PORTION = 0;
static const float &GREEDY_THRESH = 0;
static const float &SAFE_THRESH = 0;
static const float &SAFE_SAMPLE = 0;
*/

// move should be printed here.
void BotStarter::makeMove(BotState state, bool fastMode) {

	vector<Shape> shapeList = {state.currentShape_, state.nextShape_};

	Player oppInfo = state.playerMap_[state.oppName_];
	GameState oppStart = GameState(state.oppField_, oppInfo.combo);
	GameState bestOppMove = calculateBest3PlyMove(oppStart, shapeList, scoreState);
	float points = ceil(linePoints(bestOppMove));
	int linesGained = ((oppInfo.rowPoints % 3) + points) / 3;
	if (state.round_ > 1 && state.round_ % 15 == 1) {
		linesGained += 1;
	}
	

	Player myInfo = state.playerMap_[state.botName_];
	GameState start = GameState(state.myField_, myInfo.combo);
	start.linesThreatened_ = linesGained;

	int currentMaxHeight = getMaxHeight(state.myField_) + linesGained;

	int searchDepth = 2; // int x = currentMaxHeight;
	int sampleSize = 150; // (int)(37 + 29 * x - 3.57 * x * x + 0.122 * x * x * x);


	// int aggHeight = aggregateHeight(state.myField_.getContourProfile());
	// cerr << "Agg Height: " << aggHeight << "\n";
	if (currentMaxHeight < DEEP_HEIGHT) {
		sampleSize = DEEP_SIZE;
		searchDepth = 3;
	} else {
		sampleSize = SHALLOW_SIZE;
		searchDepth = 2;
	}

	if (state.playerMap_.at(state.botName_).skips > 0) {
		GameState best12Piece = calculateBest3PlyMove(start, shapeList, scoreState);
		GameState best1Piece = calculateBest3PlyMove(start, {state.currentShape_}, scoreState);
		GameState best2Piece = calculateBest3PlyMove(start, {state.nextShape_}, scoreState);

		float score12Skip = scoreState(best12Piece);
		float score1Skip = scoreState(best1Piece);
		float score2Skip = scoreState(best2Piece);

		if (score2Skip > max(score12Skip, score1Skip) + SKIP_THRESH) {
			cout << "skip" << endl;

			cerr << "skipped \n";
			cerr << state.currentShape_.toString() << "\n";
			cerr << state.myField_.toString();
			return;
		}
	}

	if (currentMaxHeight < DOWNSTACK_THRESH) {

		vector<pair<GameState, float>> startingPositions = calculateBestMoves(start, shapeList, 50, scoreState);
		vector<GameState> gsList = moveSearch(startingPositions, searchDepth, 200, 4, scoreState);

		map<vector<Move>, int> moveFreqMap;
		int startIdx = max(0, (int)gsList.size() - sampleSize);

		for (int i = startIdx; i < gsList.size(); i++) {
			GameState &x = gsList[i];
			auto search = moveFreqMap.find(x.moves_);

			if (search == moveFreqMap.end()) {
				moveFreqMap[x.moves_] = 1;
			} else {
				moveFreqMap[x.moves_] = (*search).second + 1;
			}
		}

		if (!moveFreqMap.empty()) {
			vector<Move> bestList;
			int bestCount = -1;

			for (auto &entry : moveFreqMap) {
				if (entry.second > bestCount) {
					bestList = entry.first;
					bestCount = entry.second;
				}
			}

			assert(!bestList.empty());
			cout << moveSetToString(bestList) << endl;

#ifdef VISUAL
			//// BEGIN
			Shape placer(shapeList[0].type_);
			for (Move m : bestList) {
				placer.moveDir(m);
			}
			state.myField_.dropPiece(placer);
			state.myField_.placePiece(placer);
			cerr << "Lines Threatened: " << linesGained << "\n";
			cerr << "My Max Height: " << currentMaxHeight << "\n";
			cerr << placer.toString() << "\n";
			cerr << state.myField_.toString();
			// ENDD
#endif

		} else {
			cout << "no_moves" << endl;
		}

	} else {
		GameState best = calculateBest3PlyMove(start, shapeList, scoreState);
		if (!best.moves_.empty()) {
			cout << moveSetToString(best.moves_) << endl;

#ifdef VISUAL
			/// BEGINNNN 
			Shape placer(shapeList[0].type_);
			for (Move m : best.moves_) {
				placer.moveDir(m);
			}
			state.myField_.dropPiece(placer);
			state.myField_.placePiece(placer);
			cerr << placer.toString() << "\n";
			cerr << state.myField_.toString();
			//// ENDDD
#endif

		} else if (state.playerMap_.at(state.botName_).skips > 0) {
			cout << "skip" << endl;
		} else {
			cout << "no_moves" << endl;
		}
	}

}

vector<Move> moveToColumn(const BitField &field, Shape shape, int column, int rotations) {
	if (!field.isValid(shape)) {
		return vector<Move>();
	}

	vector<Move> result;
	result.reserve(20);

	int sg = column - shape.col_;
	Move dir;
	Move turn;

	if (sg > 0) {
		dir = RIGHT;
	} else if (sg < 0) {
		dir = LEFT;
	}

	if (rotations == 3) {
		turn = TURNLEFT;
		rotations = 1;	
	} else {
		turn = TURNRIGHT;
	}

	sg = abs(sg);

	while (sg > 0 && field.isValid(shape, dir)) {
		shape.moveDir(dir);
		result.push_back(dir);
		sg--;
	}

	bool success = true;
	for (int i = 0 ; i < rotations; i++) {
		shape.moveDir(turn);
		result.push_back(turn);
		success &= field.isValid(shape);
	}

	if (success && sg > 0) {
		while (sg > 0 && field.isValid(shape, dir)) {
			shape.moveDir(dir);
			result.push_back(dir);
			sg--;
		}
	}
	result.push_back(Move::DROP);

	if (success && sg == 0) {
		return result;
	} else {
		return vector<Move>(); 
	}
}

void pushHardDropMoves(vector<GameState> &stack, GameState &currState, const Shape &currShape) {
	BitField &currentField = currState.getField();
	if (!currentField.isValid(currShape)) {
		return;
	}

	Shape placer = Shape(currShape.type_);
	int syms = getSymmetries(currShape.type_);

	for (int i = -1; i < currentField.width_; i++) {
		int startRot = 4;
		int endRot = 7;

		if (syms == 1) {
			endRot = 4;
		} else if (syms == 2) {
			if (i < currShape.col_) {
				startRot = 3;
				endRot = 4;
			} else if (i == currShape.col_) {
				startRot = 3;
				endRot = 5;
			} else {
				startRot = 4;
				endRot = 5;
			}
		}

		for (int j = startRot; j <= endRot; j++) {
			placer.setParams(-1, i, j % 4);
			vector<Move> moveSet = moveToColumn(currentField, currShape, placer.col_, placer.rotations_);

			if (!moveSet.empty()) {
				/*
				cout << placer.toString() << "\n";
				cout << currentField.toString() << "\n";
				cout << moveSetToString(moveSet) << "\n";
				*/
				GameState g(currState);
				BitField &gField = g.getField();
				gField.dropPiece(placer);

				if (gField.inField(placer)) {
					gField.placePiece(placer);
					int clearLineVal = gField.clearLines();

					g.depth_ += 1; // depth
					if (currState.depth_ == 0) {
						g.moves_ = move(moveSet); // moves
						g.oneDeepScore_ = scoreState(g);
					}


					// Update the game state
					bool tspinVal = false;
					g.tSpinArr_.push_back(tspinVal);		// tspins
					g.lineClearArr_.push_back(clearLineVal); // line clears
					g.perfClearArr_.push_back(gField.isPerfectClear());

					stack.push_back(std::move(g));
				}

			}
		}
	}
}

// Search from Bottom Up
std::vector<Move> getMoveList(const BitField &field, Shape start, Shape end) {
	if (!field.isValid(end) /* || !field.isValid(start) */) {
		return vector<Move>();
	}

	unordered_map<Shape, Move> prevMap;

	CompareShape comp(end);
	vector<Shape> container;
	container.reserve(128);
	priority_queue<Shape, std::vector<Shape>, CompareShape> q(move(comp), move(container)); 

	// Directions to move from start to end....
	vector<Move> directions = {UP, LEFT, RIGHT, TURNRIGHT, TURNLEFT};

	q.push(start);
	while (!q.empty()) {
		Shape curr = q.top();
		q.pop();

		if (curr == end) {
			vector<Move> moveList;
			auto search = prevMap.find(curr);
			while (search != prevMap.end()) {
				Move m = search->second;
				moveList.push_back(getOppositeMove(m));
				curr.moveDir(getOppositeMove(m));
				search = prevMap.find(curr);
			}
			moveList.push_back(Move::DROP);
			return moveList;
		}

		for (Move m : directions) {
			Shape s = curr.createNewShape(m);
			if (field.isValid(s) && prevMap.find(s) == prevMap.end() && s != start) {
				prevMap.insert({ s, m });
				q.push(s);
			}
		}
	}

	return vector<Move>();
}

void pushSoftDropMoves(std::vector<GameState> &stack, GameState &currState, const Shape &currentShape) {
	BitField &currentField = currState.getField();

	Shape placer(currentShape.type_);
	for (int k = 0; k < 4; k++) {
		int topRow = placer.getTopRow(k);
		for (int i = 1; i < currentField.height_ - 1; i++) {

			if (currentField.getRow(topRow + i) == EMPTY_ROW
					|| currentField.getRow(topRow + i) == SOLID_ROW) {
				continue;
			}

			for (int j = -1; j < currentField.width_ - 2; j++) {
				placer.setParams(i, j, k);
				if (!currentField.isValid(placer, Move::UP) 
						&& currentField.inField(placer)) {
					currentField.dropPiece(placer);
					vector<Move> moveList = getMoveList(currentField, placer, currentShape);
					if (!moveList.empty()) {
						GameState g(currState);
						BitField &gField = g.getField();

						g.depth_ += 1; // depth
						if (currState.depth_ == 0) {
							g.moves_ = move(moveList); // moves
							g.oneDeepScore_ = scoreState(g);
						}

						bool tspinVal = gField.isTSpin(placer);

						gField.placePiece(placer); // field
						int lineClears = gField.clearLines();

						// Update values
						g.lineClearArr_.push_back(lineClears);
						g.tSpinArr_.push_back(tspinVal);
						g.perfClearArr_.push_back(gField.isPerfectClear());

						stack.push_back(std::move(g));
					}	
				}
			}
		}
	}
}

static vector<Shape> allShapes = { Shape(Z), Shape(S), Shape(O), Shape(T), Shape(J), Shape(I), Shape(L) };
vector<pair<GameState, float>> calculateBestMoves(GameState &gameState, const vector<Shape> &shapeList, int N, 
			float (*scorer)(GameState &gs)) {

	vector<GameState> stack;
	stack.reserve(32);
	stack.push_back(gameState);

	priority_queue<pair<GameState, float>, 
		std::vector<pair<GameState, float>>, 
		CompareGameStatePair> pq;

	int startDepth = gameState.depth_;
	int maxDepth = shapeList.size() + gameState.depth_;

	while (!stack.empty()) {
		GameState curr = move(stack.back());
		stack.pop_back();

		if (curr.depth_ < maxDepth) {
			pushHardDropMoves(stack, curr, shapeList[curr.depth_ - startDepth]);
			pushSoftDropMoves(stack, curr, shapeList[curr.depth_ - startDepth]);
		} else {
			float score = scorer(curr);
			pq.push(make_pair(curr, score));
			if (pq.size() > N) {
				pq.pop();
			}
		}
	}

	vector<pair<GameState, float>> result;
	if (gameState.depth_ > 0) {
		while(!pq.empty()) {
			result.push_back(pq.top());
			pq.pop();
		}
	} else {
		map<BitField, pair<GameState, float>, CompareBitField> seenMap;
		while (!pq.empty()) {
			pair<GameState, float> entry = pq.top();
			map<BitField, pair<GameState, float>>::iterator it = seenMap.find(entry.first.getField());

			if (it != seenMap.end()) {
				float myScore = entry.first.oneDeepScore_;		
				float seenScore = it->second.first.oneDeepScore_;

				if (entry.second > it->second.second ||
				 		(entry.second == it->second.second && myScore > seenScore)) {
					it->second = move(entry);
				}
			} else {
				seenMap.insert(make_pair(entry.first.getField(), entry));
			}

			pq.pop();
		}

		for (const pair<BitField, pair<GameState, float>> &x : seenMap) {
			result.push_back(x.second);
		}
		sort(result.rbegin(), result.rend(), CompareGameStatePair());
	}


	/*
	for (auto x : result) {
		cout << x.first.getField().toString();
		cout << moveSetToString(x.first.moves_) << "\n";
		cout << x.second << "\n";
	} */

	return result;
}

vector<GameState> moveSearch(vector<pair<GameState, float>> &startingStates, int depth, int bigN, int littleN, 
	float(*scorer)(GameState &gs)) {
	vector<GameState> states;
	for (auto &x : startingStates) {
		states.push_back(x.first);
	}
	return moveSearch(states, depth, bigN, littleN, scorer);
}

vector<GameState> moveSearch(vector<GameState> &startingStates, int depth, int bigN, int littleN, float(*scorer)(GameState &gs)) {

	priority_queue<pair<GameState, float>, 
		std::vector<pair<GameState, float>>, 
		CompareGameStatePair> pq;


	vector<GameState> currentGeneration = startingStates;
	for (int i = 0; i < depth; i++) {
		for (GameState &start : currentGeneration) {
			for (const Shape &s : allShapes) {
				vector<pair<GameState, float>> localSearch = calculateBestMoves(start, {s}, littleN, scorer);
				for (auto &x : localSearch) {
					pq.push(x);
					if (pq.size() > bigN) {
						pq.pop();
					}
				}
			}	
		}

		if (!pq.empty()) {
			currentGeneration.clear();
			while (!pq.empty()) {
				currentGeneration.push_back(pq.top().first);
				pq.pop();
			}
		}	else {
			break;
		}
	}

	return currentGeneration;
}

GameState calculateBest3PlyMove(GameState &start, const vector<Shape> &shapeList,
		float(*scorer)(GameState &gs)) {
	float bestScore = (-1) * FLT_MAX;
	GameState bestState;

	vector<GameState> stack;	
	stack.reserve(512);
	stack.push_back(start);

	vector<GameState> futures;
	futures.reserve(64);

	int count = 0;
	while (!stack.empty()) {
		count++;
		GameState curr = move(stack.back());
		stack.pop_back();

		if (curr.depth_ < shapeList.size()) {
			pushHardDropMoves(stack, curr, shapeList.at(curr.depth_));
			pushSoftDropMoves(stack, curr, shapeList.at(curr.depth_));
		} else {
			float scoreSum = 0;

			float twoPlyScore = scorer(curr);
			if (/* twoPlyScore < 0 && */ bestScore - twoPlyScore > 5.0) {
				continue;
			}

			for (Shape &s : allShapes) {
				float bestCurrScore = -200;
				pushHardDropMoves(futures, curr, s);
				pushSoftDropMoves(futures, curr, s);

				for (GameState &state : futures) {
					float tmp = scorer(state);
					if (tmp > bestCurrScore) {
						bestCurrScore = tmp;
					}
				}

				scoreSum += bestCurrScore / 7.0;
				futures.clear();
			}

			if (scoreSum > bestScore) {
				bestState = move(curr);
				bestScore = scoreSum;
			}
		}
	}	
	return bestState;
}

float dummyScore(GameState &gs) {
	float sum = 0;
	assert(gs.tSpinArr_.size() == gs.lineClearArr_.size());
	
	for (int i = 0; i < gs.lineClearArr_.size(); i++) {
		float factor = 1.0 * gs.lineClearArr_[i] * gs.lineClearArr_[i];
		if (gs.tSpinArr_[i]) {
			factor *= 4;
		}
		sum += factor;
	}
	return sum;
}

string moveSetToString(const vector<Move> &moves) {
	string result = "";
	for (Move const& move : moves) {
		if (move == Move::DOWN) {
			result += "down";
		}	else if (move == Move::LEFT) {
			result += "left";
		} else if (move == Move::RIGHT) {
			result += "right";
		} else if (move == Move::TURNRIGHT) {
			result += "turnright";
		} else if (move == Move::TURNLEFT) {
			result += "turnleft";
		} else if (move == Move::DROP) {
			result += "drop";
		} else if (move == Move::SKIP) {
			result += "skip";
		} else {
			cout << result << "\n";
			assert(false); // should never reach here;
		}
		result += ",";
	}	
	return result;
}
