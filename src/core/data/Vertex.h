#pragma once

#include "core/math/Vectors.h"

namespace mdEngine
{
  struct Vertex
  {
    Vec3 pos{};
    Vec3 normal{};
    Vec2 texCoord{};
  };
} // mdEngine