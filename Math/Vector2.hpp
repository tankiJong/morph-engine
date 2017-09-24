#pragma once


//-----------------------------------------------------------------------------------------------
class Vector2 {
public:
    // Construction/Destruction
    ~Vector2 () {}											// destructor: do nothing (for speed)
    Vector2 () {}											// default constructor: do nothing (for speed)
    Vector2 (const Vector2& copyFrom);						// copy constructor (from another vec2)
    explicit Vector2 (float initialX, float initialY);		// explicit constructor (from x, y)

                                                            // Operators
    const Vector2 operator+(const Vector2& vecToAdd) const;		// vec2 + vec2
    const Vector2 operator-(const Vector2& vecToSubtract) const;	// vec2 - vec2
    const Vector2 operator*(float uniformScale) const;			// vec2 * float
    const Vector2 operator/(float inverseScale) const;			// vec2 / float
    void operator+=(const Vector2& vecToAdd);						// vec2 += vec2
    void operator-=(const Vector2& vecToSubtract);				// vec2 -= vec2
    void operator*=(const float uniformScale);					// vec2 *= float
    void operator/=(const float uniformDivisor);					// vec2 /= float
    void operator=(const Vector2& copyFrom);						// vec2 = vec2
    bool operator==(const Vector2& compare) const;				// vec2 == vec2
    bool operator!=(const Vector2& compare) const;				// vec2 != vec2

	float getLength() const;
	float getLengthSquared() const; // faster than GetLength() since it skips the sqrtf()
	float normalizeAndGetLength(); // set my new length to 1.0f; keep my direction
	Vector2 getNormalized() const; // return a new vector, which is a normalized copy of me
	float getOrientationDegrees() const; // return 0 for east (5,0), 90 for north (0,8), etc.

	static Vector2 makeDirectionAtDegrees(float degrees); // create vector at angle
    friend const Vector2 operator*(float uniformScale, const Vector2& vecToScale);	// float * vec2

	

public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
    float x;
    float y;
	static const Vector2 zero;
};


