#include "rasterizer.h"

#include <cmath>

namespace rasterizer {

template<class T> struct Vary_ {
  static_assert( IsContainedIn<T, Variable>::value, "nope...");

  const T begin;
  const T end;
  Vary_(const Variable &begin, const Variable &end)
      :
      begin(std::get<T>(begin)),
      end(std::get<T>(end)) {
  }

  void operate(T &out, float alpha) {
    out.v = begin.vary(end.v, alpha);
  }

};

struct Vary {
  const Variable begin;
  const Variable end;
  float alpha;
  Vary(const Variable begin, const Variable end, float alpha)
      :
      begin(std::move(begin)),
      end(std::move(end)),
      alpha(alpha) {
  }
  void operator()(ScalarVariable &out) {
    Vary_<ScalarVariable> v(begin, end);
    v.operate(out, alpha);
  }
  void operator()(Vector2Variable &out) {
    Vary_<Vector2Variable> v(begin, end);
    v.operate(out, alpha);
  }
  void operator()(Vector3Variable &out) {
    Vary_<Vector3Variable> v(begin, end);
    v.operate(out, alpha);
  }
  void operator()(ColorVariable &out) {
    Vary_<ColorVariable> v(begin, end);
    v.operate(out, alpha);
  }
  void operator()(TexelVariable &out) {
    Vary_<TexelVariable> v(begin, end);
    v.operate(out, alpha);
  }

};

Canvas::Canvas(uint32 w, uint32 h, const std::vector<VariableType> &types) {
  this->w = w;
  this->h = h;
  for (auto type : types) {
    if (layers.size() == maxLayers)
      break;
    switch (type) {
      case ScalarType: {
        auto layer = std::vector<ScalarVariable>(w * h);
        layers.push_back(std::move(layer));
        break;
      }
      case Vector2Type: {
        auto layer = std::vector<Vector2Variable>(w * h);
        layers.push_back(std::move(layer));
        break;
      }
      case Vector3Type: {
        auto layer = std::vector<Vector3Variable>(w * h);
        layers.push_back(std::move(layer));
        break;
      }
      case ColorType: {
        auto layer = std::vector<ColorVariable>(w * h);
        layers.push_back(std::move(layer));
        break;
      }
      case TexelType: {
        auto layer = std::vector<TexelVariable>(w * h);
        layers.push_back(std::move(layer));
        break;
      }
    }
  }
}

Point Canvas::createPoint(int32 x, int32 y) {
  Point pt;
  struct V {
    Variable &var;
    size_t index = 0;
    V(Variable &v, size_t i)
        :
        var(v),
        index(i) {
    }
    void operator()(const std::vector<ScalarVariable> &from) {
      var = from[index];
    }
    void operator()(const std::vector<Vector2Variable> &from) {
      var = from[index];
    }
    void operator()(const std::vector<Vector3Variable> &from) {
      var = from[index];
    }
    void operator()(const std::vector<ColorVariable> &from) {
      var = from[index];
    }
    void operator()(const std::vector<TexelVariable> &from) {
      var = from[index];
    }
  };
  pt.plot = std::vector<Variable>(layers.size());
  size_t i = 0;
  size_t j = xy(x, y);
  for (const auto &layer : layers) {
    V visitor(pt.plot[i++], j);
    std::visit(visitor, layer);
  }
  return pt;
}

void Canvas::plotPoint(const Point &pt, int32 x, int32 y) {

  struct V {
    const Variable &var;
    size_t index;
    V(const Variable &v, size_t i)
        :
        var(v),
        index(i) {
    }
    void operator()(std::vector<ScalarVariable> &to) {
      to[index] = std::get<ScalarVariable>(var);
    }
    void operator()(std::vector<Vector2Variable> &to) {
      to[index] = std::get<Vector2Variable>(var);
    }
    void operator()(std::vector<Vector3Variable> &to) {
      to[index] = std::get<Vector3Variable>(var);
    }
    void operator()(std::vector<ColorVariable> &to) {
      to[index] = std::get<ColorVariable>(var);
    }
    void operator()(std::vector<TexelVariable> &to) {
      to[index] = std::get<TexelVariable>(var);
    }
  };

  size_t index = xy(x, y);
  size_t i = 0;
  for (auto &layer : layers) {
    V visitor(pt.plot[i++], index);
    std::visit(visitor, layer);
  }
}

Point blendPoints(const Point &begin, const Point &end, float alpha) {
  Point pt;
  pt.p = begin.p + alpha * (end.p - begin.p);
  if (begin.plot.size() != end.plot.size())
    return pt;
  //do i need to check variants hold same datatype?
  for (size_t i = 0; i < begin.plot.size(); i++) {
    auto beginVar = begin.plot[i];
    auto endVar = end.plot[i];
    Variable result = beginVar;
    Vary vary(beginVar, endVar, alpha);
    std::visit(vary, result);
    pt.plot.push_back(std::move(result));
  }
  return pt;
}

void Scanner::Pair::push(int y, const Point &pt) {
  if (!this->y.has_value()) {
    this->y = y;
    pt0 = pt1 = pt;
  } else {
    if (pt.p.x < pt0.p.x)
      pt0 = pt;
    if (pt.p.x > pt1.p.x)
      pt1 = pt;
  }
}

void Scanner::pushPoint(const Point &pt) {
  int yBottom = int(floorf(pt.p.y));
  int yTop = int(ceilf(pt.p.y));
  yBottom = std::clamp(yBottom, 0, int(canvas.h - 1));
  yTop = std::clamp(yTop, 0, int(canvas.h - 1));
  if (yBottom < this->yBottom)
    this->yBottom = yBottom;
  if (yTop > this->yTop)
    this->yTop = yTop;
  for (int y = yBottom; y <= yTop; y++) {
    pairs[y].push(y, pt);
  }
}

void Scanner::plotPoint(const Point &pt, int x, int y) {

}

void Scanner::buildEdge(const Point &pt0, const Point &pt1) {
  const Point *bottom = &pt0;
  const Point *top = &pt1;
  if (bottom->p.y > top->p.y) {
    SWAP(top, bottom);
  }

  int yBottom = int(floorf(bottom->p.y));
  int yTop = int(ceilf(top->p.y));
  for (int y = yBottom; y <= yTop; y++) {
    float alpha = yBottom == yTop ? 0.5f : float(y - yBottom) / float(yTop - yBottom);
    pushPoint(blendPoints(*bottom, *top, alpha));
  }
}

void Scanner::scanLine(const Point &pt0, const Point &pt1, int y) {
  const Point *left = &pt0;
  const Point *right = &pt1;
  if (left->p.x > right->p.x) {
    SWAP(left, right);
  }

  int xLeft = int(floorf(left->p.x));
  int xRight = int(ceilf(right->p.x));
  for (int x = xLeft; x <= xRight; x++) {
    float alpha = xLeft == xRight ? 0.5f : float(x - xLeft) / float(xRight - xLeft);
    canvas.plotPoint(blendPoints(*left, *right, alpha), x, y);
  }
}

void Scanner::scan() {
  for (int y = yBottom; y < yTop; y++) {
    scanLine(pairs[y].pt0, pairs[y].pt1, y);
    //canvas.plotPoint(pairs[y].pt0, int(floorf(pairs[y].pt0.p.x)), y);
    //canvas.plotPoint(pairs[y].pt1, int(ceilf(pairs[y].pt1.p.x)), y);
  }
}

}
