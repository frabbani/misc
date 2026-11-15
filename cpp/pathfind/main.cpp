#include <mysdl2.h>
#include <stdio.h>

#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

#define DISP_W 512
#define DISP_H 512

using namespace sdl2;

#include "vec.h"

SDL sdl;
Vec2 mousePos;

std::map<std::string, std::unique_ptr<sdl2::Font>> fonts;
std::vector<std::pair<Vec2, Vec2>> lines;

struct Map {
  std::vector<Uint8> data;
  int w, h;

  struct Node {
    int i;
    Vec2 p;
    std::vector<int> cons;
    Node() = default;
    Node(int index, Vec2 pos) : i(index), p(pos) { cons.clear(); }
    const Vec2& pos() const { return p; }
  };

  std::vector<Node> nodes;
  std::vector<bool> selected;
  bool hasEntry = false;
  bool hasExit = false;

  Map(int width = 0, int height = 0) : w(width), h(height) {
    if (w > 0 && h > 0) data.resize(w * h);
  }

  void select(int x, int y) {
    if (selected.size() != nodes.size()) selected.resize(nodes.size(), false);
    Vec2 c(x, y);
    int j = 0;
    float lensq = (c - nodes[0].p).lengthSq();
    for (size_t i = 1; i < nodes.size(); ++i) {
      float rsq = (c - nodes[i].p).lengthSq();
      if (rsq < lensq) {
        lensq = rsq;
        j = i;
      }
    }
    for (size_t i = 0; i < selected.size(); ++i) {
      selected[i] = false;
    }
    selected[j] = true;
  }

  void load(const Pixels& pixels) {
    w = pixels.w;
    h = pixels.h;
    data.resize(w * h);
    for (int y = 0; y < h; ++y) {
      for (int x = 0; x < w; ++x) {
        Uint8 v = 255;
        if (pixels.bpp == 32) {
          auto p = pixels.get32(x, y);
          v = p ? p->r : 255;
        } else if (pixels.bpp == 24) {
          auto p = pixels.get24(x, y);
          v = p ? p->r : 255;
        }
        data[y * w + x] = v;
      }
    }
    buildNodes();
  }

  bool walkable(int x, int y) const { return at(x, y) > 4; }

  Uint8 at(int x, int y) {
    x = x < 0 ? 0 : x >= w ? w - 1 : x;
    y = y < 0 ? 0 : y >= h ? h - 1 : y;
    return data[y * w + x];
  }
  const Uint8& at(int x, int y) const {
    x = x < 0 ? 0 : x >= w ? w - 1 : x;
    y = y < 0 ? 0 : y >= h ? h - 1 : y;
    return data[y * w + x];
  }

  bool inBounds(int x, int y) const {
    return x >= 0 && x < w && y >= 0 && y < h;
  }

  float traceLine(int x0, int y0, int x1, int y1) const {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    if (dx == 0 && dy == 0) {
      return 1.0f;
    }

    int ds = dy > dx ? dy : dx;
    float sx = (x1 - x0) / static_cast<float>(ds);
    float sy = (y1 - y0) / static_cast<float>(ds);
    int x_fix = x0 << 16;
    int y_fix = y0 << 16;
    int dx_fix = int(sx * 65536.0f);
    int dy_fix = int(sy * 65536.0f);
    float t = 0.0f;
    float dt = 1.0f / static_cast<float>(ds);
    for (int i = 0; i <= ds; i++, t += dt) {
      int x = x_fix >> 16;
      int y = y_fix >> 16;
      if (!walkable(x, y)) {
        return t;
      }
      x_fix += dx_fix;
      y_fix += dy_fix;
    }
    return 1.0f;
  };

  inline float traceLine(const Ivec2& a, const Ivec2& b) const {
    return traceLine(a.x, a.y, b.x, b.y);
  }

  inline float traceLine(const Vec2& a, const Vec2& b) const {
    return traceLine(static_cast<int>(a.x), static_cast<int>(a.y),
                     static_cast<int>(b.x), static_cast<int>(b.y));
  }

