#pragma once

#include <glad/gl.h>

#include "core/memory/Allocation.h"

namespace mdEngine
{
  struct MeshHandle
  {
    Allocation vertexAllocation{};
    Allocation indexAllocation{};
    GLint indexCount = 0;
  };
} // mdEngine
