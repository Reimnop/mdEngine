#pragma once

#include <glad/gl.h>

namespace mdEngine
{
  struct MeshHandle
  {
    GLint baseVertex{};
    GLint baseIndex{};
    GLsizei indexCount{};
  };
} // mdEngine