  bool traceHit(int x0, int y0, int x1, int y1) const {
    if (!inBounds(x0, y0) || !inBounds(x1, y1) || !walkable(x0, y0) ||
        !walkable(x1, y1))
      return true;
    return traceLine(x0, y0, x1, y1) < (1.0f - eps);
  }

  inline bool traceHit(const Ivec2& a, const Ivec2& b) const {
    return traceHit(a.x, a.y, b.x, b.y);
  }

  inline bool traceHit(const Vec2& a, const Vec2& b) const {
    return traceHit(static_cast<int>(a.x), static_cast<int>(a.y),
                    static_cast<int>(b.x), static_cast<int>(b.y));
  }

  Node& entry() { return nodes.front(); }
  Node& exit() { return nodes.back(); }

  void insert(int index) {
    int n = int(nodes.size());
    if (index < 0 || index >= n) return;
    auto& node = nodes[index];
    node.cons.clear();
    for (auto& other : nodes) {
      other.cons.erase(std::remove(other.cons.begin(), other.cons.end(), index),
                       other.cons.end());
    }
    if ((index == 0 && !hasEntry) || (index == n - 1 && !hasExit)) return;
    for (int i = 0; i < n; ++i) {
      if (i == index) continue;
      if ((i == 0 && !hasEntry) || (i == n - 1 && !hasExit)) continue;
      if (!traceHit(node.p, nodes[i].p) && !traceHit(nodes[i].p, node.p)) {
        node.cons.push_back(i);
        nodes[i].cons.push_back(index);
      }
    }
  }

  void buildNodes() {
    nodes.clear();
    selected.clear();
    hasEntry = false;
    hasExit = false;
    Uint8 blocks[3][3];
    auto blocked = [&](int x, int y) { return blocks[y][x] > 0; };

    auto add_node = [&](int x, int y) {
      int nUnblocked = 0;
      for (int j = -1; j <= 1; ++j) {
        for (int i = -1; i <= 1; ++i) {
          blocks[j + 1][i + 1] = at(x + i, y + j) > 4 ? 0 : 1;
          if (!blocked(i + 1, j + 1)) ++nUnblocked;
        }
      }
      if (nUnblocked == 1) {
        if (!blocked(0, 0)) {
          // bottom left unblocked, extrudde bottm left
          nodes.push_back(Node(nodes.size(), Vec2(x, y) + Vec2(-2, -2)));
        } else if (!blocked(2, 0)) {
          // bottom right unblocked, extrude bottom right
          nodes.push_back(Node(nodes.size(), Vec2(x, y) + Vec2(+2, -2)));
        } else if (!blocked(0, 2)) {
          // top left unblocked, extrude top left
          nodes.push_back(Node(nodes.size(), Vec2(x, y) + Vec2(-2, +2)));
        } else if (!blocked(2, 2)) {
          // top right unblocked, extrude top right
          nodes.push_back(Node(nodes.size(), Vec2(x, y) + Vec2(+2, +2)));
        }
      } else if (nUnblocked == 5) {
        if (blocked(0, 0) && blocked(1, 0) && blocked(0, 1) && blocked(1, 1)) {
          nodes.push_back(Node(nodes.size(), Vec2(x, y) + Vec2(+2, +2)));
        }
        if (blocked(1, 0) && blocked(2, 0) && blocked(1, 1) && blocked(2, 1)) {
          nodes.push_back(Node(nodes.size(), Vec2(x, y) + Vec2(-2, +2)));
        }
        if (blocked(0, 1) && blocked(1, 1) && blocked(0, 2) && blocked(1, 2)) {
          nodes.push_back(Node(nodes.size(), Vec2(x, y) + Vec2(+2, -2)));
        }
        if (blocked(1, 1) && blocked(2, 1) && blocked(1, 2) && blocked(2, 2)) {
          nodes.push_back(Node(nodes.size(), Vec2(x, y) + Vec2(-2, -2)));
        }
      }
    };

    // entry node for dynamic insertion
    nodes.push_back(Node());

    for (int y = 0; y < h; ++y) {
      for (int x = 0; x < w; ++x) {
        add_node(x, y);
      }
    }
    // exit node for dynamic insertion
    nodes.push_back(Node());

    // clear selected
    for (size_t i = 0; i < nodes.size(); ++i) {
      selected.push_back(false);
    }

    for (size_t i = 1; i < nodes.size() - 1; ++i) {
      insert(i);
    }
  }

