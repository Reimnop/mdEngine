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

    ImGui::Checkbox("Grid", &gridEnabled);
  }
  ImGui::End();

  if (ImGui::Begin("Lighting"))
  {
    ImGui::DragFloat3("Direction", &lightingObj.direction.x, 0.01f);
    ImGui::ColorEdit3("Color", &lightingObj.color.r);
    ImGui::ColorEdit3("Clear Color", &lightingObj.clearColor.r);
    ImGui::DragFloat("Intensity", &lightingObj.intensity, 0.01f, 0.1f, 10.0f);
    ImGui::DragFloat("Ambient", &lightingObj.ambient, 0.01f, 0.0f, 1.0f);
  }
  ImGui::End();

  if (ImGui::Begin("Post Processing"))
  {
    ImGui::Checkbox("Use Bloom", &postProcessingObj.useBloom);
    if (postProcessingObj.useBloom)
    {
      ImGui::DragFloat("Bloom Intensity", &postProcessingObj.bloomIntensity, 0.01f, 0.0f, 10.0f);
      ImGui::DragFloat("Bloom Diffusion", &postProcessingObj.bloomDiffusion, 0.05f, 1.0f, 10.0f);
      ImGui::DragFloat("Bloom Threshold", &postProcessingObj.bloomThreshold, 0.01f, 0.0f, 10.0f);
      ImGui::DragFloat("Bloom Knee", &postProcessingObj.bloomKnee, 0.01f, 0.0f, 1.0f);
      ImGui::ColorEdit3("Bloom Color", &postProcessingObj.bloomColor.r);
    }

    ImGui::Checkbox("Use Tonemapping", &postProcessingObj.useTonemapping);
    ImGui::DragFloat("Exposure", &postProcessingObj.exposure, 0.01f, 0.1f, 10.0f);
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
    render(drawObjs, cameraObj, lightingObj, postProcessingObj);

    const auto colorTex = rendererPtr->renderFrame(
      drawObjs.data(),
      drawObjs.size(),
      cameraObj,
      lightingObj,
      postProcessingObj,
      viewportSize.x,
      viewportSize.y);

    ImGui::Image(colorTex, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
  }
  ImGui::End();
}

void SceneBase::render(std::vector<mdEngine::DrawObj>& drawObjs, mdEngine::CameraObj&, mdEngine::LightingObj&, mdEngine::PostProcessingObj&)
{
  if (gridEnabled)
    grid.render(drawObjs);
}
