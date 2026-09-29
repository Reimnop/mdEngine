#include "Orchestrator.h"

#include "scenes/Part1.h"

namespace mdEngine
{
  Orchestrator::Orchestrator(): window(Window()), renderer(&window), currentScene(std::make_unique<Part1>(&renderer))
  {
  }

  Orchestrator::~Orchestrator() = default;

  void Orchestrator::start()
  {
    while (!window.getWindowShouldClose())
    {
      window.pollEvents();

      drawObjs.clear();
      cameraObj = CameraObj();

      currentScene->update();
      currentScene->render(drawObjs, cameraObj);

      renderer.renderFrame(drawObjs.data(), drawObjs.size(), cameraObj);
      window.swapBuffers();
    }
  }
} // mdEngine