  void setEntry(int x, int y) {
    entry().p = Vec2(x, y);
    hasEntry = true;
    insert(0);
    selected.front() = true;
  }

  void setExit(int x, int y) {
    exit().p = Vec2(x, y);
    hasExit = true;
    insert(nodes.size() - 1);
    selected.back() = true;
  }

  void setLines(int index, std::vector<std::pair<Vec2, Vec2>>& lines) {
    if (index < 0 || index >= (int)nodes.size()) return;
    auto& node = nodes[index];
    for (size_t i = 0; i < node.cons.size(); ++i) {
      lines.push_back({node.p, nodes[node.cons[i]].p});
    }
  }

  std::vector<int> astar(int start, int goal) {
    const int n = static_cast<int>(nodes.size());

    if (start < 0 || start >= n || goal < 0 || goal >= n) return {};
    auto dist = [&](int a, int b) {
      const Vec2 d = nodes[a].p - nodes[b].p;
      return std::sqrt(d.x * d.x + d.y * d.y);
    };

    struct Entry {
      float f;
      int index;
      bool operator>(const Entry& other) const { return f > other.f; }
    };

    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> open;

    std::vector<float> gScore(n, std::numeric_limits<float>::infinity());

    std::vector<int> parent(n, -1);
    std::vector<bool> closed(n, false);

    gScore[start] = 0.0f;
    open.push({dist(start, goal), start});

    while (!open.empty()) {
      const int current = open.top().index;
      open.pop();

      if (closed[current]) continue;

      if (current == goal) {
        std::vector<int> path;

        for (int at = goal; at != -1; at = parent[at]) path.push_back(at);

        std::reverse(path.begin(), path.end());
        return path;
      }

      closed[current] = true;

      for (int neighbor : nodes[current].cons) {
        if (neighbor < 0 || neighbor >= n) continue;

        if (closed[neighbor]) continue;

        float tentativeG = gScore[current] + dist(current, neighbor);

        if (tentativeG < gScore[neighbor]) {
          parent[neighbor] = current;
          gScore[neighbor] = tentativeG;

          float f = tentativeG + dist(neighbor, goal);

          open.push({f, neighbor});
        }
      }
    }

    return {};  // No path found
  }

  void createPathLines(std::vector<std::pair<Vec2, Vec2>>& lines) {
    lines.clear();
    if (!hasEntry || !hasExit || entry().cons.empty() || exit().cons.empty())
      return;
    // for (int i = 1; i < static_cast<int>(nodes.size()); ++i) {
    //   lines.push_back({nodes[i - 1].p, nodes[i].p});
    // }
    auto path = astar(0, static_cast<int>(nodes.size()) - 1);
    for (size_t i = 1; i < path.size(); ++i) {
      lines.push_back({nodes[path[i - 1]].p, nodes[path[i]].p});
    }
  }

  void draw(Pixels& bg) {
    Pixel24 walkCol = Pixel24("#ffffff");
    Pixel24 blockedCol = Pixel24("#020202");
    if (!bg.hasData() || w <= 0 || h <= 0) return;
    for (int y = 0; y < bg.h; ++y) {
      for (int x = 0; x < bg.w; ++x) {
        Pixel24 col = walkable(x, y) ? walkCol : blockedCol;
        bg.plot(x, y, col);
      }
    }
    for (size_t i = 0; i < nodes.size(); i++) {
      Pixel24 col(255, 0, 0);
      if (selected.size() == nodes.size()) {
        if (selected[i]) col = Pixel24(0, 255, 0);
      }
      Pixels::Coord c = {static_cast<int>(nodes[i].p.x),
                         static_cast<int>(nodes[i].p.y)};
      bg.drawBox(c, 3, 3, col);
    }
  }
};

