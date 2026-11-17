#include "Part1.h"

#include <algorithm>
#include <cfloat>
#include <cstring>
#include <imgui.h>

namespace
{
  using mdEngine::Mat4;

  std::filesystem::path pathFromInput(std::string s)
  {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
      s = s.substr(1, s.size() - 2);
    return std::filesystem::path(std::u8string(s.begin(), s.end()));
  }

  size_t countNodes(const SceneNode& node)
  {
    size_t count = 1;
    for (const auto& child : node.children)
      count += countNodes(*child);
    return count;
  }
}

Part1::Part1(mdEngine::Renderer* rendererPtr, mdEngine::Window* windowPtr): SceneBase(rendererPtr, windowPtr)
{
}

Part1::~Part1()
{
  clearScene();
}

void Part1::update(float)
{
}

void Part1::render(std::vector<mdEngine::DrawObj>& drawObjs, mdEngine::CameraObj& cameraObj, mdEngine::LightingObj& lightingObj, mdEngine::PostProcessingObj& postProcessingObj)
{
  SceneBase::render(drawObjs, cameraObj, lightingObj, postProcessingObj);

  cameraObj.view = camera.getViewMatrix();

  for (const auto& model : models)
    collectDrawObjs(*model, *model->root, Mat4{}, drawObjs);
}

void Part1::collectDrawObjs(const Model& model, const SceneNode& node, const Mat4& parentWorld, std::vector<mdEngine::DrawObj>& drawObjs)
{
  if (!node.visible)
    return;

  const Mat4 world = parentWorld * node.transform.toMatrix();
  for (const auto meshIdx : node.meshes)
  {
    const auto& mesh = model.meshes[meshIdx];
    const auto& material = model.materials[mesh.material];
    drawObjs.push_back(mdEngine::DrawObj{
      .meshHandle = mesh.handle,
      .transform = world,
      .color = material.color,
      .shininess = material.shininess,
      .specularStrength = material.specularStrength,
      .texture = material.texture.has_value()
        ? std::optional(model.textures[material.texture.value()].handle)
        : std::nullopt
    });
  }

  for (const auto& child : node.children)
    collectDrawObjs(model, *child, world, drawObjs);
}

void Part1::renderGui()
{
  SceneBase::renderGui();

  drawImportPanel();
  drawSceneTree();
  drawInspector();
}

void Part1::importModel(const std::string& path)
{
  try
  {
    const auto fsPath = pathFromInput(path);
    auto data = loadModel(fsPath);

    auto model = std::make_unique<Model>();
    model->name = data.name;

    for (const auto& image : data.textures)
      model->textures.push_back({
        .handle = rendererPtr->createTexture(image.width, image.height, image.pixels.data(), true),
        .width = image.width,
        .height = image.height});

    for (const auto& material : data.materials)
      model->materials.push_back({
        .name = material.name,
        .color = material.color,
        .shininess = material.shininess,
        .specularStrength = material.specularStrength,
        .texture = material.texture});

    size_t vertexCount = 0, triangleCount = 0;
    for (const auto& mesh : data.meshes)
    {
      vertexCount += mesh.vertices.size();
      triangleCount += mesh.indices.size() / 3;
      model->meshes.push_back({
        .name = mesh.name,
        .handle = mesh.indices.empty()
          ? mdEngine::MeshHandle{}
          : rendererPtr->createMesh(
              mesh.vertices.data(), static_cast<GLsizei>(mesh.vertices.size()),
              mesh.indices.data(), static_cast<GLsizei>(mesh.indices.size())),
        .material = mesh.material,
        .vertexCount = mesh.vertices.size(),
        .triangleCount = mesh.indices.size() / 3});
    }

    model->root = buildNode(data.root, nullptr);

    selectedNode = model->root.get();
    selectedModel = model.get();
    importStatus = "Imported " + model->name + ": " + std::to_string(countNodes(*model->root)) + " nodes, "
      + std::to_string(data.meshes.size()) + " meshes, " + std::to_string(vertexCount) + " vertices, "
      + std::to_string(triangleCount) + " triangles";
    importFailed = false;
    importWarnings = std::move(data.warnings);
    models.push_back(std::move(model));
  }
  catch (const std::exception& e)
  {
    importStatus = std::string("Import failed: ") + e.what();
    importFailed = true;
    importWarnings.clear();
  }
}

std::unique_ptr<SceneNode> Part1::buildNode(const ModelNode& source, SceneNode* parent)
{
  auto node = std::make_unique<SceneNode>();
  node->id = nextNodeId++;
  node->name = source.name;
  node->transform = source.transform;
  node->meshes = source.meshes;
  node->parent = parent;
  node->children.reserve(source.children.size());
  for (const auto& child : source.children)
    node->children.push_back(buildNode(child, node.get()));
  return node;
}

void Part1::destroyModel(Model& model) const
{
  for (const auto& mesh : model.meshes)
    if (mesh.triangleCount > 0)
      rendererPtr->deleteMesh(mesh.handle);
  for (const auto& texture : model.textures)
    rendererPtr->deleteTexture(texture.handle);
}

void Part1::removeModel(const Model* model)
{
  const auto it = std::ranges::find_if(models, [&](const auto& m) { return m.get() == model; });
  if (it == models.end())
    return;
  destroyModel(**it);
  models.erase(it);
  selectedNode = nullptr;
  selectedModel = nullptr;
}

void Part1::removeNode(SceneNode* node)
{
  SceneNode* parent = node->parent;
  if (parent == nullptr)
    return;
  std::erase_if(parent->children, [&](const auto& child) { return child.get() == node; });
  selectedNode = nullptr;
  selectedModel = nullptr;
}

