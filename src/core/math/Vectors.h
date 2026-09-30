#pragma once

#include <cmath>

namespace mdEngine
{
  struct Vec4
  {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    Vec4& operator+=(const Vec4& other)
    {
      this->x += other.x;
      this->y += other.y;
      this->z += other.z;
      this->w += other.w;
      return *this;
    }

    Vec4& operator-=(const Vec4& other)
    {
      this->x -= other.x;
      this->y -= other.y;
      this->z -= other.z;
      this->w -= other.w;
      return *this;
    }

    Vec4& operator*=(const float scalar)
    {
      this->x *= scalar;
      this->y *= scalar;
      this->z *= scalar;
      this->w *= scalar;
      return *this;
    }

    Vec4& operator/=(const float scalar)
    {
      this->x /= scalar;
      this->y /= scalar;
      this->z /= scalar;
      this->w /= scalar;
      return *this;
    }

    Vec4 operator+(const Vec4& other) const
    {
      return Vec4{.x = this->x + other.x, .y = this->y + other.y, .z = this->z + other.z, .w = this->w + other.w};
    }

    Vec4 operator-(const Vec4& other) const
    {
      return Vec4{.x = this->x - other.x, .y = this->y - other.y, .z = this->z - other.z, .w = this->w - other.w};
    }

    Vec4 operator*(const float scalar) const
    {
      return Vec4{.x = this->x * scalar, .y = this->y * scalar, .z = this->z * scalar, .w = this->w * scalar};
    }

    Vec4 operator/(const float scalar) const
    {
      return Vec4{.x = this->x / scalar, .y = this->y / scalar, .z = this->z / scalar, .w = this->w / scalar};
    }
  };

  struct Vec3
  {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3& operator+=(const Vec3& other)
    {
      this->x += other.x;
      this->y += other.y;
      this->z += other.z;
      return *this;
    }

    Vec3& operator-=(const Vec3& other)
    {
      this->x -= other.x;
      this->y -= other.y;
      this->z -= other.z;
      return *this;
    }

    Vec3& operator*=(const float scalar)
    {
      this->x *= scalar;
      this->y *= scalar;
      this->z *= scalar;
      return *this;
    }

    Vec3& operator/=(const float scalar)
    {
      this->x /= scalar;
      this->y /= scalar;
      this->z /= scalar;
      return *this;
    }

    Vec3 operator+(const Vec3& other) const
    {
      return Vec3{.x = this->x + other.x, .y = this->y + other.y, .z = this->z + other.z};
    }

    Vec3 operator-(const Vec3& other) const
    {
      return Vec3{.x = this->x - other.x, .y = this->y - other.y, .z = this->z - other.z};
    }

    Vec3 operator*(const float scalar) const
    {
      return Vec3{.x = this->x * scalar, .y = this->y * scalar, .z = this->z * scalar};
    }

    Vec3 operator/(const float scalar) const
    {
      return Vec3{.x = this->x / scalar, .y = this->y / scalar, .z = this->z / scalar};
    }

    static Vec3 cross(const Vec3& a, const Vec3& b)
    {
      return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    }

    static Vec3 normalize(const Vec3& a)
    {
      const float len = std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
      return len > 1e-12f ? a / len : Vec3{0.0f, 0.0f, 0.0f};
    }

    static float lengthSquared(const Vec3& a)
    {
      return a.x * a.x + a.y * a.y + a.z * a.z;
    }

    static float length(const Vec3& a)
    {
      return std::sqrt(lengthSquared(a));
    }
  };

  struct Vec2
  {
    float x = 0.0f;
    float y = 0.0f;

    Vec2& operator+=(const Vec2& other)
    {
      this->x += other.x;
      this->y += other.y;
      return *this;
    }

    Vec2& operator-=(const Vec2& other)
    {
      this->x -= other.x;
      this->y -= other.y;
      return *this;
    }

    Vec2& operator*=(const float scalar)
    {
      this->x *= scalar;
      this->y *= scalar;
      return *this;
    }

    Vec2& operator/=(const float scalar)
    {
      this->x /= scalar;
      this->y /= scalar;
      return *this;
    }

    Vec2 operator+(const Vec2& other) const
    {
      return Vec2{.x = this->x + other.x, .y = this->y + other.y};
    }

    Vec2 operator-(const Vec2& other) const
    {
      return Vec2{.x = this->x - other.x, .y = this->y - other.y};
    }

    Vec2 operator*(const float scalar) const
    {
      return Vec2{.x = this->x * scalar, .y = this->y * scalar};
    }

    Vec2 operator/(const float scalar) const
    {
      return Vec2{.x = this->x / scalar, .y = this->y / scalar};
    }
  };
} // mdEngine
