#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "core/Orchestrator.h"

int main()
{
  auto orchestrator = mdEngine::Orchestrator();
  orchestrator.start();

  return 0;
}
