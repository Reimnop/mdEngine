#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace mdEngine
{
  class Window
  {
  public:
    Window();
    ~Window();

    [[nodiscard]] bool getWindowShouldClose() const;
    void getSize(int& width, int& height) const;

    void pollEvents() const;
    void swapBuffers() const;
  private:
    static constexpr auto INIT_WIDTH = 1280;
    static constexpr auto INIT_HEIGHT = 720;

    GLFWwindow* window;
  };
} // mdEngine
