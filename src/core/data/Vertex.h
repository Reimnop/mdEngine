#pragma once

#include "core/math/Colors.h"
#include "core/math/Vectors.h"

namespace mdEngine
{
  struct Vertex
  {
    Vec3 pos{};
    Vec3 normal{};
    Vec2 texCoord{};
    Color3f color{1.0f, 1.0f, 1.0f};
  };
} // mdEngine