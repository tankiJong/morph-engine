#include "Engine/Math/Curves.hpp"

CubicSpline2D::CubicSpline2D(const vec2* positionsArray, int numPoints, const vec2* velocitiesArray) {
  appendPoints(positionsArray, numPoints, velocitiesArray);
}

