#pragma once

#include "myvector.h"
#include "bitmap.h"
#include <vector>

void writePPM(const Color *pixels, uint32 w, uint32 h, const char name[]);

bool loadTexture(const BMP::Image &image, std::string_view tag, int &handle);

struct Texel : public Color {
  int textureHandle = -1;
  Vector2 uv;
  Color sample() const;
};

