#include "Noise.hpp"

class OppositeNoise final : public Noise {
public:
  OppositeNoise(const Noise& val) : val(val.clone()) {}
  ~OppositeNoise() {
    delete val;
  }
  float evaluate(float i) const override { return 1.f / val->evaluate(i); };
  OppositeNoise* clone() const override;
protected:
  owner<Noise*> val = nullptr;
};

class InverseNoise final : public Noise {
public:
  InverseNoise(const Noise& val) : val(val.clone()) {}
  ~InverseNoise() {
    delete val;
  }
  float evaluate(float i) const override { return -1.f * val->evaluate(i); };
  InverseNoise* clone() const override;
protected:
  owner<Noise*> val = nullptr;
};

Noise::~Noise() {}
SumNoise Noise::operator+(const Noise& rhs) const {
  return SumNoise(*this, rhs);
}

SumNoise Noise::operator-(const Noise& rhs) const {
  return *this + OppositeNoise(rhs);
}

ProductNoise Noise::operator*(const Noise& rhs) const {
  return ProductNoise(*this, rhs);
}

ProductNoise Noise::operator/(const Noise& rhs) const {
  return *this * InverseNoise(rhs);
}

SumNoise operator+(float lhs, const Noise& rhs) {
  return SumNoise(lhs, rhs);
}
ProductNoise operator*(float lhs, const Noise& rhs) {
  return ProductNoise(lhs, rhs);
}

SumNoise operator-(float lhs, const Noise& rhs) {
  return SumNoise(lhs, rhs);
}
ProductNoise operator/(float lhs, const Noise& rhs) {
  return ProductNoise(lhs, InverseNoise(rhs));
}

ConstNoise* ConstNoise::clone() const {
  return new ConstNoise(*this);
}

SumNoise::SumNoise(const SumNoise& from): lhs(from.lhs->clone()), rhs(from.rhs->clone()) {
  
}

float SumNoise::evaluate(float i) const {
  return lhs->evaluate(i) + rhs->evaluate(i);
}

SumNoise::~SumNoise() {
  delete lhs;
  delete rhs;
}

ProductNoise::ProductNoise(const ProductNoise& from): lhs(from.lhs->clone()), rhs(from.rhs->clone()) {
}

float ProductNoise::evaluate(float i) const {
  return lhs->evaluate(i) * rhs->evaluate(i);
}

ProductNoise::~ProductNoise() {
  delete lhs;
  delete rhs;
}