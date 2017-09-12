#pragma once
#include "Engine\Math\Vector2.hpp"

class Disc2 {
public:
	Vector2 center; // like Vector2, this breaks the ¡°no public members¡± and ¡°m_¡± naming rules
	float radius;

public:
	~Disc2() {}
	Disc2() {}
	Disc2(const Disc2& copyFrom);
	explicit Disc2(float initialX, float initialY, float initialRadius);
	explicit Disc2(const Vector2& initialCenter, float initialRadius);
	void stretchToIncludePoint(float x, float y); // expand radius if (x,y) is outside
	void stretchToIncludePoint(const Vector2& point); // expand radius if point outside
	void addPaddingToRadius(float paddingRadius);
	void translate(const Vector2& translation); // move the center
	void translate(float translationX, float translationY); // move the center
	bool isPointInside(float x, float y) const; // is (x,y) within disc¡¯s interior?
	bool isPointInside(const Vector2& point) const; // is ¡°point¡± within disc¡¯s interior?
	void operator+=(const Vector2& translation); // move
	void operator-=(const Vector2& antiTranslation);
	Disc2 operator+(const Vector2& translation) const; // create a moved copy
	Disc2 operator-(const Vector2& antiTranslation) const;
};

bool areDiscsOverlap(const Disc2& a, const Disc2& b);
bool areDiscsOverlap(const Vector2& aCenter, float aRadius,
	const Vector2& bCenter, float bRadius);