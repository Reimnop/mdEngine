#include "Part2.h"

#include <algorithm>
#include <fstream>
#include <imgui.h>
#include <numeric>

#include "Shapes.h"

namespace
{
  constexpr float DEG_TO_RAD = 0.01745329252f;

  size_t atomicNumber(const Element& e)
  {
    return std::accumulate(e.shells.begin(), e.shells.end(), size_t{0});
  }

  size_t findByName(const auto& items, const std::string& prefix)
  {
    for (size_t i = 0; i < items.size(); i++)
      if (items[i].name.rfind(prefix, 0) == 0)
        return i;
    return 0;
  }
}

ElementInstance::ElementInstance(mdEngine::Renderer* rendererPtr, Part2MeshLibrary* meshLibraryPtr, mdEngine::Color3 color, float radius, const std::vector<size_t>& shells)
  : rendererPtr(rendererPtr), meshLibraryPtr(meshLibraryPtr), color(color), radius(radius), shells(shells)
{
  shellRotations.resize(shells.size(), 0.0f);

  // generate shell tori
  shellTori.reserve(shells.size());
  for (auto i = 0; i < shells.size(); i++)
  {
    const auto shellRadius = static_cast<float>(i + 1);
    const auto torusMesh = makeTorus(shellRadius, 0.02f);
    shellTori.push_back(rendererPtr->createMesh(
      torusMesh.v.data(),
      static_cast<GLsizei>(torusMesh.v.size()),
      torusMesh.i.data(),
      static_cast<GLsizei>(torusMesh.i.size())));
  }
}

ElementInstance::~ElementInstance()
{
  for (const auto& torus : shellTori)
    rendererPtr->deleteMesh(torus);
}

void ElementInstance::update(float deltaTime)
{
  for (auto& rotation : shellRotations)
    rotation += deltaTime * 0.3490658504f; // rotate at 20 degrees per second
}

void ElementInstance::render(const mdEngine::Mat4& transform, std::vector<mdEngine::DrawObj>& drawObjs) const
{
  // render nucleus
  drawObjs.push_back({
    .meshHandle = meshLibraryPtr->sphere,
    .transform = transform * mdEngine::Mat4::scale(radius * 2.0f, radius * 2.0f, radius * 2.0f),
    .color = mdEngine::Color3f(
      static_cast<float>(color.r) / 255.0f,
      static_cast<float>(color.g) / 255.0f,
      static_cast<float>(color.b) / 255.0f),
    .shininess = 32.0f
  });

  // render shells
  for (auto i = 0; i < shells.size(); i++)
  {
    const auto shellRadius = static_cast<float>(i + 1);
    drawObjs.push_back({
      .meshHandle = shellTori[i],
      .transform = transform,
      .color = mdEngine::Color3f(0.2f, 0.2f, 0.2f),
      .shininess = 32.0f
    });

    for (auto j = 0; j < shells[i]; j++)
    {
      const float angle = 6.2831853072f * static_cast<float>(j) / static_cast<float>(shells[i]);
      const float x = shellRadius * std::cos(shellRotations[i] + angle);
      const float z = shellRadius * std::sin(shellRotations[i] + angle);
      drawObjs.push_back({
        .meshHandle = meshLibraryPtr->sphere,
        .transform = transform * mdEngine::Mat4::translate(x, 0.0f, z) * mdEngine::Mat4::scale(0.2f, 0.2f, 0.2f),
        .color = mdEngine::Color3f(0.8f, 0.4f, 0.2f),
        .shininess = 32.0f
      });
    }
  }
}

MoleculeInstance::MoleculeInstance(mdEngine::Renderer* rendererPtr, Part2MeshLibrary* meshLibraryPtr, const std::vector<Molecule::Atom>& atoms, const std::vector<Molecule::Connection>& connections)
  : rendererPtr(rendererPtr), meshLibraryPtr(meshLibraryPtr), atoms(atoms), connections(connections)
{
  rebuildConnectionTransforms();
}

MoleculeInstance::~MoleculeInstance() = default;

void MoleculeInstance::rebuildConnectionTransforms()
{
  connectionTransforms.resize(connections.size());

  // calculate bond transforms where they are cylinders
  for (size_t i = 0; i < connections.size(); i++)
  {
    const auto& conn = connections[i];
    const auto& a = atoms[conn.atom1].position;
    const auto& b = atoms[conn.atom2].position;
    const auto mid = (a + b) * 0.5f;

    // y = b - a
    const auto y = b - a;

    // x: perpendicular to y, arbitrary
    auto x = mdEngine::Vec3{y.y, -y.x, 0.0f};
    if (mdEngine::Vec3::length(x) < 0.0001f)
      x = mdEngine::Vec3{0.0f, y.z, -y.y};
    x = mdEngine::Vec3::normalize(x) * bondThickness;

    // z = x cross y
    auto z = mdEngine::Vec3::cross(x, y);
    z = mdEngine::Vec3::normalize(z) * bondThickness;

    // construct transform matrix from x, y, z
    // with a being translation
    connectionTransforms[i] = {
      x.x, y.x, z.x, mid.x,
      x.y, y.y, z.y, mid.y,
      x.z, y.z, z.z, mid.z,
      0.0f, 0.0f, 0.0f, 1.0f
    };
  }
}

