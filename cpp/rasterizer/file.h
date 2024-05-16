#pragma once

#include "defs.h"

#include <string>
#include <vector>

struct FileData {
  std::string name;
  std::vector<uint8> data;
  FileData() = default;
  FileData(std::string_view fileName);
  void save(std::string_view fileName);
};
