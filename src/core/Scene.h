#pragma once

namespace mdEngine
{
  struct Scene
  {
    virtual ~Scene() = default;
    virtual void update(float deltaTime) = 0;
    virtual void renderGui() = 0;
  };
} // mdEngine
