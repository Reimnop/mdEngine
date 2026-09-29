#pragma once

#include <vector>

#include "data/CameraObj.h"
#include "data/DrawObj.h"

namespace mdEngine
{
  struct Scene
  {
    virtual ~Scene() = default;
    virtual void update() = 0;
    virtual void render(std::vector<DrawObj>& drawObjs, CameraObj& cameraObj) = 0;
  };
} // mdEngine
