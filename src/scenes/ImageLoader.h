#pragma once

#include <filesystem>
#include <vector>

struct ImageData
{
  int width = 0, height = 0;
  std::vector<uint8_t> pixels;
};

ImageData loadImage(const std::filesystem::path& path);
