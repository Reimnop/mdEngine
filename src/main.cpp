#define STB_IMAGE_IMPLEMENTATION

#include "core/Orchestrator.h"

int main()
{
  auto orchestrator = mdEngine::Orchestrator();
  orchestrator.start();

  return 0;
}
