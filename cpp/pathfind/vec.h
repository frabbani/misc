#pragma once

#include <cmath>

constexpr float eps = 1e-6f;
constexpr float epsSq = 1e-12f;  // degrees, arbitrary max spread for a gun

constexpr float PI = 3.14159265358979323846f;
constexpr float DEG_TO_RAD = PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / PI;

struct Vec2 {
  float x, y;
  Vec2() : x(0), y(0) {}
  Vec2(float x, float y) : x(x), y(y) {}
  Vec2 operator+(const Vec2& other) const { return {x + other.x, y + other.y}; }
  Vec2 operator-(const Vec2& other) const { return {x - other.x, y - other.y}; }
  Vec2 operator*(float scalar) const { return {x * scalar, y * scalar}; }
  Vec2 operator/(float scalar) const {
    float inv = 1.0f / scalar;
    return {x * inv, y * inv};
  }
  Vec2& operator+=(const Vec2& other) {
    x += other.x;
    y += other.y;
    return *this;
  }
  Vec2& operator-=(const Vec2& other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }
  float length() const { return std::sqrt(x * x + y * y); }
  float dot(const Vec2& other) const { return x * other.x + y * other.y; }

  float lengthSq() const { return x * x + y * y; }
  Vec2 normalized() const {
    float s = lengthSq();
    if (s > epsSq) {
      return *this / std::sqrt(s);
    }
    return {0, 0};
  }
  float normalize() {
    float s = lengthSq();
    if (s > epsSq) {
      s = sqrtf(s);
      float invLen = 1.0f / s;
      x *= invLen;
      y *= invLen;
      return s;
    }
    x = 0;
    y = 0;
    return 0.0f;
  }

  // float angle() const { return std::atan2(y, x) * RAD_TO_DEG; }
  //   Vec2 rotated(float rads) const {
  //     float c = std::cos(rads);
  //     float s = std::sin(rads);
  //     return Vec2(x * c - y * s, x * s + y * c);
  //   }
  //   Vec2 rotate(float rads) {
  //     float c = std::cos(rads);
  //     float s = std::sin(rads);
  //     float newX = x * c - y * s;
  //     y = x * s + y * c;
  //     x = newX;
  //     return *this;
  //   }

  Vec2 flipped() const { return Vec2(-x, -y); }
  Vec2 flip() {
    x = -x;
    y = -y;
    return *this;
  }

  bool equals(const Vec2& other, float tol = eps) const {
    return std::abs(x - other.x) < tol && std::abs(y - other.y) < tol;
  }
};

static inline Vec2 operator*(float scalar, const Vec2& v) { return v * scalar; }

struct Ivec2 {
  int x, y;
  Ivec2(int x = 0, int y = 0) : x(x), y(y) {}
  Ivec2 operator+(const Ivec2& other) const {
    return {x + other.x, y + other.y};
  }
  Ivec2 operator-(const Ivec2& other) const {
    return {x - other.x, y - other.y};
  }
  Ivec2& operator+=(const Ivec2& other) {
    x += other.x;
    y += other.y;
    return *this;
  }
  Ivec2& operator-=(const Ivec2& other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }
  Ivec2& operator=(const Ivec2& other) {
    x = other.x;
    y = other.y;
    return *this;
  }

  Ivec2(const Vec2& v) : x(static_cast<int>(v.x)), y(static_cast<int>(v.y)) {}
};
