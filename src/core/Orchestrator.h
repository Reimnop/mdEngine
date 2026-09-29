#pragma once

#include <memory>

#include "Scene.h"
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

    std::vector<DrawObj> drawObjs{};
    CameraObj cameraObj{};

    std::unique_ptr<Scene> currentScene;
  };
} // mdEngine
