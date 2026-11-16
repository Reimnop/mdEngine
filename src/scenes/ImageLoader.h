#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

struct ImageData
{
  int width = 0, height = 0;
  std::vector<uint8_t> pixels; // RGBA8, first row is the bottom of the image
};

ImageData loadImage(const std::filesystem::path& path);
ImageData loadImageFromMemory(const uint8_t* bytes, size_t size);
