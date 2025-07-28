#include <iostream>
#include "Renderer/OpenGLRenderer/OpenGLRenderer.h"

int
main ()
{
  std::cout << "Hello, CMake!" << std::endl;

  OpenGLRenderer renderer("Hello Window!", 800, 400);
  std::cout << "Starting render loop" << std::endl;
  renderer.run();
  return 0;
}