void MoleculeInstance::setAtomScale(const float scale)
{
  atomScale = scale;
}

void MoleculeInstance::setBondThickness(const float thickness)
{
  if (thickness == bondThickness)
    return;
  bondThickness = thickness;
  rebuildConnectionTransforms();
}

void MoleculeInstance::update(float deltaTime)
{
}

void MoleculeInstance::render(const mdEngine::Mat4& transform, std::vector<mdEngine::DrawObj>& drawObjs) const
{
  // render atoms
  for (const auto& atom : atoms)
  {
    drawObjs.push_back({
      .meshHandle = meshLibraryPtr->sphere,
      .transform = transform
        * mdEngine::Mat4::translate(atom.position.x, atom.position.y, atom.position.z)
        * mdEngine::Mat4::scale(atom.radius * 2.0f * atomScale, atom.radius * 2.0f * atomScale, atom.radius * 2.0f * atomScale),
      .color = mdEngine::Color3f(
        static_cast<float>(atom.color.r) / 255.0f,
        static_cast<float>(atom.color.g) / 255.0f,
        static_cast<float>(atom.color.b) / 255.0f),
      .shininess = 32.0f
    });
  }

  // render connections
  for (size_t i = 0; i < connections.size(); i++)
  {
    drawObjs.push_back({
      .meshHandle = meshLibraryPtr->cylinder,
      .transform = transform * connectionTransforms[i],
      .color = mdEngine::Color3f(0.2f, 0.2f, 0.2f),
      .shininess = 32.0f
    });
  }
}

Part2::Part2(mdEngine::Renderer* rendererPtr, mdEngine::Window* windowPtr) : SceneBase(rendererPtr, windowPtr), meshLibrary(rendererPtr)
{
  // load elements from JSON files
  for (const auto& entry : std::filesystem::directory_iterator("assets/elements"))
  {
    if (entry.path().extension() == ".json")
    {
      std::ifstream file(entry.path());
      nlohmann::json j;
      file >> j;
      elements.push_back(Element::parse(j));
    }
  }

  // load molecules from JSON files
  for (const auto& entry : std::filesystem::directory_iterator("assets/molecules"))
  {
    if (entry.path().extension() == ".json")
    {
      std::ifstream file(entry.path());
      nlohmann::json j;
      file >> j;
      molecules.push_back(Molecule::parse(j));
    }
  }

  std::sort(elements.begin(), elements.end(), [](const Element& a, const Element& b)
  {
    return atomicNumber(a) < atomicNumber(b);
  });
  std::sort(molecules.begin(), molecules.end(), [](const Molecule& a, const Molecule& b)
  {
    return a.name < b.name;
  });

  selectElement(findByName(elements, "Carbon"));
  selectMolecule(findByName(molecules, "Water"));
}

Part2::~Part2()
{
}

void Part2::selectElement(const size_t index)
{
  elementIdx = index;
  const auto& e = elements[index];
  elementInstance = std::make_unique<ElementInstance>(rendererPtr, &meshLibrary, e.color, e.radius, e.shells);
}

void Part2::selectMolecule(const size_t index)
{
  moleculeIdx = index;
  moleculeInstance = std::make_unique<MoleculeInstance>(rendererPtr, &meshLibrary, molecules[index].atoms, molecules[index].connections);
  moleculeInstance->setAtomScale(atomScale);
  moleculeInstance->setBondThickness(bondThickness);
}

void Part2::update(float deltaTime)
{
  // ElementInstance rotates electrons at 20 degrees per second
  elementInstance->update(electronsPaused ? 0.0f : deltaTime * electronSpeed / 20.0f);
  moleculeInstance->update(deltaTime);

  if (moleculeSpin)
    moleculeAngle += moleculeSpinSpeed * DEG_TO_RAD * deltaTime;
}

