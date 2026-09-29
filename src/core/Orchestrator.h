#pragma once

#include "rendering/Renderer.h"

namespace mdEngine
{
  class Orchestrator
  {
  public:
    Orchestrator();
    ~Orchestrator();

    void start();
  private:
    Window window;
    Renderer renderer;

    MeshHandle bullshit;
  };
} // mdEngine
