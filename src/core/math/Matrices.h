#pragma once

#include <cmath>

#include "Vectors.h"

namespace mdEngine
{
  struct Mat4
  {
    float m[16] = {
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };

    [[nodiscard]] Vec4 operator*(const Vec4& v) const
    {
      return Vec4{
        .x = m[0]  * v.x + m[1]  * v.y + m[2]  * v.z + m[3]  * v.w,
        .y = m[4]  * v.x + m[5]  * v.y + m[6]  * v.z + m[7]  * v.w,
        .z = m[8]  * v.x + m[9]  * v.y + m[10] * v.z + m[11] * v.w,
        .w = m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w
      };
    }

    Mat4 operator*(const Mat4& other) const
    {
      const float* a = this->m;
      const float* b = other.m;
      Mat4 r;

      r.m[0]  = a[0]*b[0]  + a[1]*b[4]  + a[2]*b[8]   + a[3]*b[12];
      r.m[1]  = a[0]*b[1]  + a[1]*b[5]  + a[2]*b[9]   + a[3]*b[13];
      r.m[2]  = a[0]*b[2]  + a[1]*b[6]  + a[2]*b[10]  + a[3]*b[14];
      r.m[3]  = a[0]*b[3]  + a[1]*b[7]  + a[2]*b[11]  + a[3]*b[15];

      r.m[4]  = a[4]*b[0]  + a[5]*b[4]  + a[6]*b[8]   + a[7]*b[12];
      r.m[5]  = a[4]*b[1]  + a[5]*b[5]  + a[6]*b[9]   + a[7]*b[13];
      r.m[6]  = a[4]*b[2]  + a[5]*b[6]  + a[6]*b[10]  + a[7]*b[14];
      r.m[7]  = a[4]*b[3]  + a[5]*b[7]  + a[6]*b[11]  + a[7]*b[15];

      r.m[8]  = a[8]*b[0]  + a[9]*b[4]  + a[10]*b[8]  + a[11]*b[12];
      r.m[9]  = a[8]*b[1]  + a[9]*b[5]  + a[10]*b[9]  + a[11]*b[13];
      r.m[10] = a[8]*b[2]  + a[9]*b[6]  + a[10]*b[10] + a[11]*b[14];
      r.m[11] = a[8]*b[3]  + a[9]*b[7]  + a[10]*b[11] + a[11]*b[15];

      r.m[12] = a[12]*b[0] + a[13]*b[4] + a[14]*b[8]  + a[15]*b[12];
      r.m[13] = a[12]*b[1] + a[13]*b[5] + a[14]*b[9]  + a[15]*b[13];
      r.m[14] = a[12]*b[2] + a[13]*b[6] + a[14]*b[10] + a[15]*b[14];
      r.m[15] = a[12]*b[3] + a[13]*b[7] + a[14]*b[11] + a[15]*b[15];

      return r;
    }

    Mat4 operator*=(const Mat4& other)
    {
      *this = *this * other;
      return *this;
    }

    [[nodiscard]] static Mat4 translate(const float x, const float y, const float z)
    {
      Mat4 r{};
      r.m[3] = x;
      r.m[7] = y;
      r.m[11] = z;
      return r;
    }

    [[nodiscard]] static Mat4 scale(const float x, const float y, const float z)
    {
      Mat4 r{};
      r.m[0] = x;
      r.m[5] = y;
      r.m[10] = z;
      return r;
    }

    [[nodiscard]] static Mat4 rotate(const float angle, const float x, const float y, const float z)
    {
      Mat4 r{};
      const float c = std::cos(angle);
      const float s = std::sin(angle);
      const float t = 1.0f - c;

      r.m[0] = t * x * x + c;
      r.m[1] = t * x * y - s * z;
      r.m[2] = t * x * z + s * y;

      r.m[4] = t * x * y + s * z;
      r.m[5] = t * y * y + c;
      r.m[6] = t * y * z - s * x;

      r.m[8] = t * x * z - s * y;
      r.m[9] = t * y * z + s * x;
      r.m[10] = t * z * z + c;

      return r;
    }

    [[nodiscard]] static Mat4 perspective(const float fovY, const float aspect, const float zNear, const float zFar)
    {
      const float f = 1.0f / std::tan(fovY * 0.5f);
      const float range = zNear - zFar;

      Mat4 r;
      r.m[0]  = f / aspect;
      r.m[5]  = f;
      r.m[10] = (zFar + zNear) / range;
      r.m[11] = (2.0f * zFar * zNear) / range;
      r.m[14] = -1.0f;
      r.m[15] = 0.0f;

      return r;
    }
  };
} // mdEngine