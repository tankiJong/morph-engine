#pragma once
#include "Engine/Math/Vector2.hpp"

class AABB2 {
public:
	~AABB2() {}
	AABB2() {}
	AABB2(const AABB2& copy);
	AABB2(float minX, float minY, float maxX, float maxY);
	AABB2(const Vector2& mins, const Vector2& maxs);
	AABB2(const Vector2& center, float radiusX, float radiusY);
  AABB2(float width, float height, const Vector2& mins);

	void stretchToIncludePoint(float x, float y); // note: stretch, not move…
	void stretchToIncludePoint(const Vector2& point); // note: stretch, not move…
	void addPaddingToSides(float xPaddingRadius, float yPaddingRadius);
	void translate(const Vector2& translation); // move the box; similar to +=
	void translate(float translationX, float translationY);

  std::array<Vector2,4> vertices() const;
	bool isPointInside(float x, float y) const; // is “x,y” within box’s interior?
	bool isPointInside(const Vector2& point) const; // is “point” within box’s interior?
	Vector2 getDimensions() const; // return a Vector2 of ( width, height )
	Vector2 getCenter() const; // return the center position of the box
  float width() const;
  float height() const;
	void operator+=(const Vector2& translation); // move (translate) the box
	void operator-=(const Vector2& antiTranslation);
	AABB2 operator+(const Vector2& translation) const; // create a (temp) moved box
	AABB2 operator-(const Vector2& antiTranslation) const;
  void fromString(const char* data);
  std::string toString() const;
	Vector2 mins; // like Vector2, this breaks the “no public members” and “m_” naming rules;
	Vector2 maxs; // this is so low-level math primitives feel like built-in types (and are fast in Debug)
};

bool areAABBsOverlap(const AABB2& a, const AABB2& b);