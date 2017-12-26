#pragma once

struct Vector4;
class Vector3;
class Vector2;
class Noise;
namespace noise {
  class Addition;
  class Subtraction;
  class Multiplication;
  class Division;
}
class Noise {
public:


  virtual float at(float pos) = 0;
  virtual float at(const Vector2& pos) = 0;
  virtual float at(const Vector3& pos) = 0;
  virtual float at(const Vector4& pos) = 0;

  template<typename D>
  float operator()(const D& pos) {
    return at(pos);
  }
  friend noise::Addition operator+(const Noise& a, const Noise& b);
  friend noise::Subtraction operator-(const Noise& a, const Noise& b);
  friend noise::Multiplication operator*(const Noise& a, const Noise& b);
  friend noise::Division operator/(const Noise& a, const Noise& b);
};

class noise::Addition: public Noise {
  
};

class noise::Subtraction : public Noise {

};

class noise::Multiplication : public Noise {

};

class noise::Division : public Noise {

};

noise::Addition operator+(const Noise& a, const Noise& b);
noise::Subtraction operator-(const Noise& a, const Noise& b);
noise::Multiplication operator*(const Noise& a, const Noise& b);
noise::Division operator/(const Noise& a, const Noise& b);
