#include "SceneBase.h"

#include <imgui.h>

SceneBase::SceneBase(mdEngine::Renderer* rendererPtr, mdEngine::Window*): rendererPtr(rendererPtr), camera(15.0f), grid(rendererPtr)
{
}

void SceneBase::renderGui()
{
  auto& io = ImGui::GetIO();

  ImGui::DockSpaceOverViewport();

  if (ImGui::Begin("Stats"))
  {
    ImGui::Text("%.1f FPS", io.Framerate);
    ImGui::Text("Orbit: middle mouse (or Alt + left)");
    ImGui::Text("Pan: Shift + middle | Zoom: scroll");
    ImGui::Text("Views: numpad 1/3/7 (Ctrl: opposite)");
    ImGui::Text("Distance: %.2f", camera.getDistance());
    if (ImGui::Button("Reset"))
      camera.reset();
  }
  ImGui::End();

  if (ImGui::Begin("Lighting"))
  {
    if (ImGui::BeginCombo("Fill Mode", FILL_MODE_NAMES[static_cast<int>(lightingObj.fillMode)]))
    {
      for (auto i = 0; i < 2; i++)
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
      for (auto i = 1; i < 4; i++)
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
  }
  ImGui::End();

  if (ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
  {
    const auto viewportSize = ImGui::GetContentRegionAvail();

    const auto imagePos = ImGui::GetCursorScreenPos();
    const bool hovered = ImGui::IsWindowHovered()
      && ImGui::IsMouseHoveringRect(imagePos, ImVec2(imagePos.x + viewportSize.x, imagePos.y + viewportSize.y));
    camera.update(io, hovered, viewportSize.y);

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

void SceneBase::render(std::vector<mdEngine::DrawObj>& drawObjs, mdEngine::CameraObj&, mdEngine::LightingObj&)
{
  grid.render(drawObjs);
}
