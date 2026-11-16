#include "Part1.h"

#include "ImageLoader.h"

Part1::Part1(mdEngine::Renderer* rendererPtr, mdEngine::Window* windowPtr): SceneBase(rendererPtr, windowPtr)
{
}

Part1::~Part1()
{
}

void Part1::update(float deltaTime)
{
}

void Part1::render(std::vector<mdEngine::DrawObj>& drawObjs, mdEngine::CameraObj& cameraObj, mdEngine::LightingObj& lightingObj, mdEngine::PostProcessingObj& postProcessingObj)
{
  SceneBase::render(drawObjs, cameraObj, lightingObj, postProcessingObj);

  cameraObj.view = camera.getViewMatrix();
}

void Part1::renderGui()
{
  SceneBase::renderGui();
}
