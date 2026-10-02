#pragma once
#include <cmath>

namespace mdEngine
{
  namespace ch
  {
    constexpr float toLinear(const float value)
    {
      return std::pow(value, 2.2f);
    }

    constexpr float toGamma(const float value)
    {
      return std::pow(value, 1.0f / 2.2f);
    }
  } // ch

  struct Color3f
  {
    float r, g, b;

    explicit constexpr Color3f(const float red, const float green, const float blue)
        : r(red), g(green), b(blue)
    {
    }

    explicit constexpr Color3f(const float gray)
        : r(gray), g(gray), b(gray)
    {
    }

    constexpr Color3f(): r(0.0f), g(0.0f), b(0.0f)
    {
    }

    Color3f operator+(const Color3f& other) const
    {
      return Color3f(r + other.r, g + other.g, b + other.b);
    }

    Color3f operator-(const Color3f& other) const
    {
      return Color3f(r - other.r, g - other.g, b - other.b);
    }

    Color3f operator*(float scalar) const
    {
      return Color3f(r * scalar, g * scalar, b * scalar);
    }

    Color3f operator/(float scalar) const
    {
      return Color3f(r / scalar, g / scalar, b / scalar);
    }

    Color3f& operator+=(const Color3f& other)
    {
      r += other.r;
      g += other.g;
      b += other.b;
      return *this;
    }

    Color3f& operator-=(const Color3f& other)
    {
      r -= other.r;
      g -= other.g;
      b -= other.b;
      return *this;
    }

    Color3f& operator*=(float scalar)
    {
      r *= scalar;
      g *= scalar;
      b *= scalar;
      return *this;
    }

    Color3f& operator/=(float scalar)
    {
      r /= scalar;
      g /= scalar;
      b /= scalar;
      return *this;
    }

    static constexpr Color3f toLinear(const Color3f& color)
    {
      return Color3f(
        ch::toLinear(color.r),
        ch::toLinear(color.g),
        ch::toLinear(color.b));
    }

    static constexpr Color3f toGamma(const Color3f& color)
    {
      return Color3f(
        ch::toGamma(color.r),
        ch::toGamma(color.g),
        ch::toGamma(color.b));
    }

    [[nodiscard]] Color3f toLinear() const
    {
      return Color3f(
        ch::toLinear(r),
        ch::toLinear(g),
        ch::toLinear(b));
    }

    [[nodiscard]] Color3f toGamma() const
    {
      return Color3f(
        ch::toGamma(r),
        ch::toGamma(g),
        ch::toGamma(b));
    }
  };
} // mdEngine
