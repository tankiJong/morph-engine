#pragma once
#include <vector>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////////
// Standalone curve utility functions
//
// Note: Quadratic Bezier functions (only) specify an actual midpoint, not a Bezier guide point
/////////////////////////////////////////////////////////////////////////////////////////////////

template< typename T >
T evaluateQuadraticBezier(const T& startPos, const T& actualCurveMidPos, const T& endPos, float t) {
  const auto& guide = actualCurveMidPos * 2.f - (startPos + endPos) * .5f;
  const auto& a = lerp(startPos, guide, t);
  const auto& b = lerp(guide, endPos, t);
  return lerp(a, b, t);
};

template< typename T >
T evaluateCubicBezier(const T& startPos, const T& guidePos1, const T& guidePos2, const T& endPos, float t) {
  const auto& a = lerp(startPos, guidePos1, t);
  const auto& b = lerp(guidePos1, guidePos2, t);
  const auto& c = lerp(guidePos2, endPos, t);

  const auto& d = lerp(a, b, t);
  const auto& e = lerp(b, c, t);

  return lerp(d, e, t);

};

template< typename T >
T evaluateCubicHermite(const T& startPos, const T& startVel, const T& endPos, const T& endVel, float t) {
  auto guide1 = startPos + startVel / 3.f;
  auto guide2 = endPos - endVel / 3.f;

  return evaluateCubicBezier(startPos, guide1, guide2, endPos, t);
};


/////////////////////////////////////////////////////////////////////////////////////////////////
// CubicSpline2D
// 
// Cubic Hermite/Bezier spline of Vector2 positions / velocities
/////////////////////////////////////////////////////////////////////////////////////////////////
class CubicSpline2D {
public:
  CubicSpline2D() {}
  explicit CubicSpline2D(const vec2* positionsArray, int numPoints, const vec2* velocitiesArray = nullptr);
  ~CubicSpline2D() {}

  // Mutators
  inline void	 appendPoint(const vec2& position, const vec2& velocity = vec2::zero) {
    m_positions.push_back(position);
    m_velocities.push_back(velocity);
  };
  void	 appendPoints(const vec2* positionsArray, int numPoints, const vec2* velocitiesArray = nullptr) {
    m_positions.insert(m_positions.end(), positionsArray, positionsArray + numPoints);
    m_velocities.reserve(m_positions.size());
    if (velocitiesArray == nullptr) {
      for (int i = 0; i < numPoints; i++) {
        m_velocities.emplace_back(0.f, 0.f);
      }
    } else {
      m_velocities.insert(m_velocities.end(), velocitiesArray, velocitiesArray + numPoints);
    }
  };
  inline void	 insertPoint(int insertBeforeIndex, const vec2& position, const vec2& velocity = vec2::zero) {
    GUARANTEE_OR_DIE(insertBeforeIndex < (int)m_positions.size(), "index out of range");
    m_positions.insert(m_positions.begin() + insertBeforeIndex, position);
    m_velocities.insert(m_velocities.begin() + insertBeforeIndex, velocity);
  };
  inline void	 removePoint(int pointIndex) {
    m_positions.erase(m_positions.begin() + pointIndex);
    m_velocities.erase(m_velocities.begin() + pointIndex);
  };
  inline void	 removeAllPoints() {
    m_positions.clear();
    m_velocities.clear();
  };
  inline void	 setPoint(int pointIndex, const vec2& newPosition, const vec2& newVelocity) {
    GUARANTEE_OR_DIE(pointIndex < (int)m_positions.size(), "index out of range");

    m_positions[pointIndex] = newPosition;
    m_velocities[pointIndex] = newVelocity;
  };
  inline void	 setPosition(int pointIndex, const vec2& newPosition) {
    GUARANTEE_OR_DIE(pointIndex < (int)m_positions.size(), "index out of range");
    m_positions[pointIndex] = newPosition;

  };
  inline void	 setVelocity(int pointIndex, const vec2& newVelocity) {
    GUARANTEE_OR_DIE(pointIndex < (int)m_positions.size(), "index out of range");
    m_velocities[pointIndex] = newVelocity;
  };
  void setCardinalVelocities(float tension = 0.f, const vec2& startVelocity = vec2::zero,
                             const vec2& endVelocity = vec2::zero) {
    m_velocities[0] = startVelocity;
    for (size_t i = 1; i < m_positions.size() - 1; i++) {
      m_velocities[i] = (m_positions[i + 1] - m_positions[i - 1]) * .5f * (1.f - tension);
    }
    m_velocities.back() = endVelocity;
  }

  // Accessors
  inline int   getNumPoints() const { return (int)m_positions.size(); }
  inline const vec2	getPosition(int pointIndex) {
    return m_positions[pointIndex];
  };
  inline const vec2	getVelocity(int pointIndex) {
    return m_velocities[pointIndex];
  };

  const std::vector<vec2>& getPositions() const {
    return m_positions;
  }

  const std::vector<vec2>& getVelocities() const {
    return m_velocities;
  }
  int				getPositions(std::vector<vec2>& out_positions) {
    out_positions = m_positions;
    return (int)m_positions.size();
  };
  int				getVelocities(std::vector<vec2>& out_velocities) {
    out_velocities = m_velocities;
    return (int)m_velocities.size();
  };
  vec2			evaluateAtCumulativeParametric(float t) const {
    if (t + 1.f > (float)m_positions.size()) {
      return m_positions.back();
    }

    float a = floor(t), b = ceil(t);

    return evaluateCubicHermite(m_positions[int(a)], m_velocities[int(a)], m_positions[int(b)], m_velocities[int(b)], t - a);
  };
  vec2			evaluateAtNormalizedParametric(float t) const {
    float tick = 1.f / (m_positions.size() - 1);

    int index = 0;
    while (t > tick) {
      t -= tick;
      index++;
    }
    return evaluateCubicHermite(m_positions[index], m_velocities[index], m_positions[index + 1], m_velocities[index + 1], t / tick);
  };

protected:
  std::vector<vec2>	m_positions;
  std::vector<vec2>	m_velocities;
};



