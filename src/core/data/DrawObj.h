#pragma once

#include <optional>

#include "core/data/Color3.h"
#include "core/math/Colors.h"
#include "core/math/Matrices.h"
#include "core/rendering/MeshHandle.h"
#include "core/rendering/TextureHandle.h"

namespace mdEngine
{
  enum class PrimitiveType
  {
    Triangles,
    Lines
  };

  enum class ShadingMode
  {
    Normal = 0,
    Flat = 1
  };

  struct DrawObj
  {
    MeshHandle meshHandle{};
    Mat4 transform{};
    Color3f color{0.8f, 0.8f, 0.8f};
    bool drawInIdPass;
    Color3 idColor{};
    PrimitiveType primitiveType = PrimitiveType::Triangles;
    ShadingMode shadingMode = ShadingMode::Normal;
    float shininess = 32.0f;
    float specularStrength = 0.5f;
    std::optional<TextureHandle> texture{};
  };
} // mdEngine