#include "Orchestrator.h"

#include "scenes/Part1.h"

namespace mdEngine
{
  Orchestrator::Orchestrator():
    window(Window()),
    renderer(),
    imgui(&window),
    currentScene(std::make_unique<Part1>(&renderer, &window))
  {
  }

  Orchestrator::~Orchestrator() = default;

  void Orchestrator::start()
  {
    while (!window.getWindowShouldClose())
    {
      window.pollEvents();

      imgui.beginFrame();

      currentScene->update();
      currentScene->renderGui();

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glClear(GL_COLOR_BUFFER_BIT);

      imgui.endFrame();

      window.swapBuffers();
    }
  }
} // mdEngine