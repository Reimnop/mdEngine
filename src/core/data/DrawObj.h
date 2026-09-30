#pragma once

#include <optional>

#include "core/math/Matrices.h"
#include "core/math/Vectors.h"
#include "core/rendering/MeshHandle.h"
#include "core/rendering/TextureHandle.h"

namespace mdEngine
{
  struct DrawObj
  {
    MeshHandle meshHandle{};
    Mat4 transform{};
    Vec3 color{0.8f, 0.8f, 0.8f};
    float shininess = 32.0f;
    std::optional<TextureHandle> texture{};
  };
} // mdEngine