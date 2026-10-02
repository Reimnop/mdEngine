#include "Aces.h"

namespace mdEngine
{
  Aces::Aces()
  {
    const auto fss = io::readFileAsString("assets/shaders/post_processing/aces.fsh");
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

    uExposureLocation = glGetUniformLocation(this->program, "uExposure");
  }

  Aces::~Aces()
  {
    glDeleteProgram(this->program);
  }

  bool Aces::process(GLuint inputTexture, GLuint outputTexture, GLsizei width, GLsizei height, float exposure)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(this->program);
    glUniform1f(uExposureLocation, exposure);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    return true;
  }
} // mdEngine