void Part1::clearScene()
{
  for (const auto& model : models)
    destroyModel(*model);
  models.clear();
  selectedNode = nullptr;
  selectedModel = nullptr;
}

// ---- GUI ----

void Part1::drawImportPanel()
{
  if (ImGui::Begin("Import"))
  {
    ImGui::TextWrapped("OBJ, FBX, glTF, DAE, 3DS, STL, PLY");
    ImGui::SetNextItemWidth(-FLT_MIN);
    const bool submitted = ImGui::InputTextWithHint("##path", "path to model file", modelPath, sizeof(modelPath),
      ImGuiInputTextFlags_EnterReturnsTrue);

    if ((ImGui::Button("Import") || submitted) && modelPath[0] != '\0')
      importModel(modelPath);

    ImGui::SameLine();
    ImGui::BeginDisabled(models.empty());
    if (ImGui::Button("Clear scene"))
      clearScene();
    ImGui::EndDisabled();

    if (!importStatus.empty())
    {
      ImGui::PushStyleColor(ImGuiCol_Text, importFailed ? ImVec4(1.0f, 0.4f, 0.4f, 1.0f) : ImVec4(0.5f, 0.9f, 0.5f, 1.0f));
      ImGui::TextWrapped("%s", importStatus.c_str());
      ImGui::PopStyleColor();
    }
    for (const auto& warning : importWarnings)
    {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.8f, 0.3f, 1.0f));
      ImGui::TextWrapped("Warning: %s", warning.c_str());
      ImGui::PopStyleColor();
    }
  }
  ImGui::End();
}

void Part1::drawSceneTree()
{
  if (ImGui::Begin("Scene Tree"))
  {
    if (models.empty())
      ImGui::TextDisabled("Empty scene, please import a model");

    for (const auto& model : models)
      drawTreeNode(*model, *model->root);
  }
  ImGui::End();
}

void Part1::drawTreeNode(Model& model, SceneNode& node)
{
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
    | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DrawLinesToNodes;
  if (node.children.empty())
    flags |= ImGuiTreeNodeFlags_Leaf;
  if (&node == selectedNode)
    flags |= ImGuiTreeNodeFlags_Selected;
  if (node.parent == nullptr)
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);

  ImGui::PushID(static_cast<int>(node.id));
  if (!node.visible)
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));

  const bool open = ImGui::TreeNodeEx("##node", flags, "%s%s", node.name.c_str(), node.meshes.empty() ? "" : "  [mesh]");
  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
  {
    selectedNode = &node;
    selectedModel = &model;
  }

  if (!node.visible)
    ImGui::PopStyleColor();

  if (open)
  {
    for (const auto& child : node.children)
      drawTreeNode(model, *child);
    ImGui::TreePop();
  }
  ImGui::PopID();
}

void Part1::drawMaterialEditor(MaterialAsset& material, const Model& model)
{
  ImGui::ColorEdit3("Color", &material.color.r);
  ImGui::DragFloat("Shininess", &material.shininess, 0.5f, 1.0f, 256.0f);
  ImGui::DragFloat("Specular", &material.specularStrength, 0.01f, 0.0f, 1.0f);

  if (material.texture.has_value())
  {
    const auto& texture = model.textures[material.texture.value()];
    ImGui::Text("Texture: %dx%d", texture.width, texture.height);
  }
  else
  {
    ImGui::TextDisabled("Texture: none");
  }
}

void Part1::drawInspector()
{
  if (ImGui::Begin("Inspector"))
  {
    if (selectedNode == nullptr || selectedModel == nullptr)
    {
      ImGui::TextDisabled("Please select a node in the Scene Tree");
      ImGui::End();
      return;
    }

    SceneNode& node = *selectedNode;
    Model& model = *selectedModel;

    char nameBuffer[128];
    std::strncpy(nameBuffer, node.name.c_str(), sizeof(nameBuffer) - 1);
    nameBuffer[sizeof(nameBuffer) - 1] = '\0';
    if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
      node.name = nameBuffer;

    ImGui::Checkbox("Visible", &node.visible);
    ImGui::TextDisabled("Model: %s | %zu children", model.name.c_str(), node.children.size());

    ImGui::SeparatorText("Transform");
    ImGui::DragFloat3("Position", &node.transform.position.x, 0.01f, 0.0f, 0.0f, "%.3f");
    ImGui::DragFloat3("Rotation", &node.transform.rotation.x, 0.5f, 0.0f, 0.0f, "%.2f deg");
    ImGui::DragFloat3("Scale", &node.transform.scale.x, 0.01f, 0.0f, 0.0f, "%.3f");
    if (ImGui::Button("Reset transform"))
      node.transform = mdEngine::Transform{};

    if (!node.meshes.empty())
    {
      ImGui::SeparatorText("Meshes");
      for (const auto meshIdx : node.meshes)
      {
        const auto& mesh = model.meshes[meshIdx];
        auto& material = model.materials[mesh.material];

        ImGui::PushID(static_cast<int>(meshIdx));
        if (ImGui::TreeNode("##mesh", "%s (%zu verts, %zu tris)", mesh.name.c_str(), mesh.vertexCount, mesh.triangleCount))
        {
          ImGui::TextDisabled("Material: %s (shared)", material.name.c_str());
          drawMaterialEditor(material, model);
          ImGui::TreePop();
        }
        ImGui::PopID();
      }
    }

    ImGui::Separator();
    if (ImGui::Button(node.parent == nullptr ? "Remove model" : "Delete node"))
    {
      if (node.parent == nullptr)
        removeModel(&model);
      else
        removeNode(&node);
    }
  }
  ImGui::End();
}
