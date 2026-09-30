#include "Part1.h"

#include <cmath>

#include "core/data/Vertex.h"

struct MeshData { std::vector<mdEngine::Vertex> v; std::vector<uint32_t> i; };

MeshData makeSphere(int stacks, int slices)
{
  MeshData m;
  for (int y = 0; y <= stacks; y++)
    for (int x = 0; x <= slices; x++)
    {
      float th = float(y) / stacks * 3.14159265f;
      float ph = float(x) / slices * 6.28318530f;
      float px = std::sin(th) * std::cos(ph), py = std::cos(th), pz = std::sin(th) * std::sin(ph);
      m.v.push_back({{px,py,pz}, {px,py,pz}, {float(x)/slices, float(y)/stacks}});
    }
  for (int y = 0; y < stacks; y++)
    for (int x = 0; x < slices; x++)
    {
      uint32_t a = y*(slices+1)+x, b = a+slices+1;
      for (uint32_t k : {a,b,a+1, a+1,b,b+1}) m.i.push_back(k);
    }
  return m;
}

mdEngine::MeshHandle createSphereMesh(mdEngine::Renderer& renderer)
{
  auto meshData = makeSphere(20, 20);
  return renderer.addMesh(
    meshData.v.data(),
    static_cast<GLsizei>(meshData.v.size()),
    meshData.i.data(),
    static_cast<GLsizei>(meshData.i.size()));
}

Part1::Part1(mdEngine::Renderer* rendererPtr, mdEngine::Window* windowPtr)
  : rendererPtr(rendererPtr), camera(windowPtr), mesh(createSphereMesh(*rendererPtr))
{
}

Part1::~Part1()
{
  rendererPtr->clearMeshes();
}

void Part1::update()
{
  camera.update();
}

void Part1::render(std::vector<mdEngine::DrawObj>& drawObjs, mdEngine::CameraObj& cameraObj)
{
  cameraObj.view = camera.getViewMatrix();
  drawObjs.push_back({ .meshHandle = mesh });
}
