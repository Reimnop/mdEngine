#pragma once

#include "core/math/Vectors.h"
#include "core/math/Colors.h"

namespace mdEngine
{
  struct LightingObj
  {
    Vec3 direction{0.4f, 0.8f, 0.6f}; // points towards the light
    Color3f color{1.0f, 1.0f, 1.0f};
    Color3f clearColor{0.0f, 0.0f, 0.0f};
    float intensity = 1.0f;
    float ambient = 0.3f;
  };
} // mdEngine
