#pragma once

#include <optional>

#include "LightingObj.h"
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
    ShadingMode shadingMode = ShadingMode::Inherit;
    float shininess = 32.0f;
    float specularStrength = 0.5f;
    std::optional<TextureHandle> texture{};
  };
} // mdEngine