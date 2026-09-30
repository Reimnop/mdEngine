#pragma once

#include <filesystem>
#include <string>
#include <fstream>

namespace mdEngine::io
{
  inline std::string readFileAsString(const std::filesystem::path& path)
  {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
      throw std::runtime_error("failed to open file");
    }
    return {std::istreambuf_iterator(file), std::istreambuf_iterator<char>()};
  }
} // mdEngine::io