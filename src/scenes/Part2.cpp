#include "Part2.h"

#include <fstream>

#include "Shapes.h"

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
    .color = {static_cast<float>(color.r) / 255.0f, static_cast<float>(color.g) / 255.0f, static_cast<float>(color.b) / 255.0f},
    .shininess = 32.0f
  });

  // render shells
  for (auto i = 0; i < shells.size(); i++)
  {
    const auto shellRadius = static_cast<float>(i + 1);
    drawObjs.push_back({
      .meshHandle = shellTori[i],
      .transform = transform,
      .color = {0.2f, 0.2f, 0.2f},
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
        .color = {0.8f, 0.4f, 0.2f},
        .shininess = 32.0f
      });
    }
  }
}

MoleculeInstance::MoleculeInstance(mdEngine::Renderer* rendererPtr, Part2MeshLibrary* meshLibraryPtr, const std::vector<Molecule::Atom>& atoms, const std::vector<Molecule::Connection>& connections)
  : rendererPtr(rendererPtr), meshLibraryPtr(meshLibraryPtr), atoms(atoms), connections(connections)
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
    x = mdEngine::Vec3::normalize(x) * 0.2f;

    // z = x cross y
    auto z = mdEngine::Vec3::cross(x, y);
    z = mdEngine::Vec3::normalize(z) * 0.2f;

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

MoleculeInstance::~MoleculeInstance() = default;

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
        * mdEngine::Mat4::scale(atom.radius * 2.0f, atom.radius * 2.0f, atom.radius * 2.0f),
      .color = {static_cast<float>(atom.color.r) / 255.0f, static_cast<float>(atom.color.g) / 255.0f, static_cast<float>(atom.color.b) / 255.0f},
      .shininess = 32.0f
    });
  }

  // render connections
  for (size_t i = 0; i < connections.size(); i++)
  {
    drawObjs.push_back({
      .meshHandle = meshLibraryPtr->cylinder,
      .transform = transform * connectionTransforms[i],
      .color = {0.2f, 0.2f, 0.2f},
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

  elementInstance = std::make_unique<ElementInstance>(rendererPtr, &meshLibrary, elements[0].color, elements[0].radius, elements[0].shells);

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

  moleculeInstance = std::make_unique<MoleculeInstance>(rendererPtr, &meshLibrary, molecules[0].atoms, molecules[0].connections);
}

Part2::~Part2()
{
}

void Part2::update(float deltaTime)
{
  elementInstance->update(deltaTime);
  moleculeInstance->update(deltaTime);
}

void Part2::renderGui()
{
  SceneBase::renderGui();
}

void Part2::render(std::vector<mdEngine::DrawObj>& drawObjs, mdEngine::CameraObj& cameraObj, mdEngine::LightingObj& lightingObj)
{
  cameraObj.view = camera.getViewMatrix();

  elementInstance->render(mdEngine::Mat4::translate(-5, 0, 0), drawObjs);
  moleculeInstance->render(mdEngine::Mat4::translate(5, 0, 0), drawObjs);
}
