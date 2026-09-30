#include "Part1.h"

#include <imgui.h>
#include <iterator>

namespace
{
  constexpr float DEG_TO_RAD = 0.01745329252f;

  const mdEngine::Vec3 PALETTE[] = {
    {0.90f, 0.35f, 0.30f}, {0.95f, 0.70f, 0.25f}, {0.45f, 0.80f, 0.40f},
    {0.30f, 0.70f, 0.90f}, {0.55f, 0.45f, 0.90f}, {0.90f, 0.45f, 0.75f}
  };

  mdEngine::Mat4 getTransform(const Entity& e)
  {
    using mdEngine::Mat4;
    return Mat4::translate(e.position.x, e.position.y, e.position.z)
      * Mat4::rotate(e.rotation.z * DEG_TO_RAD, 0.0f, 0.0f, 1.0f)
      * Mat4::rotate(e.rotation.y * DEG_TO_RAD, 0.0f, 1.0f, 0.0f)
      * Mat4::rotate(e.rotation.x * DEG_TO_RAD, 1.0f, 0.0f, 0.0f)
      * Mat4::scale(e.scale.x, e.scale.y, e.scale.z);
  }
}

Part1::Part1(mdEngine::Renderer* rendererPtr, mdEngine::Window* windowPtr)
  : rendererPtr(rendererPtr), camera(windowPtr, 15.0f)
{
  for (auto i = 0; i < static_cast<int>(ShapeType::Count); i++)
  {
    auto mesh = makeShape(static_cast<ShapeType>(i));
    meshes[i] = rendererPtr->createMesh(
      mesh.v.data(), static_cast<GLsizei>(mesh.v.size()),
      mesh.i.data(), static_cast<GLsizei>(mesh.i.size()));
  }

  // one of each shape: 2D on the top row, 3D on the bottom row
  auto col2D = 0, col3D = 0;
  for (auto i = 0; i < static_cast<int>(ShapeType::Count); i++)
  {
    const auto shape = static_cast<ShapeType>(i);
    auto& e = addEntity(shape);
    if (is3D(shape))
    {
      e.position = {(static_cast<float>(col3D++) - 3.5f) * 1.4f, -1.0f, 0.0f};
      e.rotation = {25.0f, 35.0f, 0.0f};
    }
    else
    {
      e.position = {(static_cast<float>(col2D++) - 4.0f) * 1.4f, 1.0f, 0.0f};
    }
  }
}

Part1::~Part1()
{
  for (auto i = 0; i < static_cast<int>(ShapeType::Count); i++)
    rendererPtr->deleteMesh(meshes[i]);
}

Entity& Part1::addEntity(const ShapeType shape)
{
  newEntityCounter++;

  Entity e;
  e.name = std::string(SHAPE_NAMES[static_cast<int>(shape)]) + " " + std::to_string(newEntityCounter);
  e.shape = shape;
  e.position = {0.0f, 0.0f, 0.0f};
  e.rotation = {0.0f, 0.0f, 0.0f};
  e.scale = {1.0f, 1.0f, 1.0f};
  e.color = PALETTE[newEntityCounter % std::size(PALETTE)];
  entities.push_back(e);
  return entities.back();
}

void Part1::update()
{
}

void Part1::render()
{
  cameraObj.view = camera.getViewMatrix();

  drawObjs.clear();
  for (const auto& e : entities)
    drawObjs.push_back({
      .meshHandle = meshes[static_cast<int>(e.shape)],
      .transform = getTransform(e),
      .color = e.color
    });
}

void Part1::renderGui()
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
    ImGui::ColorEdit3("Color", &lightingObj.color.x);
    ImGui::DragFloat("Ambient", &lightingObj.ambient, 0.01f, 0.0f, 1.0f);
  }
  ImGui::End();

  if (ImGui::Begin("Entities"))
  {
    if (ImGui::BeginCombo("Shape", SHAPE_NAMES[static_cast<int>(selectedNewShape)]))
    {
      for (auto i = 0; i < static_cast<int>(ShapeType::Count); i++)
      {
        const bool isSelected = (selectedNewShape == static_cast<ShapeType>(i));
        if (ImGui::Selectable(SHAPE_NAMES[i], isSelected))
          selectedNewShape = static_cast<ShapeType>(i);
        if (isSelected)
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    if (ImGui::Button("Add"))
    {
      addEntity(selectedNewShape);
    }

    if (ImGui::BeginListBox("##entities", ImVec2(-FLT_MIN, 200.0f)))
    {
      for (auto i = 0; i < entities.size(); i++)
      {
        ImGui::PushID(i);
        if (ImGui::Selectable(entities[i].name.c_str(), selectedEntityIdx == i))
          selectedEntityIdx = i;
        ImGui::PopID();
      }
      ImGui::EndListBox();
    }

    if (selectedEntityIdx.has_value())
    {
      auto& e = entities[selectedEntityIdx.value()];

      ImGui::Separator();
      ImGui::Text("%s", e.name.c_str());
      ImGui::DragFloat3("Position", &e.position.x, 0.05f);
      ImGui::DragFloat3("Rotation", &e.rotation.x, 1.0f);
      ImGui::DragFloat3("Scale", &e.scale.x, 0.05f, 0.01f, 100.0f);
      ImGui::ColorEdit3("Color", &e.color.x);

      if (ImGui::Button("Delete"))
      {
        entities.erase(entities.begin() + selectedEntityIdx.value());
        selectedEntityIdx.reset();
      }
    }
  }
  ImGui::End();

  if (ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
  {
    if (ImGui::IsWindowFocused())
      camera.update(io);

    const auto viewportSize = ImGui::GetContentRegionAvail();
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
