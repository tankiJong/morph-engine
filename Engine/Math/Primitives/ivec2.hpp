#pragma once
#include <string>

class uvec2;
class vec2;
class ivec2 {
public:
	ivec2() {};
	~ivec2() {};
	// Construction/Destruction
	ivec2(const ivec2& copyFrom);						// copy constructor (from another vec2)
	ivec2(int initialX, int initialY);		// explicit constructor (from x, y)
  explicit ivec2(const vec2& castFrom);
  explicit ivec2(const uvec2& castFrom);

															// Operators
	const ivec2 operator+(const ivec2& vecToAdd) const;		// vec2 + vec2
  const vec2    operator+(const vec2& vecToAdd) const;
	const ivec2 operator-(const ivec2& vecToSubtract) const;	// vec2 - vec2
	const ivec2 operator*(int uniformScale) const;			// vec2 * int
	int operator*(const ivec2& another) const;            // vec2 * vec2, dot product
	const ivec2 operator/(int inverseScale) const;			// vec2 / int
	void operator+=(const ivec2& vecToAdd);						// vec2 += vec2
	void operator-=(const ivec2& vecToSubtract);				// vec2 -= vec2
	void operator*=(const int uniformScale);					// vec2 *= int
	void operator/=(const int uniformDivisor);					// vec2 /= int
	void operator=(const ivec2& copyFrom);						// vec2 = vec2
	bool operator==(const ivec2& compare) const;				// vec2 == vec2
	bool operator!=(const ivec2& compare) const;				// vec2 != vec2

  void fromString(const char* data);
  std::string toString() const;
public:
	int x = 0;
	int y = 0;
	static const ivec2 zero;
  static const ivec2 top;
  static const ivec2 down;
  static const ivec2 left;
  static const ivec2 right;
};
