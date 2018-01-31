#pragma once
#include <functional>
#include "Engine/Core/Utils.hpp"
class Noise;
class SumNoise;
class ProductNoise;

// TODO: the clone operation have protential performance problem
class Noise {
public:
  virtual ~Noise();
  SumNoise operator+(const Noise& rhs) const;
  SumNoise operator-(const Noise& rhs) const;
  ProductNoise operator*(const Noise& rhs) const;
  ProductNoise operator/(const Noise& rhs) const;

  template<typename T>
  inline float operator()(const T& v) const { return this->evaluate(v); };

  friend SumNoise operator+(float lhs, const Noise& rhs);
  friend ProductNoise operator*(float lhs, const Noise& rhs);

  friend SumNoise operator-(float lhs, const Noise& rhs);
  friend ProductNoise operator/(float lhs, const Noise& rhs);

  virtual float evaluate(float i) const = 0;
  virtual Noise* clone() const = 0;
};

SumNoise operator+(float lhs, const Noise& rhs);
ProductNoise operator*(float lhs, const Noise& rhs);

SumNoise operator-(float lhs, const Noise& rhs);
ProductNoise operator/(float lhs, const Noise& rhs);

class ConstNoise final: public Noise {
public:
  ConstNoise(float val): val(val) {}
  float evaluate(float i) const override { return val; };
  ConstNoise* clone() const override;
protected:
  float val;
};

class SumNoise : public Noise {
public:
  SumNoise() = default;
  SumNoise(const SumNoise& from);

  SumNoise(const Noise& lhs, const Noise& rhs)
    : lhs(lhs.clone()), rhs(rhs.clone()) {};

  SumNoise(const Noise& lhs, float rhs)
    : lhs(lhs.clone()), rhs(new ConstNoise(rhs)) {};

  SumNoise(float lhs, const Noise& rhs)
    : lhs(new ConstNoise(lhs)), rhs(rhs.clone()) {};

  float evaluate(float i) const override;
  Noise* clone() const override;
  ~SumNoise() override;
protected:
  owner<Noise*> lhs = nullptr;
  owner<Noise*> rhs = nullptr;
};

class ProductNoise : public Noise {
public:
  ProductNoise() = default;
  ProductNoise(const ProductNoise& from);

  ProductNoise(const Noise& lhs, const Noise& rhs)
    : lhs(lhs.clone()), rhs(rhs.clone()) {};

  ProductNoise(const Noise& lhs, float rhs)
    : lhs(lhs.clone()), rhs(new ConstNoise(rhs)) {}; 

  ProductNoise(float lhs, const Noise& rhs)
    : lhs(new ConstNoise(lhs)), rhs(rhs.clone()) {};

  float evaluate(float i) const override;
  Noise* clone() const override;
  ~ProductNoise() override;
protected:
  owner<Noise*> lhs = nullptr;
  owner<Noise*> rhs = nullptr;
};