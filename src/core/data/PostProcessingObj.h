#pragma once

#include "core/math/Colors.h"

namespace mdEngine
{
  struct PostProcessingObj
  {
    bool useBloom = false;
    float bloomIntensity = 0.5f;
    float bloomDiffusion = 7.0f;
    float bloomThreshold = 0.9f;
    float bloomKnee = 0.5f;
    Color3f bloomColor{1.0f, 1.0f, 1.0f};

    bool useTonemapping = true;
    float exposure = 1.0f;
  };
} // mdEngine