void Part2::renderGui()
{
  SceneBase::renderGui();

  if (ImGui::Begin("Atom"))
  {
    const auto elementLabel = [this](const size_t i) { return std::to_string(atomicNumber(elements[i])) + " - " + elements[i].name; };

    if (ImGui::BeginCombo("Element", elementLabel(elementIdx).c_str()))
    {
      for (size_t i = 0; i < elements.size(); i++)
      {
        ImGui::PushID(static_cast<int>(i));
        const bool isSelected = (elementIdx == i);
        if (ImGui::Selectable(elementLabel(i).c_str(), isSelected))
          selectElement(i);
        if (isSelected)
          ImGui::SetItemDefaultFocus();
        ImGui::PopID();
      }
      ImGui::EndCombo();
    }

    if (ImGui::ArrowButton("##elementPrev", ImGuiDir_Left) && elementIdx > 0)
      selectElement(elementIdx - 1);
    ImGui::SameLine();
    if (ImGui::ArrowButton("##elementNext", ImGuiDir_Right) && elementIdx + 1 < elements.size())
      selectElement(elementIdx + 1);
    ImGui::SameLine();
    ImGui::Text("%zu / %zu", elementIdx + 1, elements.size());

    const auto& element = elements[elementIdx];
    std::string shellText;
    for (const auto shell : element.shells)
      shellText += (shellText.empty() ? "" : ", ") + std::to_string(shell);

    ImGui::Separator();
    ImGui::Text("Atomic number: %zu", atomicNumber(element));
    ImGui::Text("Electrons per shell: %s", shellText.c_str());
    ImGui::Text("Nucleus radius: %.2f", element.radius);

    ImGui::Separator();
    ImGui::SliderFloat("Electron speed", &electronSpeed, 0.0f, 360.0f, "%.0f deg/s");
    ImGui::Checkbox("Pause electrons", &electronsPaused);
    ImGui::SliderFloat("Tilt", &atomTilt, 0.0f, 90.0f, "%.0f deg");
  }
  ImGui::End();

  if (ImGui::Begin("Molecule"))
  {
    if (ImGui::BeginCombo("Molecule", molecules[moleculeIdx].name.c_str()))
    {
      for (size_t i = 0; i < molecules.size(); i++)
      {
        ImGui::PushID(static_cast<int>(i));
        const bool isSelected = (moleculeIdx == i);
        if (ImGui::Selectable(molecules[i].name.c_str(), isSelected))
          selectMolecule(i);
        if (isSelected)
          ImGui::SetItemDefaultFocus();
        ImGui::PopID();
      }
      ImGui::EndCombo();
    }

    if (ImGui::ArrowButton("##moleculePrev", ImGuiDir_Left) && moleculeIdx > 0)
      selectMolecule(moleculeIdx - 1);
    ImGui::SameLine();
    if (ImGui::ArrowButton("##moleculeNext", ImGuiDir_Right) && moleculeIdx + 1 < molecules.size())
      selectMolecule(moleculeIdx + 1);
    ImGui::SameLine();
    ImGui::Text("%zu / %zu", moleculeIdx + 1, molecules.size());

    const auto& molecule = molecules[moleculeIdx];
    size_t bondCounts[4] = {};
    for (const auto& connection : molecule.connections)
      bondCounts[std::min<size_t>(connection.order, 3)]++;

    ImGui::Separator();
    ImGui::Text("Atoms: %zu", molecule.atoms.size());
    ImGui::Text("Bonds: %zu (single %zu, double %zu, triple %zu)", molecule.connections.size(), bondCounts[1], bondCounts[2], bondCounts[3]);

    ImGui::Separator();
    if (ImGui::SliderFloat("Atom size", &atomScale, 0.25f, 5.0f, "%.2fx"))
      moleculeInstance->setAtomScale(atomScale);

    if (ImGui::SliderFloat("Bond thickness", &bondThickness, 0.05f, 0.5f, "%.2f"))
      moleculeInstance->setBondThickness(bondThickness);

    ImGui::Separator();
    ImGui::Checkbox("Spin", &moleculeSpin);
    if (moleculeSpin)
      ImGui::SliderFloat("Spin speed", &moleculeSpinSpeed, 0.0f, 360.0f, "%.0f deg/s");
  }
  ImGui::End();
}

void Part2::render(std::vector<mdEngine::DrawObj>& drawObjs, mdEngine::CameraObj& cameraObj, mdEngine::LightingObj& lightingObj)
{
  SceneBase::render(drawObjs, cameraObj, lightingObj);

  cameraObj.view = camera.getViewMatrix();

  elementInstance->render(mdEngine::Mat4::translate(-5, 0, 0) * mdEngine::Mat4::rotate(atomTilt * DEG_TO_RAD, 1.0f, 0.0f, 0.0f), drawObjs);
  moleculeInstance->render(mdEngine::Mat4::translate(5, 0, 0) * mdEngine::Mat4::rotate(moleculeAngle, 0.0f, 1.0f, 0.0f), drawObjs);
}
