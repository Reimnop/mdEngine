#pragma once

#include <memory>

#include "ImGuiLayer.h"
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
    ImGuiLayer imgui;

    std::unique_ptr<Scene> currentScene;
  };
} // mdEngine
