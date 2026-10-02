#include "Composite.h"

namespace mdEngine
{
  Composite::Composite()
  {
    const auto fss = io::readFileAsString("assets/shaders/post_processing/composite.fsh");
    const auto fssPtr = fss.c_str();

    const auto vertexShader = createVertexShader();

    const auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fssPtr, nullptr);
    glCompileShader(fragmentShader);

    this->program = glCreateProgram();
    glAttachShader(this->program, vertexShader);
    glAttachShader(this->program, fragmentShader);
    glLinkProgram(this->program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
  }

  Composite::~Composite()
  {
    glDeleteProgram(this->program);
  }

  bool Composite::process(GLuint inputTexture, GLuint outputTexture, GLsizei width, GLsizei height)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(this->program);
    glBindVertexArray(vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    return true;
  }
} // mdEngine