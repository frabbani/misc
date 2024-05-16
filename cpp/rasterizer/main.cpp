#include <cstdio>
#include <cstdlib>
#include <string.h>

#include "defs.h"
#include "file.h"
#include "rasterizer.h"
#include "texture.h"

using namespace rasterizer;

int main(int argc, char *args[]) {
  printf("Hello world!\n");
  Canvas canvas(640, 480, { VariableType::ColorType, VariableType::TexelType });
  auto &layer = std::get<std::vector<ColorVariable> >(canvas.layers[0]);
  auto &layer2 = std::get<std::vector<TexelVariable> >(canvas.layers[1]);
  printf("Canvas #. of fragments per layer: %zu\n", layer.size());

  Scanner scanner(canvas);
  Point pts[3];

  auto getTextureHandle = [](std::string_view file) {
    int handle;
    loadTexture(BMP::imageFromBitmapData(FileData(file).data, file), file, handle);
    return handle;
  };

  int handle = getTextureHandle("brick.bmp");
  printf("texture handle: %d\n", handle);

  pts[0] = canvas.createPoint();
  pts[0].p = Vector2(317.5f, 430.7f);
  std::get<ColorVariable>(pts[0].plot[0]).v = Color(255, 0, 255);
  std::get<TexelVariable>(pts[0].plot[1]).set(Vector2(0.5f, 0.9f), handle);
  pts[1] = canvas.createPoint();
  pts[1].p = Vector2(80.1f, 105.6f);
  std::get<ColorVariable>(pts[1].plot[0]).v = Color(0, 255, 255);
  std::get<TexelVariable>(pts[1].plot[1]).set(Vector2(0.25f, 0.3f), handle);

  pts[2] = canvas.createPoint();
  pts[2].p = Vector2(602.25f, 50.2f);
  std::get<ColorVariable>(pts[2].plot[0]).v = Color(255, 255, 0);
  std::get<TexelVariable>(pts[2].plot[1]).set(Vector2(0.75f, 0.26f), handle);

  scanner.buildEdge(pts[0], pts[1]);
  scanner.buildEdge(pts[0], pts[2]);
  scanner.buildEdge(pts[1], pts[2]);

  scanner.scan();
  {
    auto &layer = std::get<std::vector<ColorVariable> >(canvas.layers[0]);
    std::vector<Color> buffer;
    for (auto color : layer) {
      buffer.push_back(color.v);
    }
    //writePPM(buffer.data(), canvas.w, canvas.h, "color");
    BMP::Image image = BMP::Image { .w = int(canvas.w), .h = int(canvas.h), .pixels = std::move(buffer) };
    writeImageToBitmapFile(image, "color");
  }
  {
    auto &layer = std::get<std::vector<TexelVariable> >(canvas.layers[1]);
    std::vector<Color> buffer;
    for (auto texel : layer) {
      buffer.push_back(texel.v.sample());
    }

    BMP::Image image = BMP::Image { .w = int(canvas.w), .h = int(canvas.h), .pixels = std::move(buffer) };
    //writePPM(buffer.data(), canvas.w, canvas.h, "textured");
    writeImageToBitmapFile(image, "textured");
  }

  printf("Goodbye!\n");
  return 0;
}
