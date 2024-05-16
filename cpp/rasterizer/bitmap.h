#pragma once

#include "color.h"
#include <vector>
#include <string_view>
#include <string>
#include <optional>
#include <algorithm>

namespace BMP {

struct Image {
  std::optional<std::string> source = std::nullopt;
  int w = 0, h = 0;
  std::vector<Color> pixels;
  Image() = default;
  ~Image() = default;
  bool isValid() const {
    return w > 0 && h > 0 && pixels.size() == size_t(w * h);
  }

  Color get(int x, int y, bool clamp = true) const {
    if (!clamp) {
      x %= w;
      y %= h;
      x += x < 0 ? w : 0;
      y += y < 0 ? h : 0;
    } else {
      x = std::clamp(x, 0, w - 1);
      y = std::clamp(y, 0, h - 1);
    }
    return pixels.size() ? pixels[y * w + x] : Color();
  }
  void put(int x, int y, Color color, bool clamp = true) {
    if (!clamp) {
      x %= w;
      y %= h;
      x += x < 0 ? w : 0;
      y += y < 0 ? h : 0;
    } else {
      x = std::clamp(x, 0, w - 1);
      y = std::clamp(y, 0, h - 1);
    }
    if (pixels.size())
      pixels[y * w + x] = color;

  }

  Color sample(float x, float y, bool clamp = false) const;
};

Image imageFromBitmapData(std::vector<uint8> rawData, std::string_view source);
bool writeImageToBitmapFile(const Image &image, std::string_view name);
}
