#include <mysdl2.h>
#include <stdio.h>

#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <optional>
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
    bool active = false;
    std::vector<int> connections;
    bool hasStart = false;
    bool hasEnd = false;

    Node() = default;
    Node(int index, Vec2 pos) : i(index), p(pos), active(false) {
      hasStart = hasEnd = false;
      connections.clear();
    }
    // void getConnections() {
    //   connections.clear();
    //   for (size_t i = 0; i < connected.size(); ++i) {
    //     if (connected[i]) connections.push_back(static_cast<int>(i));
    //   }
    // }

    int a() { return hasStart ? 0 : 1; }
    int b() {
      return hasEnd ? int(connections.size()) - 1 : int(connections.size()) - 2;
    }

    const Vec2& pos() const { return p; }
  };

  std::vector<Node> nodes;
  int startIdx = -1;
  int endIdx = -1;

  Map(int width = 0, int height = 0) : w(width), h(height) {
    if (w > 0 && h > 0) data.resize(w * h);
  }

  int selectClosest(int x, int y) {
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
    return j;
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

  // void connect(int a, int b) {
  //   if (a >= 0 && a < nodes.size() && b >= 0 && b < nodes.size()) {
  //     nodes[a].connected[b] = true;
  //     nodes[b].connected[a] = true;
  //   }
  // }
  // void disconnect(int a, int b) {
  //   if (a >= 0 && a < nodes.size() && b >= 0 && b < nodes.size()) {
  //     nodes[a].connected[b] = false;
  //     nodes[b].connected[a] = false;
  //   }
  // }

  void tryConnect(int a, int b) {
    int n = int(nodes.size());
    if (a >= 0 && a < n && b >= 0 && b < n) {
      if (!traceHit(nodes[a].p, nodes[b].p)) {
        nodes[a].connections.push_back(b);
        nodes[b].connections.push_back(a);
      }
    }
  }

  Node& start() { return nodes[startIdx]; }
  Node& end() { return nodes[endIdx]; }

  const Node& start() const { return nodes[startIdx]; }
  const Node& end() const { return nodes[endIdx]; }

  void buildNodes() {
    nodes.clear();
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
          nodes.back().active = true;
        } else if (!blocked(2, 0)) {
          // bottom right unblocked, extrude bottom right
          nodes.push_back(Node(nodes.size(), Vec2(x, y) + Vec2(+2, -2)));
          nodes.back().active = true;
        } else if (!blocked(0, 2)) {
          // top left unblocked, extrude top left
          nodes.push_back(Node(nodes.size(), Vec2(x, y) + Vec2(-2, +2)));
          nodes.back().active = true;
        } else if (!blocked(2, 2)) {
          // top right unblocked, extrude top right
          nodes.push_back(Node(nodes.size(), Vec2(x, y) + Vec2(+2, +2)));
          nodes.back().active = true;
        }
      } else if (nUnblocked == 5) {
        if (blocked(0, 0) && blocked(1, 0) && blocked(0, 1) && blocked(1, 1)) {
          nodes.push_back(Node(nodes.size(), Vec2(x, y) + Vec2(+2, +2)));
          nodes.back().active = true;
        }
        if (blocked(1, 0) && blocked(2, 0) && blocked(1, 1) && blocked(2, 1)) {
          nodes.push_back(Node(nodes.size(), Vec2(x, y) + Vec2(-2, +2)));
          nodes.back().active = true;
        }
        if (blocked(0, 1) && blocked(1, 1) && blocked(0, 2) && blocked(1, 2)) {
          nodes.push_back(Node(nodes.size(), Vec2(x, y) + Vec2(+2, -2)));
          nodes.back().active = true;
        }
        if (blocked(1, 1) && blocked(2, 1) && blocked(1, 2) && blocked(2, 2)) {
          nodes.push_back(Node(nodes.size(), Vec2(x, y) + Vec2(-2, -2)));
          nodes.back().active = true;
        }
      }
    };

    nodes.push_back(Node());
    for (int y = 0; y < h; ++y) {
      for (int x = 0; x < w; ++x) {
        add_node(x, y);
      }
    }
    nodes.push_back(Node());
    startIdx = 0;
    endIdx = nodes.size() - 1;

    for (size_t i = 0; i < nodes.size(); i++) {
      nodes[i].connections.push_back(startIdx);
    }
    for (size_t i = 1; i < nodes.size(); ++i) {
      for (size_t j = i + 1; j < nodes.size(); ++j) {
        tryConnect(i, j);
      }
    }
    for (size_t i = 0; i < nodes.size(); i++) {
      nodes[i].connections.push_back(endIdx);
    }
  }

  void setStart(int x, int y) {
    clearStart();
    if (!walkable(x, y)) return;
    start().active = true;
    start().p = Vec2(x, y);
    start().connections.clear();
    for (size_t i = 1; i < nodes.size(); ++i) {
      if (!traceHit(start().p, nodes[i].p)) {
        start().connections.push_back(i);
        nodes[i].hasStart = true;
      } else {
        nodes[i].hasStart = false;
      }
    }
  }

  void setEnd(int x, int y) {
    clearEnd();
    if (!walkable(x, y)) return;
    end().active = true;
    end().p = Vec2(x, y);
    for (size_t i = 0; i < nodes.size() - 1; ++i) {
      if (!traceHit(end().p, nodes[i].p)) {
        end().connections.push_back(i);
        nodes[i].hasEnd = true;
      } else {
        nodes[i].hasEnd = false;
      }
    }
  }

  void clearStart() {
    start().active = false;
    for (size_t i = 1; i < nodes.size(); ++i) {
      nodes[i].hasStart = false;
    }
    start().connections.clear();
  }
  void clearEnd() {
    end().active = false;
    for (size_t i = 0; i < nodes.size() - 1; ++i) {
      nodes[i].hasEnd = false;
    }
    end().connections.clear();
  }

  void addStartingLines(std::vector<std::pair<Vec2, Vec2>>& lines) {
    if (!start().active) return;
    for (size_t i = 0; i < start().connections.size(); ++i) {
      int idx = start().connections[i];
      lines.push_back({start().p, nodes[idx].p});
    }
  }

  void addEndingLines(std::vector<std::pair<Vec2, Vec2>>& lines) {
    if (!end().active) return;
    for (size_t i = 0; i < end().connections.size(); ++i) {
      int idx = end().connections[i];
      lines.push_back({end().p, nodes[idx].p});
    }
  }

  std::vector<int> astar(int start, int end) {
    const int n = static_cast<int>(nodes.size());

    if (start < 0 || start >= n || end < 0 || end >= n) return {};

    if (!nodes[start].active || !nodes[end].active) return {};

    auto dist = [&](int a, int b) -> float {
      return std::sqrt((nodes[a].p - nodes[b].p).lengthSq());
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
    open.push({dist(start, end), start});

    while (!open.empty()) {
      int current = open.top().index;
      open.pop();

      if (closed[current]) continue;

      if (current == end) {
        std::vector<int> path;

        for (int at = end; at != -1; at = parent[at]) path.push_back(at);

        std::reverse(path.begin(), path.end());
        return path;
      }

      closed[current] = true;

      for (size_t i = nodes[current].a(); i <= nodes[current].b(); ++i) {
        int neighbor = nodes[current].connections[i];
        if (!nodes[neighbor].active || closed[neighbor]) continue;

        float tentativeG = gScore[current] + dist(current, neighbor);

        if (tentativeG < gScore[neighbor]) {
          parent[neighbor] = current;
          gScore[neighbor] = tentativeG;

          float f = tentativeG + dist(neighbor, end);
          open.push({f, neighbor});
        }
      }

      // for (int neighbor = 0; neighbor < n; ++neighbor) {
      //   if (!nodes[current].connected[neighbor]) continue;

      //   if (!nodes[neighbor].active || closed[neighbor]) continue;

      //   float tentativeG = gScore[current] + dist(current, neighbor);

      //   if (tentativeG < gScore[neighbor]) {
      //     parent[neighbor] = current;
      //     gScore[neighbor] = tentativeG;

      //     float f = tentativeG + dist(neighbor, end);
      //     open.push({f, neighbor});
      //   }
      // }
    }

    return {};  // No path found
  }

  void setPathLines(std::vector<std::pair<Vec2, Vec2>>& lines) {
    lines.clear();
    auto path = astar(0, int(nodes.size()) - 1);
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
    map->setStart(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
  }
  if (sdl.mouseKeyDown(2)) {
    map->setEnd(static_cast<int>(mousePos.x), static_cast<int>(mousePos.y));
  }
  lines.clear();
  // map->addStartingLines(lines);
  // map->addEndingLines(lines);
  map->setPathLines(lines);
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
