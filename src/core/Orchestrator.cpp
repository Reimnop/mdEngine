#include "Orchestrator.h"

#include "scenes/Part1.h"
#include "scenes/Part2.h"

namespace mdEngine
{
  Orchestrator::Orchestrator():
    window(Window()),
    renderer(),
    imgui(&window),
    currentScene(std::make_unique<Part2>(&renderer, &window))
  {
  }

  Orchestrator::~Orchestrator() = default;

  void Orchestrator::start()
  {
    double prevTime = window.getTime();

    while (!window.getWindowShouldClose())
    {
      window.pollEvents();

      const double time = window.getTime();
      const double deltaTime = time - prevTime;
      prevTime = time;

      imgui.beginFrame();

      currentScene->update(static_cast<float>(deltaTime));
      currentScene->renderGui();

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glClear(GL_COLOR_BUFFER_BIT);

      imgui.endFrame();

      window.swapBuffers();
    }
  }
} // mdEngine