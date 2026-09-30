#pragma once

#include "core/math/Vectors.h"

namespace mdEngine
{
  enum class FillMode
  {
    Solid = 0,
    Wireframe = 1,
    Count
  };

  enum class ShadingMode
  {
    Flat = 0,
    Gouraud = 1,
    Phong = 2,
    Count
  };

  struct LightingObj
  {
    FillMode fillMode = FillMode::Solid;
    ShadingMode shadingMode = ShadingMode::Phong;
    Vec3 direction{0.4f, 0.8f, 0.6f}; // points towards the light
    Vec3 color{1.0f, 1.0f, 1.0f};
    float ambient = 0.3f;
  };
} // mdEngine
