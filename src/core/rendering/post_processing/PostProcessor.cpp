#include "PostProcessor.h"

namespace mdEngine
{
  PostProcessor::PostProcessor()
  {
    glGenVertexArrays(1, &vao);
    glGenFramebuffers(1, &fbo);
  }

  PostProcessor::~PostProcessor()
  {
    glDeleteVertexArrays(1, &vao);
    glDeleteFramebuffers(1, &fbo);
  }

  GLuint PostProcessor::createVertexShader()
  {
    const auto vss = io::readFileAsString("assets/shaders/post_processing/common.vsh");
    const auto vssPtr = vss.c_str();

    const auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vssPtr, nullptr);
    glCompileShader(vertexShader);

    return vertexShader;
  }

  GLuint PostProcessor::createProgram(const char* fragmentShaderPath)
  {
    const auto fss = io::readFileAsString(fragmentShaderPath);
    const auto fssPtr = fss.c_str();

    const auto vertexShader = createVertexShader();

    const auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fssPtr, nullptr);
    glCompileShader(fragmentShader);

    const auto program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
  }
} // mdEngine
