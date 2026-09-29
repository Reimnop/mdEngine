#pragma once

#include "core/math/Matrices.h"

namespace mdEngine
{
  struct CameraObj
  {
    Mat4 view{};
    float fov = 0.7853981634f; // 45 degrees in radians
  };
} // mdEngine