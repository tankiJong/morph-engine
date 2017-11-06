#pragma once
#include <string>

class Vector2;
class IntVector2 {
public:
	IntVector2() {};
	~IntVector2() {};
	// Construction/Destruction
	IntVector2(const IntVector2& copyFrom);						// copy constructor (from another vec2)
	explicit IntVector2(int initialX, int initialY);		// explicit constructor (from x, y)
  explicit IntVector2(const Vector2& castFrom);
  IntVector2(std::initializer_list<int> list);

															// Operators
	const IntVector2 operator+(const IntVector2& vecToAdd) const;		// vec2 + vec2
	const IntVector2 operator-(const IntVector2& vecToSubtract) const;	// vec2 - vec2
	const IntVector2 operator*(int uniformScale) const;			// vec2 * int
	int operator*(const IntVector2& another) const;            // vec2 * vec2, dot product
	const IntVector2 operator/(int inverseScale) const;			// vec2 / int
	void operator+=(const IntVector2& vecToAdd);						// vec2 += vec2
	void operator-=(const IntVector2& vecToSubtract);				// vec2 -= vec2
	void operator*=(const int uniformScale);					// vec2 *= int
	void operator/=(const int uniformDivisor);					// vec2 /= int
	void operator=(const IntVector2& copyFrom);						// vec2 = vec2
	bool operator==(const IntVector2& compare) const;				// vec2 == vec2
	bool operator!=(const IntVector2& compare) const;				// vec2 != vec2

  void fromString(const char* data);
  std::string toString() const;
public:
	int x = 0;
	int y = 0;
	static const IntVector2 zero;
  static const IntVector2 top;
  static const IntVector2 down;
  static const IntVector2 left;
  static const IntVector2 right;
};
