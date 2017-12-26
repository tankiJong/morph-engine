#include "Engine/Math/Curves.hpp"

CubicSpline2D::CubicSpline2D(const Vector2* positionsArray, int numPoints, const Vector2* velocitiesArray) {
  appendPoints(positionsArray, numPoints, velocitiesArray);
}

