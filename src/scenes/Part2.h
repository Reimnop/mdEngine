#pragma once

#include <string>
#include <nlohmann/json.hpp>

#include "Part2MeshLibrary.h"
#include "SceneBase.h"
#include "core/data/Color3.h"

struct Element
{
  std::string name;
  mdEngine::Color3 color;
  float radius = 0.0f;
  std::vector<size_t> shells;

  static Element parse(const nlohmann::json& j)
  {
    Element e;
    e.name = j["name"];
    e.color = mdEngine::Color3(j["color"]);
    e.radius = j["radius"];
    for (const auto& s : j["shells"])
      e.shells.push_back(s);
    return e;
  }
};

class ElementInstance
{
public:
  ElementInstance(mdEngine::Renderer* rendererPtr, Part2MeshLibrary* meshLibraryPtr, mdEngine::Color3 color, float radius, const std::vector<size_t>& shells);
  ~ElementInstance();

  void update(float deltaTime);
  void render(const mdEngine::Mat4& transform, std::vector<mdEngine::DrawObj>& drawObjs) const;
private:
  mdEngine::Renderer* rendererPtr;
  Part2MeshLibrary* meshLibraryPtr;

  mdEngine::Color3 color;
  float radius;
  std::vector<size_t> shells;
  std::vector<float> shellRotations;
  std::vector<mdEngine::MeshHandle> shellTori;
};

struct Molecule
{
  struct Atom
  {
    float radius = 0.0f;
    mdEngine::Vec3 position;
    mdEngine::Color3 color;
  };

  struct Connection
  {
    size_t atom1 = 0;
    size_t atom2 = 0;
    size_t order = 1;
  };

  std::string name;
  std::vector<Atom> atoms;
  std::vector<Connection> connections;

  static Molecule parse(const nlohmann::json& j)
  {
    Molecule m;
    m.name = j["name"];
    for (const auto& a : j["atoms"])
    {
      Atom atom;
      atom.radius = a["radius"];
      atom.position = {a["position"][0], a["position"][1], a["position"][2]};
      atom.color = mdEngine::Color3(a["color"]);
      m.atoms.push_back(atom);
    }
    for (const auto& c : j["connections"])
    {
      Connection conn;
      conn.atom1 = c[0];
      conn.atom2 = c[1];
      conn.order = c[2];
      m.connections.push_back(conn);
    }
    return m;
  }
};

class MoleculeInstance
{
public:
  MoleculeInstance(mdEngine::Renderer* rendererPtr, Part2MeshLibrary* meshLibraryPtr, const std::vector<Molecule::Atom>& atoms, const std::vector<Molecule::Connection>& connections);
  ~MoleculeInstance();

  void update(float deltaTime);
  void render(const mdEngine::Mat4& transform, std::vector<mdEngine::DrawObj>& drawObjs) const;
private:
  mdEngine::Renderer* rendererPtr;
  Part2MeshLibrary* meshLibraryPtr;

  std::vector<Molecule::Atom> atoms;
  std::vector<Molecule::Connection> connections;
  std::vector<mdEngine::Mat4> connectionTransforms;
};

class Part2 : public SceneBase
{
public:
  Part2(mdEngine::Renderer* rendererPtr, mdEngine::Window* windowPtr);
  ~Part2() override;

  void update(float deltaTime) override;
  void renderGui() override;
protected:
  void render(std::vector<mdEngine::DrawObj>& drawObjs, mdEngine::CameraObj& cameraObj, mdEngine::LightingObj& lightingObj) override;
private:
  Part2MeshLibrary meshLibrary;

  std::vector<Element> elements;
  std::vector<Molecule> molecules;

  std::unique_ptr<MoleculeInstance> moleculeInstance;
  std::unique_ptr<ElementInstance> elementInstance;
};
