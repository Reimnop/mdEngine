#pragma once

#include "core/math/Matrices.h"
#include "core/math/Vectors.h"

namespace mdEngine
{
  constexpr float DEG_TO_RAD = 0.01745329252f;
  constexpr float RAD_TO_DEG = 57.29577951308232f;

  struct Transform
  {
    Vec3 position{};
    Vec3 rotation{}; // euler angles, degrees
    Vec3 scale{1.0f, 1.0f, 1.0f};

    // translate * Rz * Ry * Rx * scale
    [[nodiscard]] Mat4 toMatrix() const
    {
      return Mat4::translate(position.x, position.y, position.z)
        * Mat4::rotate(rotation.z * DEG_TO_RAD, 0.0f, 0.0f, 1.0f)
        * Mat4::rotate(rotation.y * DEG_TO_RAD, 0.0f, 1.0f, 0.0f)
        * Mat4::rotate(rotation.x * DEG_TO_RAD, 1.0f, 0.0f, 0.0f)
        * Mat4::scale(scale.x, scale.y, scale.z);
    }
  };
} // mdEngine
