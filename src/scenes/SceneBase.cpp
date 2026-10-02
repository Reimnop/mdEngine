#include "SceneBase.h"

#include <imgui.h>

SceneBase::SceneBase(mdEngine::Renderer* rendererPtr, mdEngine::Window* windowPtr): rendererPtr(rendererPtr), camera(windowPtr, 15.0f)
{
}

void SceneBase::renderGui()
{
  auto& io = ImGui::GetIO();

  ImGui::DockSpaceOverViewport();

  if (ImGui::Begin("Stats"))
  {
    ImGui::Text("%.1f FPS", io.Framerate);
    ImGui::Text("Left: rotate | Right/middle: pan | Scroll: zoom");
    ImGui::Text("Distance: %.2f", camera.getDistance());
    if (ImGui::Button("Reset"))
      camera.reset();
  }
  ImGui::End();

  if (ImGui::Begin("Lighting"))
  {
    if (ImGui::BeginCombo("Fill Mode", FILL_MODE_NAMES[static_cast<int>(lightingObj.fillMode)]))
    {
      for (auto i = 0; i < static_cast<int>(mdEngine::FillMode::Count); i++)
      {
        const bool isSelected = (lightingObj.fillMode == static_cast<mdEngine::FillMode>(i));
        if (ImGui::Selectable(FILL_MODE_NAMES[i], isSelected))
          lightingObj.fillMode = static_cast<mdEngine::FillMode>(i);
        if (isSelected)
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("Shading Mode", SHADING_MODE_NAMES[static_cast<int>(lightingObj.shadingMode)]))
    {
      for (auto i = 0; i < static_cast<int>(mdEngine::ShadingMode::Count); i++)
      {
        const bool isSelected = (lightingObj.shadingMode == static_cast<mdEngine::ShadingMode>(i));
        if (ImGui::Selectable(SHADING_MODE_NAMES[i], isSelected))
          lightingObj.shadingMode = static_cast<mdEngine::ShadingMode>(i);
        if (isSelected)
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    ImGui::DragFloat3("Direction", &lightingObj.direction.x, 0.01f);
    ImGui::ColorEdit3("Color", &lightingObj.color.r);
    ImGui::DragFloat("Ambient", &lightingObj.ambient, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Specular Strength", &lightingObj.specularStrength, 0.01f, 0.0f, 1.0f);
  }
  ImGui::End();

  if (ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
  {
    if (ImGui::IsWindowFocused())
      camera.update(io);

    const auto viewportSize = ImGui::GetContentRegionAvail();

    drawObjs.clear();
    render(drawObjs, cameraObj, lightingObj);

    const auto colorTex = rendererPtr->renderFrame(
      drawObjs.data(),
      drawObjs.size(),
      cameraObj,
      lightingObj,
      viewportSize.x,
      viewportSize.y);

    ImGui::Image(colorTex, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
  }
  ImGui::End();
}
