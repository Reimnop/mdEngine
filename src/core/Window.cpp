#include "Window.h"

#include <stdexcept>

namespace mdEngine
{
  Window::Window()
  {
    if (!glfwInit())
    {
      throw std::runtime_error("glfwInit failed");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    this->window = glfwCreateWindow(INIT_WIDTH, INIT_HEIGHT, "mdEngine | CC01 | team 2", nullptr, nullptr);
    if (!this->window)
    {
      glfwTerminate();
      throw std::runtime_error("glfwCreateWindow failed");
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
  }

  Window::~Window()
  {
    glfwDestroyWindow(this->window);
    glfwTerminate();
  }

  GLFWwindow* Window::getHandle() const
  {
    return window;
  }

  bool Window::getWindowShouldClose() const
  {
    return glfwWindowShouldClose(this->window);
  }

  void Window::getSize(int& width, int& height) const
  {
    glfwGetWindowSize(this->window, &width, &height);
  }

  void Window::pollEvents() const
  {
    glfwPollEvents();
  }

  void Window::swapBuffers() const
  {
    glfwSwapBuffers(this->window);
  }
} // mdEngine