std::unique_ptr<Map> map;

void text(Pixels& bg, std::string_view fontName, std::string_view text, int x,
          int y, Pixel24 color) {
  if (!bg.hasData()) {
    return;
  }
  auto it = fonts.find(std::string(fontName));
  if (it != fonts.end() && it->second) {
    auto& font = *it->second;
    font.render(bg, x, y, text, color);
  }
}

void init() {
  printf("*** INIT ***\n");
  fonts["default"] = std::make_unique<sdl2::Font>("Armwarmer.otf", 20);
  Bitmap bmp("map.bmp");
  bmp.lock();
  map = std::make_unique<Map>();
  map->load(bmp.pixels);
  printf("************\n");
}

void step() {
  mousePos = Vec2(static_cast<float>(sdl.mouseX),
                  static_cast<float>(DISP_H - 1 - sdl.mouseY));

  if (sdl.mouseKeyDown(0)) {
    map->setEntry(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
  }
  if (sdl.mouseKeyDown(2)) {
    map->setExit(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
  }
  map->createPathLines(lines);
  // map->setLines(0, lines);
  // map->setLines(map->nodes.size() - 1, lines);

  // auto& e = map->entry();
  // for (size_t i = 0; i < e.cons.size(); ++i) {
  //   lines.push_back({e.p, map->nodes[e.cons[i]].p});
  // }
  // for (int i = 0; i < int(map->nodes.size()); ++i) {
  //   if (map->selected[i]) {
  //     auto& node = map->nodes[i];
  //     for (int j = 0; j < (int)node.cons.size(); ++j) {
  //       lines.push_back({node.p, map->nodes[node.cons[j]].p});
  //     }
  //   }
  // }
}

void info(Pixels& bg) {
  std::stringstream ss;
  ss << "M {" << static_cast<int>(mousePos.x) << ", "
     << static_cast<int>(mousePos.y) << "}";
  text(bg, "default", ss.str(), 350, 8, Pixel24("#4e1ff5"));
}

void draw() {
  auto bg = sdl.lock();
  bg.inverted = true;
  // bg.clear(Pixel24("#11072b"));
  map->draw(bg);
  for (auto& line : lines) {
    Pixels::Coord c1, c2;
    c1.x = int(line.first.x);
    c1.y = int(line.first.y);
    c2.x = int(line.second.x);
    c2.y = int(line.second.y);
    bg.drawLine(c1, c2, 0, 0, 255, 255, false);
  }
  info(bg);
}

void term() {
  printf("*** TERM ***\n");
  map.reset();
  fonts.clear();
  printf("************\n");
}

int main(int argc, char* args[]) {
  setbuf(stdout, NULL);
  if (!sdl.init(DISP_W, DISP_H, false)) return 0;

  setbuf(stdout, NULL);

  double drawTimeInSecs = 0.0;
  Uint32 drawCount = 0.0;
  double invFreq = 1.0 / sdl.getPerfFreq();

  auto start = sdl.getTicks();
  auto last = start;
  init();
  while (true) {
    if (sdl.keyDown(SDLK_ESCAPE)) break;

    auto now = sdl.getTicks();
    auto elapsed = now - last;
    if (elapsed >= 20) {
      sdl.pump();
      step();
      last = (now / 20) * 20;
    }
    Uint64 beginCount = sdl.getPerfCounter();
    draw();
    drawCount++;
    sdl.swap();
    drawTimeInSecs += (double)(sdl.getPerfCounter() - beginCount) * invFreq;
  }
  term();
  if (drawCount)
    printf("Average draw time: %f ms\n",
           (float)((drawTimeInSecs * 1e3) / (double)drawCount));

  sdl.term();
  printf("goodbye!\n");
  return 0;
}
