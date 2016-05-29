#include <math.h>
#include <algorithm>

#include "heuristic.h"
#include "botStarter.h"
#include "shape.h"

using namespace std;

// COEFF_ARR[5] is used in botStarter.cpp for testing squared score vs. normal score
float COEFF_ARR[] = { -1.0, 0.90, -0.75, -1.66, -3.9, -1.0, -0.85, 5.0, 14.0, 1.8, 0.0, 0.0};

static const float &HOLE_DECAY = 0.10;
static const float &MAX_HEIGHT = COEFF_ARR[0];
static const float &POINTS = 1.0;
static const float &POINTS_DECAY = COEFF_ARR[1];

// static const float &TSPINS = 0.85;
static const float &TSPIN_SINGLE = 0.6;
static const float &TSPIN_DOUBLE = 1.0;
static const float &TSPIN_ZERO = 0.2;
static const float &TSPIN_SLOPE = COEFF_ARR[2];

static const float &SMOOTHNESS = COEFF_ARR[3]; // squared smooth is: -1.55;
static const float &HOLES = COEFF_ARR[4];
static const float &HEIGHT = 0.0;
static const float &TRIANGLE_AREA = COEFF_ARR[5];

static const float &OPP_COST = COEFF_ARR[6];
static const float &OPP_THRESH = COEFF_ARR[7];

void printValues() {
	cout << "MAX_HEIGHT:" << MAX_HEIGHT << "\n";
	cout << "POINTS:" << POINTS << "\n";
}

int getLinesCleared(GameState &gs) {
	int result = 0;
	for (int i = 0; i < gs.lineClearArr_.size(); i++) {
		result += gs.lineClearArr_[i];
	}
	return result;
}

float opportunityCost(GameState &gs) {
	float points = linePoints(gs);
	int linesCleared = 0;
	for (int i = 0; i < gs.lineClearArr_.size(); i++) {
		linesCleared += gs.lineClearArr_[i];
	}

	if (linesCleared == 0) {
		return 0;
	} else {
		return max(0.0, 3.0 * linesCleared - points);
	}
}

float linePoints(GameState &gs) {
	int currCombo = gs.combo_;
	float result = 0;
	for (int i = 0; i < gs.lineClearArr_.size(); i++) {
		float sum = 0;
		int lines = gs.lineClearArr_[i];
		bool isTspin = gs.tSpinArr_[i];
		bool isPerfectClear = gs.perfClearArr_[i];

		if (isPerfectClear) {
			sum += 18.0 * pow(0.20, max(0, i - 1));
		} else if (isTspin) {
			if (lines == 1) {
				sum += 5.0;
			} else if (lines == 2) {
				sum += 10.0;
			}
		} else {
			if (lines == 2) {
				sum += 3.0;
			} else if (lines == 3) {
				sum += 6.0;
			} else if (lines == 4) {
				sum += 10.0;
			}
		}

		if (lines > 1) {
			sum += currCombo;
			currCombo += 1;
		} else if (lines == 1) {
			sum += currCombo;
		} else {
			currCombo = 0;
		}

		if (i <= 1) {
			sum *= pow(0.96, i);
		} else {
			sum *= pow(POINTS_DECAY, i);
		}
		result += sum;
	}

	return result;
}

float holeScore(BitField &field, const vector<int> &contourProfile, float decay) {
	
	float holeScore = 0;
	for (int i = 1; i < field.height_; i++) {
		for (int j = 0; j < field.width_; j++) {
			if (field.height_ - i < contourProfile[j] && field.getCell(i, j) == EMPTY) {
				int depth = contourProfile[j] - (field.height_ - i);
				holeScore += (1 - pow(decay, depth)) / (1 - decay);
			}
		}
	}

	return holeScore;
}

float holeScore(BitField &field, float decay) {
	return holeScore(field, field.getContourProfile(), decay);
}

float aggregateHeightSqrt(const vector<int> &contourProfile) {
	float sum = 0;
	for (auto &x : contourProfile) {
		sum += sqrt(x);
	}
	return sum;
}

float aggregateHeight(const vector<int> &contourProfile) {
	float sum = 0;
	for (auto &x : contourProfile) {
		sum += x;
	}
	return sum;
}

float aggregateHeightSquared(const vector<int> &contourProfile) {
	float sum = 0;
	for (auto &x : contourProfile) {
		sum += x * x;
	}
	return sqrt(sum);
}

float aggregateHeightSquared(BitField &field) {
	return aggregateHeightSquared(field.getContourProfile());
}

int getMaxHeight(const vector<int> &contourProfile) {
	int result = 0;
	for (auto &x : contourProfile) {
		result = max(result, x);
	}
	return result;
}

int getMaxHeight(BitField &field) {
	int result = 0;
	for (int i = 0; i < field.height_; i++) {
		int row = field.getRow(i);
		if (row != EMPTY_ROW && row != SOLID_ROW) {
			result = field.height_ - i;
			break;
		}
	}

	return result;
}

float tSpinScore(BitField &field) {
	Shape start(T);
	Shape placer(T);

	for (int k = 2; k < 6; k++) {
		for (int i = 1; i < field.height_ - 2; i++) {

			if ((field.getRow(i) & ~field.getRow(i+1)) == 0) {
				continue;
			}

			if ((~field.getRow(i+1) & field.getRow(i+2)) == 0) {
				continue;
			}

			for (int j = 0; j < field.width_ - 1; j++) {
				placer.setParams(i, j, k);

				if (field.isValid(placer) && field.isTSpin(placer)) {
					BitField tmp(field);	
					tmp.placePiece(placer);
					int linesCleared = tmp.clearLines();

					if (linesCleared == 2) {
						return TSPIN_DOUBLE * 10.0;
					} else if (linesCleared == 1) {
						return TSPIN_SINGLE * 10.0;
					} else {
						return TSPIN_ZERO * 10.0;
					}

				}
			}
		}
	}
	return 0.0;
}

