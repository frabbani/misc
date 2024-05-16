#pragma once

#include "defs.h"

#pragma pack(push,1)
struct Color {
  union {
    struct {
      uint8 r, g, b;
    };
    uint8 rgb[3];
  };
  Color(uint8 r = 0, uint8 g = 0, uint8 b = 0)
      :
      r(r),
      g(g),
      b(b) {
  }
  Color(const Color &rhs)
      :
      r(rhs.r),
      g(rhs.g),
      b(rhs.b) {
  }

  Color lerp(Color that, float alpha) const {
    alpha = alpha < 0.0f ? 0.0f : alpha > 1.0f ? 1.0f : alpha;
    float rf = (float) r + alpha * (float) (that.r - r);
    float gf = (float) g + alpha * (float) (that.g - g);
    float bf = (float) b + alpha * (float) (that.b - b);
    return Color(rf, gf, bf);
  }

};
#pragma pack(pop)
