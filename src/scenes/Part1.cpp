#include "Part1.h"

#include <algorithm>
#include <imgui.h>
#include <iterator>

#include "ImageLoader.h"
#include "ObjLoader.h"

namespace
{
  constexpr float DEG_TO_RAD = 0.01745329252f;

  const mdEngine::Vec3 PALETTE[] = {
    {0.90f, 0.35f, 0.30f}, {0.95f, 0.70f, 0.25f}, {0.45f, 0.80f, 0.40f},
    {0.30f, 0.70f, 0.90f}, {0.55f, 0.45f, 0.90f}, {0.90f, 0.45f, 0.75f}
  };

  // ImGui strings are UTF-8, std::filesystem needs to be told
  std::filesystem::path pathFromInput(std::string s)
  {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') // "Copy as path" on Windows
      s = s.substr(1, s.size() - 2);
    return std::filesystem::path(reinterpret_cast<const char8_t*>(s.c_str()));
  }

  // a textured entity usually wants the texture's own colors, so the tint is reset to white
  void setTexture(Entity& e, const std::optional<size_t> texture)
  {
    e.texture = texture;
    if (texture.has_value())
      e.color = {1.0f, 1.0f, 1.0f};
  }

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

Part1::Part1(mdEngine::Renderer* rendererPtr, mdEngine::Window* windowPtr): SceneBase(rendererPtr, windowPtr)
{
  for (auto i = 0; i < static_cast<int>(ShapeType::Count); i++)
  {
    auto mesh = makeShape(static_cast<ShapeType>(i));
    meshes.push_back({SHAPE_NAMES[i], rendererPtr->createMesh(
      mesh.v.data(), static_cast<GLsizei>(mesh.v.size()),
      mesh.i.data(), static_cast<GLsizei>(mesh.i.size()))});
  }

  // one of each shape: 2D on the top row, 3D on the bottom row
  auto col2D = 0, col3D = 0;
  for (auto i = 0; i < static_cast<int>(ShapeType::Count); i++)
  {
    const auto shape = static_cast<ShapeType>(i);
    auto& e = addEntity(i);
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
  for (const auto& mesh : meshes)
    rendererPtr->deleteMesh(mesh.handle);
  for (const auto& texture : textures)
    rendererPtr->deleteTexture(texture.handle);
}

Entity& Part1::addEntity(const size_t mesh)
{
  newEntityCounter++;

  Entity e;
  e.name = meshes[mesh].name + " " + std::to_string(newEntityCounter);
  e.mesh = mesh;
  e.position = {0.0f, 0.0f, 0.0f};
  e.rotation = {0.0f, 0.0f, 0.0f};
  e.scale = {1.0f, 1.0f, 1.0f};
  e.color = PALETTE[newEntityCounter % std::size(PALETTE)];
  entities.push_back(e);
  return entities.back();
}

void Part1::importObj(const std::string& path)
{
  try
  {
    const auto fsPath = pathFromInput(path);
    auto mesh = loadObj(fsPath);

    const auto stem = fsPath.stem().u8string();
    meshes.push_back({std::string(stem.begin(), stem.end()), rendererPtr->createMesh(
      mesh.v.data(), static_cast<GLsizei>(mesh.v.size()),
      mesh.i.data(), static_cast<GLsizei>(mesh.i.size()))});

    selectedNewMesh = meshes.size() - 1;
    addEntity(selectedNewMesh);
    selectedEntityIdx = entities.size() - 1;
    importStatus = "Imported " + meshes.back().name + ": " + std::to_string(mesh.v.size()) + " vertices, "
      + std::to_string(mesh.i.size() / 3) + " triangles";
  }
  catch (const std::exception& e)
  {
    importStatus = std::string("Import failed: ") + e.what();
  }
}

void Part1::loadTexture(const std::string& path)
{
  try
  {
    const auto fsPath = pathFromInput(path);
    const auto image = loadImage(fsPath);

    const auto stem = fsPath.stem().u8string();
    textures.push_back({
      std::string(stem.begin(), stem.end()),
      rendererPtr->createTexture(image.width, image.height, image.pixels.data()),
      image.width, image.height});

    textureStatus = "Loaded " + textures.back().name + " (" + std::to_string(image.width) + "x" + std::to_string(image.height) + ")";
    if (selectedEntityIdx.has_value())
    {
      auto& e = entities[selectedEntityIdx.value()];
      setTexture(e, textures.size() - 1);
      textureStatus += ", applied to " + e.name;
    }
  }
  catch (const std::exception& e)
  {
    textureStatus = std::string("Load failed: ") + e.what();
  }
}

void Part1::update()
{
}

void Part1::render(std::vector<mdEngine::DrawObj>& drawObjs, mdEngine::CameraObj& cameraObj, mdEngine::LightingObj& lightingObj)
{
  cameraObj.view = camera.getViewMatrix();

  drawObjs.clear();
  for (const auto& e : entities)
  {
    mdEngine::DrawObj obj{
      .meshHandle = meshes[e.mesh].handle,
      .transform = getTransform(e),
      .color = e.color,
      .shininess = e.shininess
    };
    if (e.texture.has_value())
      obj.texture = textures[e.texture.value()].handle;
    drawObjs.push_back(obj);
  }
}

void Part1::renderGui()
{
  SceneBase::renderGui();

  if (ImGui::Begin("Entities"))
  {
    if (ImGui::BeginCombo("Shape", meshes[selectedNewMesh].name.c_str()))
    {
      for (size_t i = 0; i < meshes.size(); i++)
      {
        ImGui::PushID(static_cast<int>(i));
        const bool isSelected = (selectedNewMesh == i);
        if (ImGui::Selectable(meshes[i].name.c_str(), isSelected))
          selectedNewMesh = i;
        if (isSelected)
          ImGui::SetItemDefaultFocus();
        ImGui::PopID();
      }
      ImGui::EndCombo();
    }

    if (ImGui::Button("Add"))
    {
      addEntity(selectedNewMesh);
    }

    ImGui::Separator();
    const bool enterPressed = ImGui::InputTextWithHint("OBJ path", "path/to/model.obj", objPath, sizeof(objPath), ImGuiInputTextFlags_EnterReturnsTrue);
    if (ImGui::Button("Import OBJ") || enterPressed)
      importObj(objPath);
    if (!importStatus.empty())
      ImGui::TextWrapped("%s", importStatus.c_str());

    ImGui::Separator();
    const bool textureEnterPressed = ImGui::InputTextWithHint("Texture path", "path/to/image.png", texturePath, sizeof(texturePath), ImGuiInputTextFlags_EnterReturnsTrue);
    if (ImGui::Button("Load Texture") || textureEnterPressed)
      loadTexture(texturePath);
    if (!textureStatus.empty())
      ImGui::TextWrapped("%s", textureStatus.c_str());
    ImGui::Separator();

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
      ImGui::DragFloat("Shininess", &e.shininess, 1.0f, 1.0f, 256.0f);

      if (ImGui::BeginCombo("Texture", e.texture.has_value() ? textures[e.texture.value()].name.c_str() : "None"))
      {
        if (ImGui::Selectable("None", !e.texture.has_value()))
          setTexture(e, std::nullopt);
        for (size_t i = 0; i < textures.size(); i++)
        {
          ImGui::PushID(static_cast<int>(i));
          const bool isSelected = (e.texture == i);
          if (ImGui::Selectable(textures[i].name.c_str(), isSelected))
            setTexture(e, i);
          if (isSelected)
            ImGui::SetItemDefaultFocus();
          ImGui::PopID();
        }
        ImGui::EndCombo();
      }

      if (e.texture.has_value())
      {
        const auto& texture = textures[e.texture.value()];
        const float scale = 96.0f / static_cast<float>(std::max(texture.width, texture.height));
        ImGui::Image(texture.handle.handle, ImVec2(texture.width * scale, texture.height * scale), ImVec2(0, 1), ImVec2(1, 0));
      }

      if (ImGui::Button("Delete"))
      {
        entities.erase(entities.begin() + selectedEntityIdx.value());
        selectedEntityIdx.reset();
      }
    }
  }
  ImGui::End();
}
