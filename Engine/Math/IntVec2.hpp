#pragma once
#include <string>

class vec2;
class IntVec2 {
public:
	IntVec2() {};
	~IntVec2() {};
	// Construction/Destruction
	IntVec2(const IntVec2& copyFrom);						// copy constructor (from another vec2)
	explicit IntVec2(int initialX, int initialY);		// explicit constructor (from x, y)
  explicit IntVec2(const vec2& castFrom);
  IntVec2(std::initializer_list<int> list);

															// Operators
	const IntVec2 operator+(const IntVec2& vecToAdd) const;		// vec2 + vec2
  const vec2    operator+(const vec2& vecToAdd) const;
	const IntVec2 operator-(const IntVec2& vecToSubtract) const;	// vec2 - vec2
	const IntVec2 operator*(int uniformScale) const;			// vec2 * int
	int operator*(const IntVec2& another) const;            // vec2 * vec2, dot product
	const IntVec2 operator/(int inverseScale) const;			// vec2 / int
	void operator+=(const IntVec2& vecToAdd);						// vec2 += vec2
	void operator-=(const IntVec2& vecToSubtract);				// vec2 -= vec2
	void operator*=(const int uniformScale);					// vec2 *= int
	void operator/=(const int uniformDivisor);					// vec2 /= int
	void operator=(const IntVec2& copyFrom);						// vec2 = vec2
	bool operator==(const IntVec2& compare) const;				// vec2 == vec2
	bool operator!=(const IntVec2& compare) const;				// vec2 != vec2

  void fromString(const char* data);
  std::string toString() const;
public:
	int x = 0;
	int y = 0;
	static const IntVec2 zero;
  static const IntVec2 top;
  static const IntVec2 down;
  static const IntVec2 left;
  static const IntVec2 right;
};
