#include "Orchestrator.h"

#include "scenes/Part1.h"
#include "scenes/Part2.h"

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

      renderSceneSwitcher();

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glClear(GL_COLOR_BUFFER_BIT);

      imgui.endFrame();

      window.swapBuffers();
    }
  }

  void Orchestrator::renderSceneSwitcher()
  {
    if (ImGui::Begin("Scene Switcher"))
    {
      if (ImGui::BeginCombo("Scene", SCENE_NAMES[currentSceneIdx]))
      {
        for (size_t i = 0; i < std::size(SCENE_NAMES); i++)
        {
          ImGui::PushID(static_cast<int>(i));
          const bool isSelected = (currentSceneIdx == i);
          if (ImGui::Selectable(SCENE_NAMES[i], isSelected))
          {
            currentSceneIdx = i;
            switch (currentSceneIdx)
            {
              case 0:
                currentScene = std::make_unique<Part1>(&renderer, &window);
                break;
              case 1:
                currentScene = std::make_unique<Part2>(&renderer, &window);
                break;
              default:
                break;
            }
          }
          if (isSelected)
            ImGui::SetItemDefaultFocus();
          ImGui::PopID();
        }
        ImGui::EndCombo();
      }
    }
    ImGui::End();
  }
} // mdEngine