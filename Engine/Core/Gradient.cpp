#include "Gradient.hpp"
#include "Engine/Math/Curves.hpp"
#include "Engine/Core/Rgba.hpp"

Gradient Gradient::white = Gradient(Rgba{255, 255, 255, 255});


Gradient::Gradient(const Rgba& from, const Rgba& to) {
  mColors.reserve(10);
  mColors.push_back({ from, 0.f });
  mColors.push_back({ to, 1.f });
}

Gradient::Gradient(const Rgba& color) {
  mColors.push_back({ color, 0.f });
}

void Gradient::clear() {
  mColors.clear();
}

Gradient& Gradient::add(const Rgba& color, float percent) {
  mColors.push_back({ color, percent });
  return *this;
}

Rgba Gradient::evaluate(float t) const {
  if (mColors.size() == 0) return Rgba::white;

  if (t < mColors.front().percent) return mColors.front().color;

  size_t i = 0, size = mColors.size() - 1;
  while(i<size) {
    const GradientKey& current = mColors[i], next = mColors[i + 1];
    ++i;

    if (t > next.percent) continue;

    ENSURES(t >= current.percent);

    float actualPercent = rangeMap(t, current.percent, next.percent, 0.f, 1.f);

    return lerp(current.color, next.color, actualPercent);
  }

  return mColors.back().color;
}
