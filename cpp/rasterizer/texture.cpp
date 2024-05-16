#include "texture.h"

#include <cstdlib>
#include <cstdio>
#include <map>
#include <algorithm>
#include <memory>

void writePPM(const Color *pixels, uint32 w, uint32 h, const char name[]) {
  char file[128];
  sprintf(file, "%s.ppm", name);
  FILE *fp = fopen(file, "wb");
  fprintf(fp, "P6\n");
  fprintf(fp, "%d %d 255\n", w, h);
  fwrite(pixels, 3, w * h, fp);
  fclose(fp);
}

static std::map<int, std::shared_ptr<BMP::Image>> images;
static std::map<int, std::string> imageTags;

bool loadTexture(const BMP::Image &image, std::string_view tag, int &handle) {
  static int uniqueId = 0;
  handle = -1;
  if (!image.isValid()) {
    printf("%s:error - invalid image\n", __FUNCTION__);
    return false;
  }
  auto newImage = std::make_shared<BMP::Image>();
  newImage->w = image.w;
  newImage->h = image.h;
  newImage->pixels = image.pixels;
  handle = ++uniqueId;
  images[handle] = std::move(newImage);
  imageTags[handle] = tag;
  return true;
}

Color sampleTexture(int handle, Vector2 uv) {
  const auto it = images.find(handle);
  if (it == images.end())
    return Color();

  auto image = it->second;

  float xScale = float(image->w) - 1.0f;
  float yScale = float(image->h) - 1.0f;

  float xOffset = 0.5f / float(image->w);
  float yOffset = 0.5f / float(image->h);

  uv.x = (uv.x - xOffset) * xScale;
  uv.y = (uv.y - yOffset) * yScale;
  return image->sample(uv.x, uv.y, true);
}

Color Texel::sample() const {
  return sampleTexture(textureHandle, uv);
}
