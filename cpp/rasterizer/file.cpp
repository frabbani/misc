#include "file.h"

#include <cstdio>

FileData::FileData(std::string_view fileName) {
  FILE *fp = fopen(fileName.data(), "rb");
  if (!fp) {
    printf("FileData: '%s' is not a valid file\n", fileName.data());
    return;
  }
  fseek(fp, 0, SEEK_END);
  auto size = ftell(fp);
  if (!size) {
    printf("FileData: '%s' is empty\n", fileName.data());
    fclose(fp);
    return;
  }
  name = fileName;
  data.reserve(size);
  fseek(fp, 0, SEEK_SET);
  while (!feof(fp)) {
    data.push_back(fgetc(fp));
  }
  fclose(fp);
}

void FileData::save(std::string_view fileName) {
  FILE *fp = fopen(fileName.data(), "rb");
  fwrite(data.data(), data.size(), 1, fp);
  fclose(fp);
}
