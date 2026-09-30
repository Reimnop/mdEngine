#pragma once

namespace mdEngine
{
  struct Scene
  {
    virtual ~Scene() = default;
    virtual void update() = 0;
    virtual void renderGui() = 0;
    virtual void render() = 0;
  };
} // mdEngine
