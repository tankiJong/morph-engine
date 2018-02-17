#pragma once
#include "Engine/Math/Primitives/vec2.hpp"

class aabb2 {
public:
	~aabb2() {}
	aabb2() {}
	aabb2(const aabb2& copy);
	aabb2(float minX, float minY, float maxX, float maxY);
	aabb2(const vec2& mins, const vec2& maxs);
	aabb2(const vec2& center, float radiusX, float radiusY);
  aabb2(float width, float height, const vec2& mins);

	void stretchToIncludePoint(float x, float y); // note: stretch, not move…
	void stretchToIncludePoint(const vec2& point); // note: stretch, not move…
	void addPaddingToSides(float xPaddingRadius, float yPaddingRadius);
	void translate(const vec2& translation); // move the box; similar to +=
	void translate(float translationX, float translationY);

  std::array<vec2,4> vertices() const;
	bool isPointInside(float x, float y) const; // is “x,y” within box’s interior?
	bool isPointInside(const vec2& point) const; // is “point” within box’s interior?
	vec2 getDimensions() const; // return a Vector2 of ( width, height )
	vec2 getCenter() const; // return the center position of the box
  float width() const;
  float height() const;
	void operator+=(const vec2& translation); // move (translate) the box
	void operator-=(const vec2& antiTranslation);
	aabb2 operator+(const vec2& translation) const; // create a (temp) moved box
	aabb2 operator-(const vec2& antiTranslation) const;
  void fromString(const char* data);
  std::string toString() const;
	vec2 mins; // like Vector2, this breaks the “no public members” and “m_” naming rules;
	vec2 maxs; // this is so low-level math primitives feel like built-in types (and are fast in Debug)
};

bool areAABBsOverlap(const aabb2& a, const aabb2& b);