#pragma once

#include "defs.h"
#include "myvector.h"
#include "texture.h"

#include <string>
#include <variant>
#include <functional>
#include <optional>
#include <type_traits>

namespace rasterizer {

enum VariableType {
  ScalarType,
  Vector2Type,
  Vector3Type,
  ColorType,
  TexelType,
};

constexpr size_t maxLayers = 8;

template<class T>
struct VariableBase {
  static_assert(std::is_same<T, float>::value || std::is_same<T, Vector2>::value || std::is_same<T, Vector3>::value || std::is_same<T, Color>::value || std::is_same<T, Texel>::value, "variable type not supported" );

  static constexpr VariableType type = []() {
    if (std::is_same<T, float>::value)
      return ScalarType;
    else if (std::is_same<T, Vector2>::value)
      return Vector2Type;
    else if (std::is_same<T, Vector3>::value)
      return Vector3Type;
    return ColorType;
  }();
  T v;
  virtual T vary(const T &next, float alpha) const {
    return alpha >= 0.5f ? next : v;
  }
  std::string getType() {
    switch (type) {
      case ScalarType:
        return "ScalarType";
        break;
      case Vector2Type:
        return "Vector2Type";
        break;
      case Vector3Type:
        return "Vector3Type";
        break;
      case ColorType:
        return "ColorType";
        break;
      case TexelType:
        return "TexelType";
        break;
    }
    return "the hell???";
  }

  VariableBase() = default;
  VariableBase(T v)
      :
      v(v) {
  }
  virtual ~VariableBase() = default;
}
;

struct ScalarVariable : public VariableBase<float> {
  float vary(const float &next, float alpha) const override {
    return v + alpha * (next - v);
  }
};

struct Vector2Variable : public VariableBase<Vector2> {
  Vector2 vary(const Vector2 &next, float alpha) const override {
    return v + alpha * (next - v);
  }
};

struct Vector3Variable : public VariableBase<Vector3> {
  Vector3 vary(const Vector3 &next, float alpha) const override {
    return v + alpha * (next - v);
  }
};

struct ColorVariable : public VariableBase<Color> {
  Color vary(const Color &next, float alpha) const override {
    return v.lerp(next, alpha);
  }
};

struct TexelVariable : public VariableBase<Texel> {
  Texel vary(const Texel &next, float alpha) const override {
    if (v.textureHandle >= 0) {
      Texel t;
      if (v.textureHandle == next.textureHandle) {
        t.textureHandle = v.textureHandle;
        t.uv = v.uv + alpha * (next.uv - v.uv);
      } else {
        t.textureHandle = alpha >= 0.5f ? next.textureHandle : v.textureHandle;
        t.uv = alpha >= 0.5f ? next.uv : v.uv;
      }
      return t;
    }
    return v;
  }
  void set(Vector2 uv, int textureHandle) {
    v.textureHandle = textureHandle;
    v.uv = uv;
  }
};

using Variable = std::variant< ScalarVariable, Vector2Variable, Vector3Variable, ColorVariable, TexelVariable>;

using Variables = std::variant< std::vector<ScalarVariable>, std::vector<Vector2Variable>, std::vector<Vector3Variable>, std::vector<ColorVariable>, std::vector<TexelVariable> >;
using Plot = std::vector<Variable>;
using Layers = std::vector<Variables>;

struct Point {
  Vector2 p;
  Plot plot;
  static Point blendPoints(const Point &begin, const Point &end, float alpha);
};

struct Canvas {
  uint32 w = 0, h = 0;
  Layers layers;

  Canvas() = default;
  Canvas(uint32 w, uint32 h, const std::vector<VariableType> &types);

  size_t xy(int32 x, int32 y) {
    x = std::clamp(x, 0, int32(w - 1));
    y = std::clamp(y, 0, int32(h - 1));
    return size_t(y * w + x);
  }

  /*
   template<class T> void fill(size_t layerNo) {
   static_assert( IsContainedIn<T, Variable>::value, "Canvas::fill error");
   if (layerNo < layers.size()) {
   auto &layer = std::get<std::vector<T>>(layers[layerNo]);
   layer.clear();
   for (size_t i = 0; i < w * h; i++) {
   layer.emplace_back();
   }
   }
   }
   */

  Point createPoint(int32 x = 0, int32 y = 0);
  void plotPoint(const Point &pt, int32 x, int32 y);
};

struct Scanner {
 protected:
  struct Pair {
    std::optional<int> y = std::nullopt;
    Point pt0, pt1;
    Pair() = default;

    void push(int y, const Point &pt);
    void reset() {
      y = std::nullopt;
    }
  };

  int yTop = -1;
  int yBottom = INT_MAX;
  Canvas &canvas;
  std::vector<Pair> pairs;

  void pushPoint(const Point &pt);
  void plotPoint(const Point &pt, int x, int y);
  void scanLine(const Point &pt0, const Point &pt1, int y);

 public:
  Scanner(Canvas &canvas)
      :
      canvas(canvas) {
    pairs = std::vector<Pair>(canvas.h);
  }
  void reset() {
    yTop = -1;
    yBottom = INT_MAX;
    for (auto &pair : pairs) {
      pair.reset();
    }
  }

  void buildEdge(const Point &pt0, const Point &pt1);
  void scan();
};

}
