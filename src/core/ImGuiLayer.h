#pragma once

#include "core/Window.h"

namespace mdEngine
{
  // Owns the Dear ImGui context and its GLFW / OpenGL3 backends.
  // Per frame: beginFrame() -> (build UI with ImGui::*) -> endFrame().
  class ImGuiLayer
  {
  public:
    explicit ImGuiLayer(Window* windowPtr);
    ~ImGuiLayer();

    ImGuiLayer(const ImGuiLayer&) = delete;
    ImGuiLayer& operator=(const ImGuiLayer&) = delete;

    void beginFrame();
    void endFrame();
  };
} // mdEngine
