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

    glfwSetWindowUserPointer(window, this);
    glfwSetScrollCallback(window, [](GLFWwindow* w, double, double yoffset)
    {
      static_cast<Window*>(glfwGetWindowUserPointer(w))->scrollDelta += yoffset;
    });
  }

  Window::~Window()
  {
    glfwDestroyWindow(this->window);
    glfwTerminate();
  }

  bool Window::getWindowShouldClose() const
  {
    return glfwWindowShouldClose(this->window);
  }

  void Window::getSize(int& width, int& height) const
  {
    glfwGetWindowSize(this->window, &width, &height);
  }

  bool Window::isMouseButtonDown(const int button) const
  {
    return glfwGetMouseButton(this->window, button) == GLFW_PRESS;
  }

  void Window::getCursorPos(double& x, double& y) const
  {
    glfwGetCursorPos(this->window, &x, &y);
  }

  double Window::consumeScrollDelta()
  {
    const double d = this->scrollDelta;
    this->scrollDelta = 0.0;
    return d;
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