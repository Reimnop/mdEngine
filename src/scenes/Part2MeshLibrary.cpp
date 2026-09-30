#include "Part2MeshLibrary.h"

#include "Shapes.h"

Part2MeshLibrary::Part2MeshLibrary(mdEngine::Renderer* rendererPtr) : rendererPtr(rendererPtr)
{
  auto sphereMesh = makeSphere();
  sphere = rendererPtr->createMesh(
    sphereMesh.v.data(),
    static_cast<GLsizei>(sphereMesh.v.size()),
    sphereMesh.i.data(),
    static_cast<GLsizei>(sphereMesh.i.size()));

  auto cylinderMesh = makeCylinder();
  cylinder = rendererPtr->createMesh(
    cylinderMesh.v.data(),
    static_cast<GLsizei>(cylinderMesh.v.size()),
    cylinderMesh.i.data(),
    static_cast<GLsizei>(cylinderMesh.i.size()));
}

Part2MeshLibrary::~Part2MeshLibrary()
{
  rendererPtr->deleteMesh(sphere);
  rendererPtr->deleteMesh(cylinder);
}
