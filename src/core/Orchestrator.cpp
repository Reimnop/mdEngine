#include "Orchestrator.h"

#include <cmath>

namespace mdEngine
{
  struct MeshData { std::vector<Vertex> v; std::vector<uint32_t> i; };

  MeshData makeQuad()
  {
    return {
      {{{-1,-1,0},{0,0,1},{0,0}}, {{1,-1,0},{0,0,1},{1,0}},
       {{1,1,0},{0,0,1},{1,1}},   {{-1,1,0},{0,0,1},{0,1}}},
      {0,1,2, 0,2,3}
    };
  }

  // 24 verts so each face gets its own flat normal
  MeshData makeCube()
  {
    MeshData m;
    const float n[6][3] = {{0,0,1},{0,0,-1},{1,0,0},{-1,0,0},{0,1,0},{0,-1,0}};
    const float u[6][3] = {{1,0,0},{-1,0,0},{0,0,-1},{0,0,1},{1,0,0},{1,0,0}};
    for (int f = 0; f < 6; f++)
    {
      // v = n x u
      float vx = n[f][1]*u[f][2] - n[f][2]*u[f][1];
      float vy = n[f][2]*u[f][0] - n[f][0]*u[f][2];
      float vz = n[f][0]*u[f][1] - n[f][1]*u[f][0];
      const float s[4][2] = {{-1,-1},{1,-1},{1,1},{-1,1}};
      for (auto& c : s)
        m.v.push_back({
          {n[f][0]+u[f][0]*c[0]+vx*c[1], n[f][1]+u[f][1]*c[0]+vy*c[1], n[f][2]+u[f][2]*c[0]+vz*c[1]},
          {n[f][0], n[f][1], n[f][2]},
          {(c[0]+1)/2, (c[1]+1)/2}});
      uint32_t b = f * 4;
      for (uint32_t k : {0,1,2, 0,2,3}) m.i.push_back(b + k);
    }
    return m;
  }

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

  Orchestrator::Orchestrator(): window(Window()), renderer(&window)
  {
    auto sphere = makeSphere(32, 32);
    bullshit = renderer.addMesh(sphere.v.data(), static_cast<GLsizei>(sphere.v.size()), sphere.i.data(), static_cast<GLsizei>(sphere.i.size()));
  }

  Orchestrator::~Orchestrator() = default;

  void Orchestrator::start()
  {
    while (!window.getWindowShouldClose())
    {
      window.pollEvents();

      const DrawObj drawObjs[] = { {.meshHandle = bullshit} };

      renderer.renderFrame(drawObjs, 1, CameraObj{.view = Mat4::translate(0, 0, -5)});
      window.swapBuffers();
    }
  }
} // mdEngine