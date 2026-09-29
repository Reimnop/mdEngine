#pragma once

#include "core/math/Matrices.h"
#include "core/rendering/MeshHandle.h"

namespace mdEngine
{
  struct DrawObj
  {
    MeshHandle meshHandle{};
    Mat4 transform{};
  };
} // mdEngine