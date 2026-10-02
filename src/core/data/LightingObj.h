#pragma once

#include "core/math/Vectors.h"
#include "core/math/Colors.h"

namespace mdEngine
{
  enum class FillMode
  {
    Solid = 0,
    Wireframe = 1
  };

  enum class ShadingMode
  {
    Inherit = 0,
    Flat = 1,
    Gouraud = 2,
    Phong = 3
  };

  struct LightingObj
  {
    FillMode fillMode = FillMode::Solid;
    ShadingMode shadingMode = ShadingMode::Phong;
    Vec3 direction{0.4f, 0.8f, 0.6f}; // points towards the light
    Color3f color{1.0f, 1.0f, 1.0f};
    Color3f clearColor{0.0f, 0.0f, 0.0f};
    float intensity = 1.0f;
    float ambient = 0.3f;
  };
} // mdEngine
