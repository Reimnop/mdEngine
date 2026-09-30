#pragma once

#include "core/math/Matrices.h"
#include "core/math/Vectors.h"
#include "core/rendering/MeshHandle.h"

namespace mdEngine
{
  struct DrawObj
  {
    MeshHandle meshHandle{};
    Mat4 transform{};
    Vec3 color{0.8f, 0.8f, 0.8f};
  };
} // mdEngine