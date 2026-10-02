#pragma once

#include <optional>

#include "core/math/Colors.h"
#include "core/math/Matrices.h"
#include "core/rendering/MeshHandle.h"
#include "core/rendering/TextureHandle.h"

namespace mdEngine
{
  struct DrawObj
  {
    MeshHandle meshHandle{};
    Mat4 transform{};
    Color3f color{0.8f, 0.8f, 0.8f};
    float shininess = 32.0f;
    std::optional<TextureHandle> texture{};
  };
} // mdEngine