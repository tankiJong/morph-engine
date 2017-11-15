#pragma once
#include <string>

class IntVector2;
class Vector3;
namespace std {
  template< typename T >
  class initializer_list;
}

//-----------------------------------------------------------------------------------------------
class Vector2 {
public:
  // Construction/Destruction
  ~Vector2() {} // destructor: do nothing (for speed)
  Vector2() {} // default constructor: do nothing (for speed)
  Vector2(const Vector2& copyFrom); // copy constructor (from another vec2)
  Vector2(const IntVector2& copyFrom); // convert from IntVec2)
  Vector2(const Vector3& copyFrom);   // need explicit convertion to use as vector3
  explicit Vector2(float initialX, float initialY); // explicit constructor (from x, y)
  Vector2(std::initializer_list<float>);
  Vector2(const char* str);
  void fromString(const char* data);
  std::string toString() const;

  // Operators
  const Vector2 operator+(const Vector2& vecToAdd) const; // vec2 + vec2
  const Vector2 operator-(const Vector2& vecToSubtract) const; // vec2 - vec2
  const Vector2 operator*(float uniformScale) const; // vec2 * float
  const Vector2 operator/(float inverseScale) const; // vec2 / float
  void operator=(const Vector3& copyFrom); // vec2 = vec3
  void operator+=(const Vector2& vecToAdd); // vec2 += vec2
  void operator-=(const Vector2& vecToSubtract); // vec2 -= vec2
  void operator*=(const float uniformScale); // vec2 *= float
  void operator/=(const float uniformDivisor); // vec2 /= float
  void operator=(const Vector2& copyFrom); // vec2 = vec2
  bool operator==(const Vector2& compare) const; // vec2 == vec2
  bool operator!=(const Vector2& compare) const; // vec2 != vec2

  float getLength() const;
  float getLengthSquared() const; // faster than GetLength() since it skips the sqrtf()
  float normalizeAndGetLength(); // set my new length to 1.0f; keep my direction
  Vector2 getNormalized() const; // return a new vector, which is a normalized copy of me
  float getOrientationDegrees() const; // return 0 for east (5,0), 90 for north (0,8), etc.
  float dot(const Vector2& another) const;
  static Vector2 makeDirectionAtDegrees(float degrees); // create vector at angle
  static float dotProduct(const Vector2& a, const Vector2& b);
  friend const Vector2 operator*(float uniformScale, const Vector2& vecToScale); // float * vec2

public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
  float x = 0;
  float y = 0;
  static const Vector2 zero;
  static const Vector2 top;
  static const Vector2 down;
  static const Vector2 left;
  static const Vector2 right;
};


// Gets the projected vector in the “projectOnto” direction, whose magnitude is the projected length of “vectorToProject” in that direction.
const Vector2 projectTo(const Vector2& vectorToProject, const Vector2& projectOnto);
const Vector2 transform(const Vector2& originalVector, 
                        const Vector2& fromX, const Vector2& fromY,
                        const Vector2& toI, const Vector2& toJ);

// Returns the vector’s representation/coordinates in (i,j) space (instead of its original x,y space)
const Vector2 transToBasis(const Vector2& originalVector,
                           const Vector2& toBasisI, const Vector2& toBasisJ);

// Takes “vectorInBasis” in (i,j) space and returns the equivalent vector in [axis-aligned] (x,y) Cartesian space
const Vector2 transFromBasis(const Vector2& originalVector,
                        const Vector2& fromBasisI, const Vector2& fromBasisJ);

// Decomposes “originalVector” into two component vectors, which add up to the original:
//   “vectorAlongI” is the vector portion in the “newBasisI” direction, and
//   “vectorAlongJ” is the vector portion in the “newBasisJ” direction.
void decompose(const Vector2& originalVector,
                              const Vector2& newBasisI, const Vector2& newBasisJ,
                              Vector2& out_vectorAlongI, Vector2& out_vectorAlongJ);
