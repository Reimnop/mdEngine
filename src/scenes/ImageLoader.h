#pragma once

#include <filesystem>
#include <vector>

struct ImageData
{
  int width = 0, height = 0;
  std::vector<unsigned char> pixels;
};

ImageData loadImage(const std::filesystem::path& path);
