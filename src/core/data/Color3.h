#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace mdEngine
{
  struct Color3
  {
    uint8_t r, g, b;

    constexpr Color3() : r(255), g(255), b(255) {}
    constexpr Color3(const uint8_t r, const uint8_t g, const uint8_t b) : r(r), g(g), b(b) {}

    explicit constexpr Color3(const std::string& hex)
    {
      if (hex.size() != 7 || hex[0] != '#') {
        throw std::invalid_argument("invalid hex color format");
      }

      r = static_cast<uint8_t>(std::stoi(hex.substr(1, 2), nullptr, 16));
      g = static_cast<uint8_t>(std::stoi(hex.substr(3, 2), nullptr, 16));
      b = static_cast<uint8_t>(std::stoi(hex.substr(5, 2), nullptr, 16));
    }
  };

  namespace colors
  {
    constexpr Color3 BLACK = {0, 0, 0};
    constexpr Color3 WHITE = {255, 255, 255};
  } // colors
} // mdEngine
