#pragma once
#include "Engine/Core/common.hpp"
#include <vector>
#include "Rgba.hpp"

class Gradient {
public:
  struct GradientKey{
    Rgba color;
    float percent;
  };

  Gradient() { mColors.reserve(10); };
  Gradient(const Rgba& from, const Rgba& to);
  Gradient(const Rgba& color);
  void clear();

  Gradient& add(const Rgba& color, float percent);

  Rgba evaluate(float t) const;

  static Gradient white;
protected:

  // should sort according to percent
  std::vector<GradientKey> mColors;
};