float getTriangleArea(const vector<int> &contourProfile, int idx) {
	int center = contourProfile[idx];
	int left = (idx == 0) ? contourProfile[1] : contourProfile[idx - 1];
	int right = (idx == contourProfile.size() - 1) ? contourProfile[contourProfile.size() - 2] : contourProfile[idx + 1];

	if (center + 1 < left && center + 1 < right) {
		int height = max(left, right) - center;
		return (2 * height) - (0.5) * (abs(center - left) + abs(center - right) + 2 * abs(right - left));
	}
	else {
		return 0;
	}
}

float getTriangleArea(BitField &field, int idx) {
	return getTriangleArea(field.getContourProfile(), idx);
}

float getSmoothScore(const vector<int> &contour, int start, int end) {
	float score = 0;
	for (int i = start; i < end - 1 && i < contour.size() - 1; i++) {
		float x = abs(contour[i] - contour[i + 1]);
		score += x;
	}
	return sqrt(score);
	/*
	float score = 0;
	for (int i = start; i < end - 1 && i < contour.size() - 1; i++) {
		float x = abs(contour[i] - contour[i + 1]);
		score += x * x;
	}
	return sqrt(score);
	*/
}

float getSmoothScore(BitField &field) {
	return getSmoothScore(field, 0, field.width_);
}

float getSmoothScore(BitField &field, int start, int end) {
	return getSmoothScore(field.getContourProfile(), start, end);
}

void printIntermediate(GameState &state) {
	BitField &field = state.getField();
	vector<int> contourProfile = field.getContourProfile();

	float maxHeight = getMaxHeight(contourProfile);

	float triangleArea = 0;																			// < 10
	for (int i = 0; i < field.width_; i++) {
		float area = getTriangleArea(contourProfile, i);
		if (area >= 2.0) {
			triangleArea += area;
		}
	}
	
	float points = linePoints(state);										// < 30
	float smoothness = getSmoothScore(contourProfile, 0, field.width_);
	// float smoothness = getSmoothScore(contourProfile, 0, triangleTip)		// < 30
				// + getSmoothScore(contourProfile, triangleTip, field.width_);

	float holes = holeScore(field, contourProfile, HOLE_DECAY);								// < 30
	float height = aggregateHeight(contourProfile); 										// < 63

	float oppCost = opportunityCost(state);
	float tSpins = tSpinScore(field);															// = 10
	tSpins *= max((20.0 + TSPIN_SLOPE * maxHeight) / 20.0, 0.0);

	cout << "Max Height: " << maxHeight << "\n";
	cout << "Triangle Area: " << triangleArea << "\n";
	cout << "Points: " << points << "\n";
	cout << "Smoothness: " << smoothness << "\n";
	cout << "Holes: " << holes << "\n";
	cout << "Height: " << height << "\n";
	cout << "Tspin Score: " << tSpins << "\n";	
	cout << "Opp cost: " << oppCost << "\n";
	cout << "Root MaxHeight: " << state.rootMaxHeight_ << "\n";
	
}


float scoreState(GameState &state) {
	BitField &field = state.getField();
	vector<int> contourProfile = field.getContourProfile();

	int maxHeight = getMaxHeight(contourProfile);

	float triangleArea = 0;																			// < 10
	for (int i = 0; i < field.width_; i++) {
		float area = getTriangleArea(contourProfile, i);
		if (area >= 3.0) {
			triangleArea += area;
		} else {
			triangleArea += 0.5 * area;
		}
	}
	
	float points = linePoints(state);										// < 30
	float smoothness = getSmoothScore(contourProfile, 0, field.width_);
	// float smoothness = getSmoothScore(contourProfile, 0, triangleTip)		// < 30
				// + getSmoothScore(contourProfile, triangleTip, field.width_);

	float holes = holeScore(field, contourProfile, HOLE_DECAY);								// < 30
	// int height = aggregateHeight(contourProfile); 								// < 63
	float tSpins = tSpinScore(field);															// = 10

	float oppCost = opportunityCost(state);

	int effMaxHeight = max(maxHeight + state.linesThreatened_ - 10, 0);
	float maxHeightScore = 1.0 * pow(effMaxHeight, 2) / 10.0;
	if (effMaxHeight > 10) {
		maxHeightScore *= 2.0;
	}

	int rootEffMaxHeight = state.rootMaxHeight_ + state.linesThreatened_;

	float oppMultiplier = 0.0;
	if (rootEffMaxHeight < OPP_THRESH) {
		oppMultiplier = 0.0;
	}
	/*
		static float HOLE_DECAY
		static float MAX_HEIGHT
		static float POINTS
		static float TSPINS
		static float TSPIN_SLOPE 
		static float SMOOTHNESS 
		static float HOLES
		static float HEIGHT
		static float TRIANGLE_AREA
	*/

	return 
		MAX_HEIGHT * maxHeightScore
		+ POINTS * points
		+ tSpins * max((20.0 + TSPIN_SLOPE * rootEffMaxHeight) / 20.0, 0.0)
		+ SMOOTHNESS * smoothness
		+ HOLES * holes
		// + HEIGHT * height / 10.0
		+ TRIANGLE_AREA * triangleArea
		+ OPP_COST * oppCost * oppMultiplier; // max((20.0 + OPP_SLOPE * rootEffMaxHeight) / 20.0, 0.0);
}

