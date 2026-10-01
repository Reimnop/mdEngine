#pragma once

#include <memory>

#include "ImGuiLayer.h"
#include "Scene.h"
#include "rendering/Renderer.h"

namespace mdEngine
{
  constexpr const char* SCENE_NAMES[] = {
    "Part 1 (Drawing Basic Shapes)",
    "Part 2 (Visualization of Atoms and Molecules)"
  };

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

    size_t currentSceneIdx = 0;

    void renderSceneSwitcher();
  };
} // mdEngine
