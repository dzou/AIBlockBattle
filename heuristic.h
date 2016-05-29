#include "botStarter.h"

#ifndef __HEURISTIC_H
#define __HEURISTIC_H


float scoreState(GameState &gs);

float linePoints(GameState &gs);

float extremaCount(BitField &field);

float getTriangleArea(BitField &field, int idx);

float getSmoothScore(BitField &field);
float getSmoothScore(BitField &field, int start, int end);

float aggregateHeightSquared(BitField &field);
float aggregateHeight(const std::vector<int> &contourProfile);

float holeScore(BitField &field, float decay = 0.5);
float tSpinScore(BitField &field);
int getMaxHeight(BitField &field);

void printValues();
void printIntermediate(GameState &gs);

float opportunityCost(GameState &gs);

inline double fastPow(double a, double b) {
  union {
    double d;
    int x[2];
  } u = { a };
  u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
  u.x[0] = 0;
  return u.d;
}

#endif // __HEURISTIC_H